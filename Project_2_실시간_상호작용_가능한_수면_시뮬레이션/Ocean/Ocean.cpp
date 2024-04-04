#include "stdafx.h"
#include "Ocean.h"

#include "GameInstance.h"
#include "Engine_Struct.h"
#include "Light_Manager.h"

#include "TextureUtil.h"

#include "Gui.h"

#include "QuadTree.h"

COcean::COcean(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}

COcean::COcean(const COcean& rhs)
	: CGameObject(rhs)
{
}

HRESULT COcean::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT COcean::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	InitSurface();
	Add_Components();

	// Init Compute Shader Constant Buffer
	{
		if (FAILED(m_pComputeBufferCom->Bind_ShaderResourceView(m_pShaderCom, "g_CurrentSurface", 0)))
			return E_FAIL;
		if (FAILED(m_pComputeBufferCom->Bind_ShaderResourceView(m_pShaderCom, "g_PreviousSurface", 1)))
			return E_FAIL;
		if (FAILED(m_pComputeBufferCom->Bind_ShaderResourceView(m_pShaderCom, "g_DampingCoefficient", 2)))
			return E_FAIL;
		if (FAILED(m_pComputeBufferCom->Bind_UnorderedAccessView(m_pShaderCom, "g_AnimatedSurface", 0)))
			return E_FAIL;
		if (FAILED(m_pComputeBufferCom->Bind_UnorderedAccessView(m_pShaderCom, "g_SurfaceVertices", 1)))
			return E_FAIL;
		if (FAILED(m_pComputeBufferCom->Bind_UnorderedAccessView(m_pShaderCom, "g_SurfacePosition", 2)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_Int("g_iSimulateBufferWidth", m_iSimulateGridWidth)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_Int("g_iQuadTreeGridWidth", m_iQuadTreeGridWidth)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_Int("g_iPixelsPerThread", m_iPixelsPerThread)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_Float("g_fGridCellWidth", m_fGridCellWidth)))
			return E_FAIL;
		
		const float fWavePropagationCoefficient = (m_fWaveSpaceOrigin * m_fWaveTimeStep / m_fGridCellWidth) * (m_fWaveSpaceOrigin * m_fWaveTimeStep / m_fGridCellWidth);
		const float fCenterWeightingFactor = 2 - 4 * fWavePropagationCoefficient;
		if (FAILED(m_pShaderCom->Bind_Float("g_fWavePropagationCoefficient", fWavePropagationCoefficient)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_Float("g_fCenterWeightingFactor", fCenterWeightingFactor)))
			return E_FAIL;
	}

	// Init Shader Constant Buffer
	{
		if (FAILED(m_pShaderCom->Bind_Float("g_dynamicTesselationAmount", m_fDynamicTesselationAmount)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_Float("g_staticTesselationOffset", m_fStaticTessellationOffset)))
			return E_FAIL;

		if (FAILED(m_pShaderCom->Bind_Float("g_localWavesSimulationDomainWorldspaceSize", m_fLocalWavesSimulationDomainWorldspaceSize)))
			return E_FAIL;
		if (FAILED(m_pShaderCom->Bind_RawValue("g_localWavesSimulationDomainWorldspaceCenter", &m_fLocalWavesSimulationDomainWorldspaceCenter, sizeof(_float2))))
			return E_FAIL;

		if (FAILED(m_pFlowingWaterTexture->Bind_ShaderResourceView(m_pShaderCom, "g_textureFlowingWater")))
			return E_FAIL;

		// Find IBL Cubemap
		CGameInstance* pGameInstance = CGameInstance::Get_Instance();
		Safe_AddRef(pGameInstance);

		auto pCubemap = pGameInstance->Find_GameObject(TEXT("IBLCube"));
		m_pIBLTexture = static_cast<CTexture*>(pCubemap->Find_Component(TEXT("Com_Texture")));
		Safe_AddRef(m_pIBLTexture);

		Safe_Release(pGameInstance);

		// Send IBL Texture
		if (FAILED(m_pIBLTexture->Bind_ShaderResourceView(m_pShaderCom, "g_ReflectionCapture")))
			return E_FAIL;
	}

	std::vector<_float3> vecVertices;
	vecVertices.resize(m_iQuadTreeGridWidth * m_iQuadTreeGridWidth);

	m_pComputeBufferCom->Dispatch(m_pShaderCom, 1, 2, m_iPixelsPerThread, m_iPixelsPerThread, 1);
	m_pComputeBufferCom->StoreOutput(vecVertices.data(), 2);

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_VIBuffer_Surface"),
		TEXT("Com_VIBuffer"), (CComponent**)&m_pVIBufferCom)))
		return E_FAIL;

	// Quad Tree는 반드시 (2의 n승 + 1) 이어야 함.
	m_pQuadTree = CQuadTree::Create(m_iQuadTreeGridWidth, m_iQuadTreeGridWidth);
	m_pQuadTree->Build_QuadTree(vecVertices);

	m_vecPatch.reserve(MAX_SURFACE_PATCH);
		
	return S_OK;
}

void COcean::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	TuneDampingCoefficient(fTimeDelta);
	m_pShaderCom->Bind_Float("g_TempDampingCoefficient", m_fDampingCoefficient);

	AnimateWater();

	// WindWave
	m_vWindWave.x += fTimeDelta * 0.05f;
	m_pShaderCom->Bind_RawValue("g_windWave", &m_vWindWave, sizeof(_float2));
	

	// Gui
#ifdef USE_IMGUI
	{
		ImGui::Begin("Ocean Simulation");

		(ImGui::Text("Simulation Setting"));
		{
			ImGui::SliderFloat("Wave TimeStep", &m_fWaveTimeStep, 0.f, 1.f);
		}

		(ImGui::Text("Tessellation Parameter"));
		{
			if (ImGui::SliderFloat("DynamicTessellationAmount", &m_fDynamicTesselationAmount, 0.f, 10.f))
				m_pShaderCom->Bind_Float("g_dynamicTesselationAmount", m_fDynamicTesselationAmount);

			if (ImGui::SliderFloat("StaticTessellationOffset", &m_fStaticTessellationOffset, 0.f, 10.f))
				m_pShaderCom->Bind_Float("g_staticTesselationOffset", m_fStaticTessellationOffset);
		}

		(ImGui::Text("Domain Parameter"));
		{
			/*if (ImGui::SliderFloat("Cascade0UVScale", &m_fCascade0UVScale, 0.f, 1.f))
				m_pShaderCom->Bind_Float("g_cascade0UVScale", m_fCascade0UVScale);
			if (ImGui::SliderFloat("Cascade1UVScale", &m_fCascade1UVScale, 0.f, 1.f))
				m_pShaderCom->Bind_Float("g_cascade1UVScale", m_fCascade1UVScale);
			if (ImGui::SliderFloat("Cascade2UVScale", &m_fCascade2UVScale, 0.f, 1.f))
				m_pShaderCom->Bind_Float("g_cascade2UVScale", m_fCascade2UVScale);
			if (ImGui::SliderFloat("Cascade3UVScale", &m_fCascade3UVScale, 0.f, 1.f))
				m_pShaderCom->Bind_Float("g_cascade3UVScale", m_fCascade3UVScale);

			if (ImGui::SliderFloat("Cascade0UVOffset", &m_fCascade0UVOffset, 0.f, 1.f))
				m_pShaderCom->Bind_Float("g_cascade0UVOffset", m_fCascade0UVScale);
			if (ImGui::SliderFloat("Cascade1UVOffset", &m_fCascade1UVOffset, 0.f, 1.f))
				m_pShaderCom->Bind_Float("g_cascade1UVOffset", m_fCascade1UVScale);
			if (ImGui::SliderFloat("Cascade2UVOffset", &m_fCascade2UVOffset, 0.f, 1.f))
				m_pShaderCom->Bind_Float("g_cascade2UVOffset", m_fCascade2UVScale);
			if (ImGui::SliderFloat("Cascade3UVOffset", &m_fCascade3UVOffset, 0.f, 1.f))
				m_pShaderCom->Bind_Float("g_cascade3UVOffset", m_fCascade3UVScale);

			if (ImGui::SliderFloat("UVWarpingAmplitude", &m_fUVWarpingAmplitude, 0.f, 1.f))
				m_pShaderCom->Bind_Float("g_UVWarpingAmplitude", m_fUVWarpingAmplitude);
			if (ImGui::SliderFloat("UVWarpingFrequency", &m_fUVWarpingFrequency, 0.f, 5.f))
				m_pShaderCom->Bind_Float("g_UVWarpingFrequency", m_fUVWarpingFrequency);

			if (ImGui::SliderFloat("LocalWavesSimulationDomainWorldspaceSize", &m_fLocalWavesSimulationDomainWorldspaceSize, 0.f, 10.f))
				m_pShaderCom->Bind_Float("g_localWavesSimulationDomainWorldspaceSize", m_fLocalWavesSimulationDomainWorldspaceSize);
			if (ImGui::SliderFloat2("LocalWavesSimulationDomainWorldspaceCenter", &m_fLocalWavesSimulationDomainWorldspaceCenter.x, 0.f, 10.f))
				m_pShaderCom->Bind_RawValue("g_localWavesSimulationDomainWorldspaceCenter", &m_fLocalWavesSimulationDomainWorldspaceCenter, sizeof(_float2));*/
		}
		ImGui::End();

		ImGui::Begin("Physics & Buoyancy");
		{
			ImGui::SliderFloat("Water Density", &m_fWaterDensity, 0.f, 5000.f);
			ImGui::SliderFloat("Gravity", &m_fGravity, 0.f, 30.f);

			ImGui::SliderFloat("Drag Force", &m_fDragForce, -50000.f, 1000.f);
			ImGui::SliderFloat("Velocity Damping Coefficient", &m_fVelocityDampingCoefficient, 0.f, 2.f);
		}
		ImGui::End();
	}
#endif
}

void COcean::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	// 쿼드 트리 컬링
	if (m_vecAnimatedSurfaceVertices.empty() == false)
	{
		m_vecPatch.clear();
		m_pComputeBufferCom->StoreOutput(m_vecAnimatedSurfacePosition.data(), 2);
		m_pQuadTree->Generate_Patch(m_vecPatch, m_vecAnimatedSurfacePosition);

#ifdef USE_IMGUI
		GUITEXT GuiText;
		GuiText.vPosition = { 5.f, 40.f };
		GuiText.vColor = { 1.f, 1.f, 1.f, 1.f };

		GuiText.Content = "Render Surface Patch Count: " + to_string(m_vecPatch.size());

		CGui::Get_Instance()->RenderText(GuiText);
#endif

		m_pVIBufferCom->Update_Instance_Buffer(m_vecPatch);
	}

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_OCEAN, this);
}

HRESULT COcean::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	if (FAILED(m_pRigidbodyStateCom->Bind_ShaderResource(m_pShaderCom, "g_WorldMatrix")))
		return E_FAIL;

	CGameInstance* pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);

	_float4x4 ViewMatrix = pGameInstance->Get_Transform_Float4x4_Inverse(CPipeLine::D3DTS_VIEW);
	_float3 vEyePos = *(_float3*)ViewMatrix.m[3];
	if (FAILED(m_pShaderCom->Bind_RawValue("g_eyePos", &vEyePos, sizeof(_float3))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_RawValue("g_eyeLook", (_float3*)ViewMatrix.m[2], sizeof(_float3))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_ViewMatrix", &pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_ProjMatrix", &pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(m_pShaderCom->Bind_Matrix("g_LightViewMatrix", pGameInstance->Get_DirectionalLight_Float4x4(CLight_Manager::D3DTS_VIEW))))
		return E_FAIL;
	if (FAILED(m_pShaderCom->Bind_Matrix("g_LightProjMatrix", pGameInstance->Get_DirectionalLight_Float4x4(CLight_Manager::D3DTS_PROJ))))
		return E_FAIL;

	if (FAILED(pGameInstance->Bind_ShaderResource_RTV(m_pShaderCom, TEXT("Target_Water_Occ"), "g_OccTexture")))
		return E_FAIL;

	if (FAILED(pGameInstance->Bind_ShaderResource_RTV(m_pShaderCom, TEXT("Target_ShadowDepth"), "g_ShadowDepth")))
		return E_FAIL;

	if (FAILED(m_pFoamTexture->Bind_ShaderResourceView(m_pShaderCom, "g_FoamTexture")))
		return E_FAIL;

	_float2 vTextureSize = _float2(g_iWinSizeX, g_iWinSizeY);
	if (FAILED(m_pShaderCom->Bind_RawValue("g_vTextureSize", &vTextureSize, sizeof(_float2))))
		return E_FAIL;
	
	Safe_Release(pGameInstance);

	// g_SurfaceVertices를 VertexShader에서 사용하기 위해 g_SurfaceVertexTable로 복사
	m_pComputeBufferCom->Bind_Result_ShaderResourceView(m_pShaderCom, "g_SurfaceVertexTable", 1);

#if defined(_DEBUG) && defined(USE_IMGUI)
	static _bool bWireframe = false;
	static _bool bRenderNormal = false;
	ImGui::Begin("Ocean Simulation");

	ImGui::Text("Render State");
	ImGui::Checkbox("Wireframe mode", &bWireframe);
	ImGui::Checkbox("Render Normals", &bRenderNormal);

	ImGui::End();

	if (bWireframe)
	{
		m_pShaderCom->Begin(1);
		if (FAILED(m_pVIBufferCom->Render()))
			return E_FAIL;
	}
	else
	{
		m_pShaderCom->Begin(0);
		if (FAILED(m_pVIBufferCom->Render()))
			return E_FAIL;
	}

	if (bRenderNormal)
	{
		m_pShaderCom->Begin(2);
		if (FAILED(m_pVIBufferCom->Render_Normal_Lines()))
			return E_FAIL;
	}
#else
	m_pShaderCom->Begin(0);
	if (FAILED(m_pVIBufferCom->Render()))
		return E_FAIL;
#endif

	return S_OK;
}

void COcean::Animate_Ship(CRigidbody* pRigidbodyCom, const GEOMETRYDESC& GeometryDesc)
{
	const float k = m_fWaterDensity * m_fGravity;//water density * gravity (물의 밀도 * 중력 가속도)
	_vector netForce = XMVectorSet(0.f, 0.f, 0.f, 0.f);//net force (물체에 작용하는 힘)
	_vector netTorque = XMVectorSet(0.f, 0.f, 0.f, 0.f);//net torque (물체에 작용하는 회전력)

	const float fMass = pRigidbodyCom->Get_Mass();

	// 중력 적용
	netForce -= XMVectorSet(0.f, m_fGravity * fMass, 0.f, 0.f); // (중력가속도 * 질량) 중력에 의해 발생하는 힘 적용

	// Buoyancy 부력 계산
	auto& vecVertices = GeometryDesc.vecVertices;
	auto& vecBuoyancyWeights = GeometryDesc.vecBuoyancyWeights;

	const _vector vVelocity = XMLoadFloat3(&pRigidbodyCom->Get_Velocity());
	const _vector vAngularVelocity = XMLoadFloat3(&pRigidbodyCom->Get_AngularVelocity());
	
	_uint iVertexCount = (_uint)vecVertices.size();
	for (_uint i = 0; i < iVertexCount; ++i)
	{
		// 회전, 크기 성분 적용
		const _vector vWorldPos = pRigidbodyCom->TransformVectorToParent(XMLoadFloat3(&vecVertices[i].vPosition));
		_float3 vHullVertex;
		XMStoreFloat3(&vHullVertex, vWorldPos + pRigidbodyCom->Get_State(CRigidbody::STATE_POSITION));
		
		_float3 vWaterNormal = { 0.f, 0.f, 0.f };
		_float fWaterHeight = 0.f;
		Interpolation_Water(vHullVertex.x, vHullVertex.z, fWaterHeight, vWaterNormal);
		
		// 수면 아래에 있는 정점들에 대해서면 부력 계산
		if (fWaterHeight > vHullVertex.y)
		{
			// 물속의 선체의 정점의 속도
			_vector vhullVertexVelocity = vVelocity + XMVector3Cross(vAngularVelocity, vWorldPos);
			_vector vDragForce = m_fDragForce * vhullVertexVelocity;
			float fBuoyancyForce = -k * (fWaterHeight - vHullVertex.y) * vecBuoyancyWeights[i] * XMVectorGetY(XMLoadFloat3(&vecVertices[i].vNormal));

			_vector vBuoyancyForceWithWaterNormal = fBuoyancyForce * XMLoadFloat3(&vWaterNormal);
			_vector vBuoyancyForceWithUpDirection = fBuoyancyForce * XMVectorSet(0.f, 1.f, 0.f, 0.f);

			_vector vTotalForceWithWaterNormal = vDragForce + vBuoyancyForceWithWaterNormal;
			_vector vTotalForceWithUpDirection = vDragForce + vBuoyancyForceWithUpDirection;

			netForce += vTotalForceWithUpDirection;
			netTorque += XMVector3Cross(vWorldPos, vTotalForceWithWaterNormal);
		}
	}

	_vector vLinearAcceleration = (1 / fMass) * netForce * m_fWaveTimeStep;
	// 각 속도 시간 미분
	_vector vAngularAcceleration = pRigidbodyCom->dwdt(netTorque) * m_fWaveTimeStep;
	
	pRigidbodyCom->Translate(vVelocity * m_fWaveTimeStep);
	pRigidbodyCom->Rotate(vAngularVelocity * m_fWaveTimeStep); 

	_float3 vNewVelocity;
	XMStoreFloat3(&vNewVelocity, vVelocity + vLinearAcceleration);
	pRigidbodyCom->Set_Velocity(vNewVelocity);

	_float3 vNewAngularVelocity;
	XMStoreFloat3(&vNewAngularVelocity, (vAngularVelocity + vAngularAcceleration) * m_fVelocityDampingCoefficient);
	pRigidbodyCom->Set_AngularVelocity(vNewAngularVelocity);
}

void COcean::Interpolation_Water(const float fX, const float fZ, float& fWaterHeight, XMFLOAT3& vWaterNormal)
{
	const float fReciprocalGridCellWidth = 1.0f / m_fGridCellWidth;

	// fractional index
	float fFractionalU = (fX + 0.5f * m_fGridEntireWidth) * fReciprocalGridCellWidth;
	float fFractionalV = (fZ + 0.5f * m_fGridEntireWidth) * fReciprocalGridCellWidth;

	// lower-left vertex of the enclosing grid cell
	const _int iGridCellX = _int(fFractionalU);
	const _int iGridCellZ = _int(fFractionalV);

	// interpolation coefficients
	const float fInterpolationA = fFractionalU - iGridCellX;
	const float fInterpolationB = fFractionalV - iGridCellZ;
	const float fInterpolationAB = fInterpolationA * fInterpolationB;

	// 그리드 범위를 벗어나는 경우
	if (iGridCellX < 0 || (_int)m_iSimulateGridWidth <= iGridCellX || iGridCellZ < 0 || (_int)m_iSimulateGridWidth <= iGridCellZ)
	{
		fWaterHeight = 0.f;
		vWaterNormal = { 0.f, 1.f, 0.f };
	}
	else
	{
		auto& vecSurfaceVertices = m_vecAnimatedSurfaceVertices;
		// Bilinear interpolation 쌍선형 보간
		// https://en.wikipedia.org/wiki/Bilinear_interpolation
		float fSurfaceHeightA = m_vecCurrentSurfaceHeight[iGridCellX * m_iSimulateGridWidth + iGridCellZ];
		float fSurfaceHeightB = m_vecCurrentSurfaceHeight[iGridCellX * m_iSimulateGridWidth + (iGridCellZ + 1)];
		float fSurfaceHeightC = m_vecCurrentSurfaceHeight[(iGridCellX + 1) * m_iSimulateGridWidth + iGridCellZ];
		float fSurfaceHeightD = m_vecCurrentSurfaceHeight[(iGridCellX + 1) * m_iSimulateGridWidth + (iGridCellZ + 1)];

		fWaterHeight = (1 - fInterpolationA - fInterpolationB + fInterpolationAB) * fSurfaceHeightA
			+ (fInterpolationB - fInterpolationAB) * fSurfaceHeightB
			+ (fInterpolationA - fInterpolationAB) * fSurfaceHeightC
			+ (fInterpolationAB)*fSurfaceHeightD;

		_vector vSurfaceNormalA = XMLoadFloat3(&vecSurfaceVertices[iGridCellX * m_iSimulateGridWidth + iGridCellZ].vNormal);
		_vector vSurfaceNormalB = XMLoadFloat3(&vecSurfaceVertices[iGridCellX * m_iSimulateGridWidth + (iGridCellZ + 1)].vNormal);
		_vector vSurfaceNormalC = XMLoadFloat3(&vecSurfaceVertices[(iGridCellX + 1) * m_iSimulateGridWidth + iGridCellZ].vNormal);
		_vector vSurfaceNormalD = XMLoadFloat3(&vecSurfaceVertices[(iGridCellX + 1) * m_iSimulateGridWidth + (iGridCellZ + 1)].vNormal);

		XMVECTOR vNormal = (1 - fInterpolationA - fInterpolationB + fInterpolationAB) * vSurfaceNormalA
			+ (fInterpolationB - fInterpolationAB) * vSurfaceNormalB
			+ (fInterpolationA - fInterpolationAB) * vSurfaceNormalC
			+ fInterpolationAB * vSurfaceNormalD;

		vNormal = XMVector3Normalize(vNormal);
		XMStoreFloat3(&vWaterNormal, vNormal);
	}
}

_bool COcean::Compute_WaterHeightA(_float fX, _float fZ, _float& fWaterHeight)
{
	const float fReciprocalGridCellWidth = 1.0f / m_fGridCellWidth;

	// fractional index
	float fFractionalU = (fX + 0.5f * m_fGridEntireWidth) * fReciprocalGridCellWidth;
	float fFractionalV = (fZ + 0.5f * m_fGridEntireWidth) * fReciprocalGridCellWidth;

	// lower-left vertex of the enclosing grid cell
	const _int iGridCellX = _int(fFractionalU);
	const _int iGridCellZ = _int(fFractionalV);

	// interpolation coefficients
	const float fInterpolationA = fFractionalU - iGridCellX;
	const float fInterpolationB = fFractionalV - iGridCellZ;
	const float fInterpolationAB = fInterpolationA * fInterpolationB;

	// 그리드 범위를 벗어나는 경우
	if (iGridCellX < 0 || (_int)m_iSimulateGridWidth <= iGridCellX || iGridCellZ < 0 || (_int)m_iSimulateGridWidth <= iGridCellZ)
	{
		return false;
	}

	auto& vecSurfaceVertices = m_vecAnimatedSurfaceVertices;
	float fSurfaceHeightA = m_vecCurrentSurfaceHeight[iGridCellX * m_iSimulateGridWidth + iGridCellZ];
	float fSurfaceHeightB = m_vecCurrentSurfaceHeight[iGridCellX * m_iSimulateGridWidth + (iGridCellZ + 1)];
	float fSurfaceHeightC = m_vecCurrentSurfaceHeight[(iGridCellX + 1) * m_iSimulateGridWidth + iGridCellZ];
	float fSurfaceHeightD = m_vecCurrentSurfaceHeight[(iGridCellX + 1) * m_iSimulateGridWidth + (iGridCellZ + 1)];

	fWaterHeight = (1 - fInterpolationA - fInterpolationB + fInterpolationAB) * fSurfaceHeightA
		+ (fInterpolationB - fInterpolationAB) * fSurfaceHeightB
		+ (fInterpolationA - fInterpolationAB) * fSurfaceHeightC
		+ (fInterpolationAB)*fSurfaceHeightD;

	return true;
}

HRESULT COcean::Add_Components()
{
	/* Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"),
		TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* Com_Transform */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_RigidbodyState"),
		TEXT("Com_RigidbodyState"), (CComponent**)&m_pRigidbodyStateCom)))
		return E_FAIL;

	/* Com_ComputeBuffer */
	COMPUTESHADERDESC tComputeShaderDesc;
	tComputeShaderDesc.iNumInputData = 3;
	tComputeShaderDesc.iInputStride[0] = sizeof(_float);
	tComputeShaderDesc.iInputStride[1] = sizeof(_float);
	tComputeShaderDesc.iInputStride[2] = sizeof(_float);
	tComputeShaderDesc.iNumInputElements[0] = m_iSimulateGridWidth * m_iSimulateGridWidth;
	tComputeShaderDesc.iNumInputElements[1] = m_iSimulateGridWidth * m_iSimulateGridWidth;
	tComputeShaderDesc.iNumInputElements[2] = m_iSimulateGridWidth * m_iSimulateGridWidth;

	void* pInputDatas[3] = { m_vecCurrentSurfaceHeight.data(), m_vecPrevSurfaceHeight.data(), m_vecDampingCoefficient.data() };
	tComputeShaderDesc.pInputData = pInputDatas;
	
	tComputeShaderDesc.iNumOutputData = 3;
	tComputeShaderDesc.iOutputStride[0] = sizeof(COMPUTEOUTPUT_SURFACEHEIGHT);
	tComputeShaderDesc.iNumOutputElements[0] = m_iSimulateGridWidth * m_iSimulateGridWidth;
	tComputeShaderDesc.bIsStructuredBuffer[0] = false;
	tComputeShaderDesc.iOutputStride[1] = sizeof(VTXPOSNORTEX);
	tComputeShaderDesc.iNumOutputElements[1] = m_iSimulateGridWidth * m_iSimulateGridWidth;
	tComputeShaderDesc.bIsStructuredBuffer[1] = true;
	tComputeShaderDesc.iOutputStride[2] = sizeof(_float3);
	tComputeShaderDesc.iNumOutputElements[2] = m_iQuadTreeGridWidth * m_iQuadTreeGridWidth;
	tComputeShaderDesc.bIsStructuredBuffer[2] = true;
	
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_ComputeBuffer"),
		TEXT("Com_ComputeBuffer"), (CComponent**)&m_pComputeBufferCom, &tComputeShaderDesc)))
		return E_FAIL;

	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Shader_Ocean"),
		TEXT("Com_Shader"), (CComponent**)&m_pShaderCom)))
		return E_FAIL;

	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Gradients"),
		TEXT("Com_Gradients_Texture"), (CComponent**)&m_pGradientsTexture)))
		return E_FAIL;

	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Moments"),
		TEXT("Com_Moments_Texture"), (CComponent**)&m_pMomentsTexture)))
		return E_FAIL;

	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Flowing_Water"),
		TEXT("Com_Texture_Flowing_Water"), (CComponent**)&m_pFlowingWaterTexture)))
		return E_FAIL;

	/* Com_Texture */
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Texture_Foam"),
		TEXT("Com_Texture_Foam"), (CComponent**)&m_pFoamTexture)))
		return E_FAIL;

	return S_OK;
}

void COcean::InitSurface()
{
	m_vecPrevSurfaceHeight.resize(m_iSimulateGridWidth * m_iSimulateGridWidth);
	m_vecCurrentSurfaceHeight.resize(m_iSimulateGridWidth * m_iSimulateGridWidth);
	m_vecDampingCoefficient.resize(m_iSimulateGridWidth* m_iSimulateGridWidth);
	m_vecAnimatedSurfaceVertices.resize(m_iSimulateGridWidth * m_iSimulateGridWidth);
	m_vecAnimatedSurfacePosition.resize(m_iQuadTreeGridWidth * m_iQuadTreeGridWidth);

	int iWidth, iHeight;
	std::vector<uint32_t> pixels;
	if (FAILED(CTextureUtil::LoadRawTexture("../Bin/Resources/Textures/HeightMaps/wind_gusts.png", pixels, iWidth, iHeight)))
	{
		MessageBox(g_hWnd, L"Failed to load surface height map", L"Error", MB_OK);
		return;
	}
	
	for (_uint i = 0; i < m_iSimulateGridWidth; ++i)
	{
		for (_uint j = 0; j < m_iSimulateGridWidth; ++j)
		{
			if (i * m_iSimulateGridWidth + j > (512 * 512))
			{
				m_vecDampingCoefficient[i * m_iSimulateGridWidth + j] = 1.f;
				continue;
			}
			
			m_vecPrevSurfaceHeight[i * m_iSimulateGridWidth + j] = pixels[(j * 4) + (i * iWidth * 4)] * 0.0003f;
			m_vecCurrentSurfaceHeight[i * m_iSimulateGridWidth + j] = pixels[(j * 4) + (i * iWidth * 4)] * -0.0003f;

			m_vecDampingCoefficient[i * m_iSimulateGridWidth + j] = 0.9999f;
		}
	}
}

void COcean::AnimateWater()
{
	m_pComputeBufferCom->Update_ShaderResourceView(m_vecCurrentSurfaceHeight.data(), 0);
	m_pComputeBufferCom->Update_ShaderResourceView(m_vecPrevSurfaceHeight.data(), 1);
	
	// 수면 정보 시뮬레이션
	m_pComputeBufferCom->Dispatch(m_pShaderCom, 1, 0, m_iPixelsPerThread, m_iPixelsPerThread, 1);
	m_pComputeBufferCom->StoreOutput(m_vecCurrentSurfaceHeight.data(), 0);
	
	// 정점 높이와 Normal 벡터를 갱신
	m_pComputeBufferCom->Dispatch(m_pShaderCom, 1, 1, m_iPixelsPerThread, m_iPixelsPerThread, 1);
	m_pComputeBufferCom->StoreOutput(m_vecAnimatedSurfaceVertices.data(), 1); // g_SurfaceVertices 가져오기

	m_pComputeBufferCom->Copy_OutputToShaderResourceView(1, 1);
	
	// 이전 물결상태 m_ppPrevSurfaceHeight와 현재 물결상태 m_ppCurrentSurfaceHeight의 값을 교환하여
	// 다음 시간 스텝에서 이전 물결상태를 현재 물결상태로 사용할 수 있도록 함
	std::swap(m_vecPrevSurfaceHeight, m_vecCurrentSurfaceHeight);
}

void COcean::TuneDampingCoefficient(_float fTimeDelta)
{
	m_fAmplificationAccTime += fTimeDelta;
	if (m_fAmplificationAccTime >= (m_bStorm ? m_fAmplificationInterval * 0.75f : m_fAmplificationInterval))
	{
		m_fDampingCoefficient = 1.f;

		m_fAmplificationLimitAccTime += fTimeDelta;
		if (m_fAmplificationLimitAccTime >= m_fAmplificationLimitInterval)
		{
			m_fDampingCoefficient = 0.9999f;

			m_fAmplificationAccTime = 0.f;
			m_fAmplificationLimitAccTime = 0.f;
		}
	}

#ifdef USE_IMGUI
	ImGui::SliderFloat("m_fAmplificationAccTime", &m_fAmplificationAccTime, 0.f, 60.f);
	ImGui::SliderFloat("m_fAmplificationLimitAccTime", &m_fAmplificationLimitAccTime, 0.f, 3.f);
#endif
}

COcean* COcean::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	COcean* pInstance = new COcean(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Created : COcean");
	}

	return pInstance;
}

CGameObject* COcean::Clone(void* pArg)
{
	COcean* pInstance = new COcean(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Cloned : COcean");
	}

	return pInstance;
}

void COcean::Free()
{
	__super::Free();

	CQuadTree::Destroy_QuadTree(m_pQuadTree);

	Safe_Release(m_pIBLTexture);
	Safe_Release(m_pGradientsTexture);
	Safe_Release(m_pMomentsTexture);
	Safe_Release(m_pFlowingWaterTexture);
	Safe_Release(m_pFoamTexture);

	Safe_Release(m_pShaderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pVIBufferCom);
	Safe_Release(m_pComputeBufferCom);
	Safe_Release(m_pRigidbodyStateCom);
}
