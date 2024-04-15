#include "stdafx.h"
#include "Sloop.h"

#include "GameInstance.h"
#include "CollisionManager.h"
#include "Light_Manager.h"

#include "Cannon.h"
#include "Wheal.h"
#include "Capstan.h"
#include "Barrel.h"
#include "Sloop_Ladder.h"

#include "Gui.h"

#include "GroupCollider.h"
#include "Effect.h"

#include "PoolObject.h"
#include "Kraken_Tentacle.h"

#include "Hull_Damage_Point.h"
#include "WaterPlane.h"
#include "Water_Splash_Point.h"

#include "CannonBall.h"

#include "Fox.h"

CSloop::CSloop(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CShip(pDevice, pContext)
{
}

CSloop::CSloop(const CShip& rhs)
	: CShip(rhs)
{
}

HRESULT CSloop::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CSloop::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	// Init BuoyancyPoints
	{
		auto& BuoyancyVertices = m_GeometryDesc.vecVertices;
		BuoyancyVertices.clear();
		BuoyancyVertices.reserve(POINT_END);

		auto& BuoyancyWeights = m_GeometryDesc.vecBuoyancyWeights;
		BuoyancyWeights.clear();
		BuoyancyWeights.reserve(POINT_END);

		_float3 vPosition;
		_float3 vNormal;

		// 부심
		XMStoreFloat3(&vNormal, XMVector3Normalize(XMVectorSet(0.f, -1.f, 0.f, 0.f)));
		BuoyancyVertices.emplace_back(VTXPOSNORTEX{ XMFLOAT3(0.f, -3.f, 0.f), vNormal, XMFLOAT2(0, 0) });
		BuoyancyWeights.emplace_back(30.f);

		// 선수
		vPosition = { 0.f, -3.f, 8.f };
		XMStoreFloat3(&vNormal, XMVector3Normalize(XMVectorSet(vPosition.x, vPosition.y, vPosition.z, 0.f)));
		BuoyancyVertices.emplace_back(VTXPOSNORTEX{ vPosition, vNormal, XMFLOAT2(0, 0) });
		BuoyancyWeights.emplace_back(15.f);

		// 선미
		vPosition = { 0.f, -3.f, -10.f };
		XMStoreFloat3(&vNormal, XMVector3Normalize(XMVectorSet(vPosition.x, vPosition.y, vPosition.z, 0.f)));
		BuoyancyVertices.emplace_back(VTXPOSNORTEX{ vPosition, vNormal, XMFLOAT2(0, 0) });
		BuoyancyWeights.emplace_back(10.f);
		
		// 좌현
		vPosition = { -4.f, -3.f, 0.f };
		XMStoreFloat3(&vNormal, XMVector3Normalize(XMVectorSet(vPosition.x, vPosition.y, vPosition.z, 0.f)));
		BuoyancyVertices.emplace_back(VTXPOSNORTEX{ vPosition, vNormal, XMFLOAT2(0, 0) });
		BuoyancyWeights.emplace_back(10.f);

		// 우현
		vPosition = { 4.f, -3.f, 0.f };
		XMStoreFloat3(&vNormal, XMVector3Normalize(XMVectorSet(vPosition.x, vPosition.y, vPosition.z, 0.f)));
		BuoyancyVertices.emplace_back(VTXPOSNORTEX{ vPosition, vNormal, XMFLOAT2(0, 0) });
		BuoyancyWeights.emplace_back(10.f);
	}

	// GroupCollider
	{
		CGameInstance* pGameInstance = CGameInstance::Get_Instance();
		Safe_AddRef(pGameInstance);

		CGroupCollider::GROUPCOLLIDERDESC GroupColliderDesc;
		GroupColliderDesc.pFilePath = TEXT("../Bin/Resources/Models/Ships/Sloop/Sloop_GroupCollider.col");
		GroupColliderDesc.pOwner = this;

		m_pGroupCollider = static_cast<CGroupCollider*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_GroupCollider"), &GroupColliderDesc));
		m_pGroupCollider->Subscribe_Collision_Enter_Callback(std::bind(&CSloop::OnCollisionEnter, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

		Safe_Release(pGameInstance);
	}

	if (FAILED(Add_Components()))
		return E_FAIL;

	m_pVehicleColliderComs->Subscribe_Collision_Enter_Callback(std::bind(&CSloop::OnVehicleCollisionEnter, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	m_pRigidbodyCom->Set_State(CRigidbodyState::STATE_POSITION, XMVectorSet(0.f, 10.f, 0.f, 1.f));

	if (FAILED(Add_ShipParts()))
		return E_FAIL;

	if (FAILED(Add_BattleProps()))
		return E_FAIL;

	if (FAILED(Add_WaterSplashPoints()))
		return E_FAIL;

	if (FAILED(Add_Pet()))
		return E_FAIL;

	for (_uint i = 0; i < SLOOP_ANIM_MODEL_END; ++i)
		m_pSloopAnimModelComs[i]->Play_Animation(0);

	m_pSloopAnimModelComs[SLOOP_SAIL]->SetUp_Animation(ANIMATION_SAIL_LAX_FOLD, 0.1f, false);

	m_ShadowMappingModels.reserve(SLOOP_MODEL_END);
	m_ShadowMappingModels = { 
		SLOOP_CANOPY, 
		SLOOP_CROWS_NEST,
		SLOOP_HULL, 
		SLOOP_HULL_DAMAGE,
		SLOOP_MAST,
		SLOOP_MAST_CROSSBEAM,
		SLOOP_RUDDER,
		SLOOP_STAIRS,
	};

	return S_OK;
}

void CSloop::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (m_bMovable)
	{
		_vector vVelocity = XMLoadFloat3(&m_pRigidbodyCom->Get_Velocity());
		_vector vAngularVelocity = XMLoadFloat3(&m_pRigidbodyCom->Get_AngularVelocity());
		_vector vForward = m_pRigidbodyCom->Get_State(CRigidbodyState::STATE_LOOK);
		vForward = XMVector3Normalize(XMVectorSetY(vForward, 3.f));

		CWheel* pWheel = static_cast<CWheel*>(m_ShipParts.find(TEXT("Part_Wheel"))->second);

		vAngularVelocity += XMVectorSet(0.f, -pWheel->Get_RotationAmount() * fTimeDelta * 0.01f, 0.f, 0.f);
		vForward = XMVectorScale(vForward, 10 * fTimeDelta);

		vVelocity += vForward;

		_float3 vNewVelocity;
		XMStoreFloat3(&vNewVelocity, vVelocity);
		m_pRigidbodyCom->Set_Velocity(vNewVelocity);

		_float3 vNewAngularVelocity;
		XMStoreFloat3(&vNewAngularVelocity, vAngularVelocity);
		m_pRigidbodyCom->Set_AngularVelocity(vNewAngularVelocity);
	}

	if (m_bOnHited)
	{
		//m_bOnHited = false;
		m_fHitAngularVelocityAccTime += fTimeDelta;
		if (m_fHitAngularVelocityAccTime >= 1.f)
		{
			m_bOnHited = false;
			m_fHitAngularVelocityAccTime = 0.f;
		}

		_vector vAngularVelocity = XMLoadFloat3(&m_pRigidbodyCom->Get_AngularVelocity());
		vAngularVelocity += XMVectorSet(-2.5f * fTimeDelta, 5.f * fTimeDelta, 2.5f * fTimeDelta, 0.f);

		_float3 vNewAngularVelocity;
		XMStoreFloat3(&vNewAngularVelocity, vAngularVelocity);
		m_pRigidbodyCom->Set_AngularVelocity(vNewAngularVelocity);
	}

	m_pSloopAnimModelComs[SLOOP_SAIL]->Play_Animation(fTimeDelta);
	
	if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_1) & 0x8000)
	{
		m_bMovable = false;
		m_pSloopAnimModelComs[SLOOP_SAIL]->SetUp_Animation(0, 0.5f, false);
	}
	
	if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_2) & 0x8000)
		m_pSloopAnimModelComs[SLOOP_SAIL]->SetUp_Animation(1, 0.5f, false);

	if (CGameInstance::Get_Instance()->Get_DIKeyState(DIK_3) & 0x8000)
	{
		m_bMovable = true;
		m_pSloopAnimModelComs[SLOOP_SAIL]->SetUp_Animation(2, 0.5f, true);
	}

#ifdef USE_IMGUI
	GUITEXT GuiText;
	GuiText.vPosition = { 5.f, 20.f };
	GuiText.vColor = { 1.f, 1.f, 1.f, 1.f };

	XMFLOAT3 vPosition;
	XMStoreFloat3(&vPosition, m_pRigidbodyCom->Get_State(CRigidbodyState::STATE_POSITION));
	GuiText.Content = "Ship Position: " +
		to_string(vPosition.x) + ", " +
		to_string(vPosition.y) + ", " +
		to_string(vPosition.z);

	CGui::Get_Instance()->RenderText(GuiText);

	ImGui::Begin("Ship Animate Parameter");

	//if (ImGui::SliderFloat3("Ship Geometry Scale", &m_fGeometryScale.x, 0.f, 10.f))
	//	Update_Geometry();

	_float fMass = m_pRigidbodyCom->Get_Mass();
	if (ImGui::SliderFloat("Ship Mass", &fMass, 0.f, 50000.f))
		m_pRigidbodyCom->Set_Mass(fMass);

	ImGui::End();
#endif

	m_pGroupCollider->Tick(m_pRigidbodyCom->Get_WorldMatrix());
	m_pSloopWaterOcc->Tick(fTimeDelta);

	if (m_bPetEnable)
		m_pPet->Tick(fTimeDelta);

	// Sloop 충돌체
	m_pVehicleColliderComs->Tick(m_pRigidbodyCom->Get_WorldMatrix());

	for (auto& Pair : m_ShipParts)
		Pair.second->Tick(fTimeDelta);

	for (auto* pDamagePoint : m_vecDamagePoints)
	{
		pDamagePoint->Tick(fTimeDelta);

		if (pDamagePoint->Is_Activate())
			m_pWaterPlane->Add_SurfaceHeight(fTimeDelta * 0.005f);
	}

	m_pWaterPlane->Tick(fTimeDelta);

	for (auto* pSplashPoint : m_vecWaterSplashPoints)
	{
		pSplashPoint->Tick(fTimeDelta);
	}

#ifdef _DEBUG

	for (_uint i = 0; i < POINT_END; ++i)
	{
		m_pBuoyancyPointColliderCom[i]->Tick(m_pRigidbodyCom->Get_WorldMatrix());
	}

#endif
}

void CSloop::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);

	for (auto& Pair : m_ShipParts)
		Pair.second->Late_Tick(fTimeDelta);

	for (auto& Pair : m_ShipParts)
	{
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONBLEND, Pair.second);
		m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_SHADOWDEPTH, Pair.second);
	}

	for (auto* pDamagePoint : m_vecDamagePoints)
	{
		pDamagePoint->Late_Tick(fTimeDelta);
	}

	m_pWaterPlane->Late_Tick(fTimeDelta);

	for (auto* pSplashPoint : m_vecWaterSplashPoints)
	{
		pSplashPoint->Late_Tick(fTimeDelta);
	}

	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_NONBLEND, this);
	m_pRendererCom->Add_RenderGroup(CRenderer::RENDER_SHADOWDEPTH, this);
	m_pSloopWaterOcc->Late_Tick(fTimeDelta);

	if (m_bPetEnable)
		m_pPet->Late_Tick(fTimeDelta);
}

HRESULT CSloop::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	// NonAnim Model
	{
		CGameInstance* pGameInstance = CGameInstance::Get_Instance();
		Safe_AddRef(pGameInstance);

		if (FAILED(m_pMeshShaderCom->Bind_Matrix("g_ViewMatrix", &pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
			return E_FAIL;
		if (FAILED(m_pMeshShaderCom->Bind_Matrix("g_ProjMatrix", &pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
			return E_FAIL;

		Safe_Release(pGameInstance);

		for (_uint i = 0; i < SLOOP_MODEL_END; ++i)
		{
			_uint iNumMeshes = m_pSloopModelComs[i]->Get_NumMeshes();
			if (m_pSloopModelComs[i]->Is_StaticModel())
			{
				if (FAILED(m_pRigidbodyCom->Bind_ShaderResource(m_pMeshShaderCom, "g_WorldMatrix")))
					return E_FAIL;
			}
			else
			{
				if (FAILED(m_pSloopModelComs[i]->Bind_ShaderResource(m_pRigidbodyCom->Get_WorldMatrix(), m_pMeshShaderCom, "g_WorldMatrix")))
					return E_FAIL;
			}

			for (_uint j = 0; j < iNumMeshes; ++j)
			{
				if (FAILED(m_pSloopModelComs[i]->Bind_ShaderResourceView(m_pMeshShaderCom, "g_DiffuseTexture", j, TextureType_DIFFUSE)))
					return E_FAIL;
				if (FAILED(m_pSloopModelComs[i]->Bind_ShaderResourceView(m_pMeshShaderCom, "g_NormalTexture", j, TextureType_NORMALS)))
					return E_FAIL;

				m_pMeshShaderCom->Begin(Mesh_NonSpecular);

				m_pSloopModelComs[i]->Render(j);
			}
		}
	}

	// AnimModel
	{
		CGameInstance* pGameInstance = CGameInstance::Get_Instance();
		Safe_AddRef(pGameInstance);

		if (FAILED(m_pAnimMeshShaderCom->Bind_Matrix("g_ViewMatrix", &pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_VIEW))))
			return E_FAIL;
		if (FAILED(m_pAnimMeshShaderCom->Bind_Matrix("g_ProjMatrix", &pGameInstance->Get_Transform_Float4x4(CPipeLine::D3DTS_PROJ))))
			return E_FAIL;
		
		Safe_Release(pGameInstance);
		
		for (_uint i = 0; i < SLOOP_ANIM_MODEL_END; ++i)
		{
			_uint iNumMeshes = m_pSloopAnimModelComs[i]->Get_NumMeshes();
			if (m_pSloopAnimModelComs[i]->Is_StaticModel())
			{
				if (FAILED(m_pRigidbodyCom->Bind_ShaderResource(m_pAnimMeshShaderCom, "g_WorldMatrix")))
					return E_FAIL;
			}
			else
			{
				if (FAILED(m_pSloopAnimModelComs[i]->Bind_ShaderResource(m_pRigidbodyCom->Get_WorldMatrix(), m_pAnimMeshShaderCom, "g_WorldMatrix")))
					return E_FAIL;
			}

			for (size_t j = 0; j < iNumMeshes; ++j)
			{
				if (FAILED(m_pSloopAnimModelComs[i]->Bind_ShaderResourceView(m_pAnimMeshShaderCom, "g_DiffuseTexture", j, TextureType_DIFFUSE)))
					return E_FAIL;

				if (FAILED(m_pSloopAnimModelComs[i]->Bind_BoneMatrices(m_pAnimMeshShaderCom, "g_BoneMatrices", j)))
					return E_FAIL;

				if (i == SLOOP_SAIL)
					m_pAnimMeshShaderCom->Begin(AnimMesh_Sail);
				else
					m_pAnimMeshShaderCom->Begin(0);

				m_pSloopAnimModelComs[i]->Render(j);
			}
		}
	}

	return S_OK;
}

HRESULT CSloop::Render_ShadowDepth()
{
	if (FAILED(__super::Render()))
		return E_FAIL;

	// NonAnim Model
	{
		CGameInstance* pGameInstance = CGameInstance::Get_Instance();
		Safe_AddRef(pGameInstance);

		if (FAILED(m_pMeshShaderCom->Bind_Matrix("g_ViewMatrix", pGameInstance->Get_DirectionalLight_Float4x4(CLight_Manager::D3DTS_VIEW))))
			return E_FAIL;
		if (FAILED(m_pMeshShaderCom->Bind_Matrix("g_ProjMatrix", pGameInstance->Get_DirectionalLight_Float4x4(CLight_Manager::D3DTS_PROJ))))
			return E_FAIL;

		Safe_Release(pGameInstance);

		for (SLOOP_MODEL ShadowDepthModel : m_ShadowMappingModels)
		{
			_float fDepthElipse = 0.f;
			if (ShadowDepthModel == SLOOP_HULL_DAMAGE)
				fDepthElipse = -1.f;

			if (FAILED(m_pMeshShaderCom->Bind_Float("g_fDepthEpsilon", fDepthElipse)))
				return E_FAIL;

			_uint iNumMeshes = m_pSloopModelComs[ShadowDepthModel]->Get_NumMeshes();
			if (m_pSloopModelComs[ShadowDepthModel]->Is_StaticModel())
			{
				if (FAILED(m_pRigidbodyCom->Bind_ShaderResource(m_pMeshShaderCom, "g_WorldMatrix")))
					return E_FAIL;
			}
			else
			{
				if (FAILED(m_pSloopModelComs[ShadowDepthModel]->Bind_ShaderResource(m_pRigidbodyCom->Get_WorldMatrix(), m_pMeshShaderCom, "g_WorldMatrix")))
					return E_FAIL;
			}

			for (_uint j = 0; j < iNumMeshes; ++j)
			{
				m_pMeshShaderCom->Begin(Mesh_ShadowDepth);

				m_pSloopModelComs[ShadowDepthModel]->Render(j);
			}
		}
	}

	// AnimModel
	{
		CGameInstance* pGameInstance = CGameInstance::Get_Instance();
		Safe_AddRef(pGameInstance);

		if (FAILED(m_pAnimMeshShaderCom->Bind_Matrix("g_ViewMatrix", pGameInstance->Get_DirectionalLight_Float4x4(CLight_Manager::D3DTS_VIEW))))
			return E_FAIL;
		if (FAILED(m_pAnimMeshShaderCom->Bind_Matrix("g_ProjMatrix", pGameInstance->Get_DirectionalLight_Float4x4(CLight_Manager::D3DTS_PROJ))))
			return E_FAIL;

		Safe_Release(pGameInstance);

		for (_uint i = 0; i < SLOOP_ANIM_MODEL_END; ++i)
		{
			_uint iNumMeshes = m_pSloopAnimModelComs[i]->Get_NumMeshes();
			if (m_pSloopAnimModelComs[i]->Is_StaticModel())
			{
				if (FAILED(m_pRigidbodyCom->Bind_ShaderResource(m_pAnimMeshShaderCom, "g_WorldMatrix")))
					return E_FAIL;
			}
			else
			{
				if (FAILED(m_pSloopAnimModelComs[i]->Bind_ShaderResource(m_pRigidbodyCom->Get_WorldMatrix(), m_pAnimMeshShaderCom, "g_WorldMatrix")))
					return E_FAIL;
			}

			for (size_t j = 0; j < iNumMeshes; ++j)
			{
				if (FAILED(m_pSloopAnimModelComs[i]->Bind_BoneMatrices(m_pAnimMeshShaderCom, "g_BoneMatrices", j)))
					return E_FAIL;

				if (i == SLOOP_SAIL)
					m_pAnimMeshShaderCom->Begin(AnimMesh_ShadowDepth_NonCull);
				else
					m_pAnimMeshShaderCom->Begin(0);

				m_pSloopAnimModelComs[i]->Render(j);
			}
		}
	}

	return S_OK;
}

void CSloop::Enable_Standing_Colliders(_bool bEnable)
{
	m_pGroupCollider->Set_Enable(bEnable);
}

_float CSloop::Compute_WaterPlaneHeight(_fvector vPosition)
{
	return m_pWaterPlane->Compute_Height(vPosition);
}

_float CSloop::Get_WaterPlaneHeight()
{
	return m_pWaterPlane->Get_Height();
}

void CSloop::Add_SurfaceHeight(_float fAmount)
{
	m_pWaterPlane->Add_SurfaceHeight(fAmount);
}

void CSloop::Put_Pet()
{
	m_bPetEnable = true;
}

void CSloop::OnCollisionEnter(CCollider* pOwn, CCollider* pOther, CCollisionManager::COLLISION_MASK eMask)
{
	if (eMask == CCollisionManager::COLLISION_CANNONBALL)
	{
		CPoolObject* pCannonBall = static_cast<CPoolObject*>(pOther->Get_Owner());
		CRigidbody* pRigidbody = static_cast<CRigidbody*>(pCannonBall->Find_Component(TEXT("Com_Rigidbody")));
		_vector vDirection = XMLoadFloat3(&pRigidbody->Get_Velocity());
		_vector vCannonBallPosition = pRigidbody->Get_State(CRigidbodyState::STATE_POSITION);

		_float3 vNormals[6];
		pOwn->Get_Normals(vNormals);

		_vector vLook, vRight, vUp;

		_float fMinDot = FLT_MAX;
		for (int i = 0; i < 6; i++)
		{
			XMVECTOR vNormal = XMLoadFloat3(&vNormals[i]);
			float fDot = XMVectorGetX(XMVector3Dot(vNormal, vDirection));

			if (fDot < fMinDot)
			{
				fMinDot = fDot;
				vLook = vNormal;
			}
		}

		vRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook));
		vUp = XMVector3Normalize(XMVector3Cross(vRight, vLook));

		_float4x4 EffectMatrix;
		XMStoreFloat4((_float4*)EffectMatrix.m[0], XMVectorScale(vRight, 2.f));
		XMStoreFloat4((_float4*)EffectMatrix.m[1], XMVectorScale(vUp, 2.f));
		XMStoreFloat4((_float4*)EffectMatrix.m[2], XMVectorScale(vLook, 2.f));
		XMStoreFloat4((_float4*)EffectMatrix.m[3], vCannonBallPosition);

		auto* pGameInstance = CGameInstance::Get_Instance();
		Safe_AddRef(pGameInstance);
		// Create Smoke Effect
		{
			CEffect::EFFECTDESC EffectDesc;
			XMStoreFloat3(&EffectDesc.vPosition, vCannonBallPosition);
			EffectDesc.vWorldMatrix = EffectMatrix;

			pGameInstance->Acquire(TEXT("Pool_CannonHitEffect"), &EffectDesc);

			// Cross 이펙트
			*(_float4*)EffectDesc.vWorldMatrix.m[0] = *(_float4*)EffectMatrix.m[2];
			*(_float4*)EffectDesc.vWorldMatrix.m[2] = *(_float4*)EffectMatrix.m[0];
			pGameInstance->Acquire(TEXT("Pool_CannonHitEffect"), &EffectDesc);
		}
		// Create Debris Effect
		{
			CEffect::EFFECTDESC EffectDesc;
			XMStoreFloat3(&EffectDesc.vPosition, vCannonBallPosition);
			XMStoreFloat4((_float4*)EffectMatrix.m[1], XMVectorScale(vUp, -2.f));
			EffectDesc.vWorldMatrix = EffectMatrix;

			pGameInstance->Acquire(TEXT("Pool_WoodDebrisEffect"), &EffectDesc);
		}

		if (static_cast<CCannonBall*>(pCannonBall)->Get_CannonType() == CANNON_FAR)
		{
			pGameInstance->PlaySFX(TEXT("sfx_cannon_wood_impact_solo_02.wav"), CHANNEL_ENVIRONMENT_1, 2.f, true);
		}

		Safe_Release(pGameInstance);

		pCannonBall->Reclaim();
	}
	else if (eMask == CCollisionManager::COLLISION_KRAKEN_HEAD)
	{
		if (false == static_cast<CKraken_Tentacle*>(pOther->Get_Owner())->Get_Attackable())
			return;

		auto* pGameInstance = CGameInstance::Get_Instance();
		Safe_AddRef(pGameInstance);

		// Create Debris Effect
		{
			_vector vHeadPosition = pOther->Get_Center();
			_vector vDirection = vHeadPosition - pOwn->Get_Center();

			_float3 vNormals[6];
			pOwn->Get_Normals(vNormals);

			_vector vLook, vRight, vUp;

			_float fMinDot = FLT_MAX;
			for (int i = 0; i < 6; i++)
			{
				XMVECTOR vNormal = XMLoadFloat3(&vNormals[i]);
				float fDot = XMVectorGetX(XMVector3Dot(vNormal, vDirection));

				if (fDot < fMinDot)
				{
					fMinDot = fDot;
					vLook = vNormal;
				}
			}

			vRight = XMVector3Normalize(XMVector3Cross(XMVectorSet(0.f, 1.f, 0.f, 0.f), vLook));
			vUp = XMVector3Normalize(XMVector3Cross(vRight, vLook));

			_float4x4 EffectMatrix;
			XMStoreFloat4((_float4*)EffectMatrix.m[0], XMVectorScale(vRight, 2.f));
			XMStoreFloat4((_float4*)EffectMatrix.m[1], XMVectorScale(vUp, 2.f));
			XMStoreFloat4((_float4*)EffectMatrix.m[2], XMVectorScale(vLook, 2.f));
			XMStoreFloat4((_float4*)EffectMatrix.m[3], vHeadPosition);

			CEffect::EFFECTDESC EffectDesc;
			XMStoreFloat3(&EffectDesc.vPosition, vHeadPosition);
			XMStoreFloat4((_float4*)EffectMatrix.m[1], XMVectorScale(vUp, -2.f));
			EffectDesc.vWorldMatrix = EffectMatrix;

			pGameInstance->Acquire(TEXT("Pool_WoodDebrisEffect"), &EffectDesc);
		}

		pGameInstance->PlaySFX(TEXT("sfx_cannon_wood_impact_solo_02.wav"), CHANNEL_CANNON_NEAR, 2.f, true);

		m_bOnHited = true;

		Safe_Release(pGameInstance);
	}
}

void CSloop::OnVehicleCollisionEnter(CCollider* pOwn, CCollider* pOther, CCollisionManager::COLLISION_MASK eMask)
{
	if (eMask == CCollisionManager::COLLISION_CANNONBALL)
	{
		CCannonBall* pCannonBall = static_cast<CCannonBall*>(pOther->Get_Owner());
		if (pCannonBall->Get_CannonType() == CANNON_FAR)
		{
			CGameInstance* pGameInstance = CGameInstance::Get_Instance();
			Safe_AddRef(pGameInstance);

			pGameInstance->PlaySFX(TEXT("sfx_cannon_whizz_01.wav"), CHANNEL_ENVIRONMENT_0, 2.f, true);

			Safe_Release(pGameInstance);
		}
	}
}

HRESULT CSloop::Add_Components()
{
	/* Com_Renderer */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Renderer"),
		TEXT("Com_Renderer"), (CComponent**)&m_pRendererCom)))
		return E_FAIL;

	/* Com_AABB */
	CCollider::COLLIDERDESC		ColliderDesc;
	ZeroMemory(&ColliderDesc, sizeof ColliderDesc);

	ColliderDesc.vSize = _float3(4.f, 6.f, 8.f);
	ColliderDesc.vPosition = _float3(0.f, 0.f, 0.f);

	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_AABB"),
		TEXT("Com_AABB"), (CComponent**)&m_pColliderCom, &ColliderDesc)))
		return E_FAIL;

	/* Com_Rigidbody */
	RIGIDBODYDESC RigidbodyDesc;
	RigidbodyDesc.fMass = 15000.f;
	RigidbodyDesc.TransformState.fSpeedPerSec = 10.f;
	RigidbodyDesc.TransformState.fRotationPerSec = 3.14f;
	RigidbodyDesc.vDimension = XMFLOAT3(8.f, 3.f, 20.f);
	RigidbodyDesc.vInertia = { 0.f, 0.f, 0.f };
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Rigidbody"),
		TEXT("Com_Rigidbody"), (CComponent**)&m_pRigidbodyCom, &RigidbodyDesc)))
		return E_FAIL;

	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxMesh"),
		TEXT("Com_MeshShader"), (CComponent**)&m_pMeshShaderCom)))
		return E_FAIL;

	/* Com_Shader */
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Shader_VtxAnimMesh"),
		TEXT("Com_Anim_Shader"), (CComponent**)&m_pAnimMeshShaderCom)))
		return E_FAIL;

	/* Com_Models */
	if (FAILED(Add_Ship_Models()))
		return E_FAIL;

	/* Com_Sloop_Vehicle_Colliders */
	ZeroMemory(&ColliderDesc, sizeof ColliderDesc);
	ColliderDesc.vPosition = _float3(0.f, 3.f, 1.5f);
	ColliderDesc.vSize = _float3(8.f, 12.f, 32.5f);
	ColliderDesc.vRotation = _float3(0.f, 0.f, 0.f);
	ColliderDesc.eMask = CCollisionManager::COLLISION_VEHICLE;
	ColliderDesc.pOwner = this;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_OBB"),
		TEXT("Com_Sloop_Vehicle_Collider_Hull"), (CComponent**)&m_pVehicleColliderComs, &ColliderDesc)))
		return E_FAIL;

#ifdef _DEBUG
	/* Com_BuoyancyPoint */

	ZeroMemory(&ColliderDesc, sizeof ColliderDesc);
	ColliderDesc.fRadius = 2.f;
	ColliderDesc.vPosition = m_GeometryDesc.vecVertices[POINT_COB].vPosition;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Point_COB"), (CComponent**)&m_pBuoyancyPointColliderCom[POINT_COB], &ColliderDesc)))
		return E_FAIL;

	ZeroMemory(&ColliderDesc, sizeof ColliderDesc);
	ColliderDesc.fRadius = 2.f;
	ColliderDesc.vPosition = m_GeometryDesc.vecVertices[POINT_BOW].vPosition;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Point_Bow"), (CComponent**)&m_pBuoyancyPointColliderCom[POINT_BOW], &ColliderDesc)))
		return E_FAIL;

	ZeroMemory(&ColliderDesc, sizeof ColliderDesc);
	ColliderDesc.fRadius = 2.f;
	ColliderDesc.vPosition = m_GeometryDesc.vecVertices[POINT_STERN].vPosition;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Point_Stern"), (CComponent**)&m_pBuoyancyPointColliderCom[POINT_STERN], &ColliderDesc)))
		return E_FAIL;

	ZeroMemory(&ColliderDesc, sizeof ColliderDesc);
	ColliderDesc.fRadius = 2.f;
	ColliderDesc.vPosition = m_GeometryDesc.vecVertices[POINT_PORT].vPosition;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Point_Port"), (CComponent**)&m_pBuoyancyPointColliderCom[POINT_PORT], &ColliderDesc)))
		return E_FAIL;

	ZeroMemory(&ColliderDesc, sizeof ColliderDesc);
	ColliderDesc.fRadius = 2.f;
	ColliderDesc.vPosition = m_GeometryDesc.vecVertices[POINT_STARBOARD].vPosition;
	if (FAILED(__super::Add_Component(LEVEL_STATIC, TEXT("Prototype_Component_Collider_Sphere"),
		TEXT("Com_Point_Starboard"), (CComponent**)&m_pBuoyancyPointColliderCom[POINT_STARBOARD], &ColliderDesc)))
		return E_FAIL;
	
#endif

	return S_OK;
}

HRESULT CSloop::Add_ShipParts()
{
	CGameInstance* pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);

	// Right Cannon
	{
		CGameObject* pPartObject = nullptr;

		CCannon::CANNONDESC			CannonDesc;
		CannonDesc.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();
		XMStoreFloat4x4(&CannonDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(90.f)) * XMMatrixTranslation(2.8f, 1.6f, 3.f));

		XMStoreFloat4x4(&CannonDesc.CannonBarrelDesc.PivotMatrix, XMMatrixTranslation(2.8f, 1.6f, 3.f));
		XMStoreFloat4x4(&CannonDesc.CameraPivotMatrix, XMMatrixTranslation(3.1f, 1.5f, 3.f));

		CannonDesc.eCannonType = CANNON_NEAR;
		CannonDesc.eChannelID = CHANNEL_CANNON_NEAR;

		CannonDesc.TooltipPivot = _float3(0.f, 1.f, -0.8f);
		pPartObject = pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Cannon"), &CannonDesc);
		if (nullptr == pPartObject)
			return E_FAIL;

		m_ShipParts.emplace(TEXT("Part_Cannon_Right"), pPartObject);
	}

	// Left Cannon
	{
		CGameObject* pPartObject = nullptr;

		CCannon::CANNONDESC			CannonDesc;
		CannonDesc.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();
		XMStoreFloat4x4(&CannonDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(-90.f)) * XMMatrixTranslation(-2.8f, 1.6f, 3.f));

		XMStoreFloat4x4(&CannonDesc.CannonBarrelDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(180.f)) * XMMatrixTranslation(-2.8f, 1.6f, 3.f));
		XMStoreFloat4x4(&CannonDesc.CameraPivotMatrix, XMMatrixTranslation(3.1f, 1.5f, -3.f) * XMMatrixRotationY(XMConvertToRadians(180.f)));
		
		CannonDesc.eCannonType = CANNON_NEAR;
		CannonDesc.eChannelID = CHANNEL_CANNON_NEAR;
		
		CannonDesc.TooltipPivot = _float3(0.f, 1.f, -0.8f);
		pPartObject = pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Cannon"), &CannonDesc);
		if (nullptr == pPartObject)
			return E_FAIL;

		m_ShipParts.emplace(TEXT("Part_Cannon_Left"), pPartObject);
	}

	// Wheel
	{
		CGameObject* pPartObject = nullptr;

		CWheel::WHEELDESC			WheelDesc;
		WheelDesc.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();
		XMStoreFloat4x4(&WheelDesc.PivotMatrix, XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.f), XMConvertToRadians(90.f), XMConvertToRadians(-90.f)) *
			XMMatrixTranslation(0.f, 3.15f, -3.5f));

		XMStoreFloat4x4(&WheelDesc.PlayerPivotMatrix, XMMatrixTranslation(0.f, 3.17f, -4.39f));

		pPartObject = pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Wheel"), &WheelDesc);
		if (nullptr == pPartObject)
			return E_FAIL;

		m_ShipParts.emplace(TEXT("Part_Wheel"), pPartObject);
	}

	// Capstan
	{
		CGameObject* pPartObject = nullptr;

		CCapstan::CAPSTANDESC			CapstanDesc;
		CapstanDesc.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();
		XMStoreFloat4x4(&CapstanDesc.PivotMatrix, XMMatrixTranslation(0.f, 3.15f, -7.3f));

		XMStoreFloat4x4(&CapstanDesc.PlayerPivotMatrix, XMMatrixTranslation(0.f, 4.f, -4.5f));

		pPartObject = pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Capstan"), &CapstanDesc);
		if (nullptr == pPartObject)
			return E_FAIL;

		m_ShipParts.emplace(TEXT("Part_Capstan"), pPartObject);
	}

	// Barrel_CannonBall_Under
	{
		CGameObject* pPartObject = nullptr;

		CBarrel::BARRELDESC			BarrelDesc;
		BarrelDesc.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();
		XMStoreFloat4x4(&BarrelDesc.PivotMatrix, XMMatrixTranslation(-3.f, 1.6f, -2.75f));

		BarrelDesc.eBarrelType = BARREL_CANNONBALL;

		pPartObject = pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Barrel"), &BarrelDesc);
		if (nullptr == pPartObject)
			return E_FAIL;

		m_ShipParts.emplace(TEXT("Part_Barrel_CannonBall_Under"), pPartObject);
	}

	// Barrel_CannonBall_Upper
	{
		CGameObject* pPartObject = nullptr;

		CBarrel::BARRELDESC			BarrelDesc;
		BarrelDesc.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();
		XMStoreFloat4x4(&BarrelDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(30.f)) * XMMatrixTranslation(-3.f, 2.54f, -2.75f));

		BarrelDesc.eBarrelType = BARREL_CANNONBALL;

		pPartObject = pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Barrel"), &BarrelDesc);
		if (nullptr == pPartObject)
			return E_FAIL;

		m_ShipParts.emplace(TEXT("Part_Barrel_CannonBall_Upper"), pPartObject);
	}

	// Barrel_Food_Lower
	{
		CGameObject* pPartObject = nullptr;

		CBarrel::BARRELDESC			BarrelDesc;
		BarrelDesc.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();
		XMStoreFloat4x4(&BarrelDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(130.f)) * XMMatrixTranslation(-2.f, -0.9f, 3.75f));

		BarrelDesc.eBarrelType = BARREL_FOOD;

		pPartObject = pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Barrel"), &BarrelDesc);
		if (nullptr == pPartObject)
			return E_FAIL;

		m_ShipParts.emplace(TEXT("Part_Barrel_Food_Lower"), pPartObject);
	}

	// Barrel_Food_Upper
	{
		CGameObject* pPartObject = nullptr;

		CBarrel::BARRELDESC			BarrelDesc;
		BarrelDesc.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();
		XMStoreFloat4x4(&BarrelDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(150.f)) * XMMatrixTranslation(-2.f, 0.f, 3.75f));

		BarrelDesc.eBarrelType = BARREL_FOOD;

		pPartObject = pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Barrel"), &BarrelDesc);
		if (nullptr == pPartObject)
			return E_FAIL;

		m_ShipParts.emplace(TEXT("Part_Barrel_Food_Upper"), pPartObject);
	}

	// Barrel_Wood
	{
		CGameObject* pPartObject = nullptr;

		CBarrel::BARRELDESC			BarrelDesc;
		BarrelDesc.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();
		XMStoreFloat4x4(&BarrelDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(140.f)) * XMMatrixTranslation(0.f, 0.2f, 3.8f));

		BarrelDesc.eBarrelType = BARREL_WOOD;

		pPartObject = pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Barrel"), &BarrelDesc);
		if (nullptr == pPartObject)
			return E_FAIL;

		m_ShipParts.emplace(TEXT("Part_Barrel_Wood"), pPartObject);
	}

	// Ladder_Right
	{
		CGameObject* pPartObject = nullptr;

		CLadder::LADDERDESC			LadderDesc;
		LadderDesc.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();
		XMStoreFloat4x4(&LadderDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(-90.f)) * XMMatrixTranslation(3.8f, 0.1f, -3.75));

		pPartObject = pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Sloop_Ladder"), &LadderDesc);
		if (nullptr == pPartObject)
			return E_FAIL;

		m_ShipParts.emplace(TEXT("Part_Ladder_Right"), pPartObject);
	}

	// Ladder_Left
	{
		CGameObject* pPartObject = nullptr;

		CLadder::LADDERDESC			LadderDesc;
		LadderDesc.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();
		XMStoreFloat4x4(&LadderDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(90.f)) * XMMatrixTranslation(-3.8f, 0.1f, -3.75));

		pPartObject = pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Sloop_Ladder"), &LadderDesc);
		if (nullptr == pPartObject)
			return E_FAIL;

		m_ShipParts.emplace(TEXT("Part_Ladder_Left"), pPartObject);
	}

	// Ladder_Left
	{
		m_pSloopWaterOcc = pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Sloop_Water_Occ"), (void*)m_pRigidbodyCom->Get_WorldMatrixPtr());
		if (nullptr == m_pSloopWaterOcc)
			return E_FAIL;
	}

	
	Safe_Release(pGameInstance);

	return S_OK;
}

HRESULT CSloop::Add_BattleProps() 
{
	CGameInstance* pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);

#pragma region Hull_Damage_Point
	CHull_Damage_Point::DAMAGEPOINTDESC DecalDesc;
	DecalDesc.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();

	{
		XMStoreFloat4x4(&DecalDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(90.f)) * XMMatrixTranslation(2.7f, 0.f, 1.f));
		DecalDesc.eDecalType = CHull_Damage_Point::HULL_DAMAGE_DECAL_04;
		DecalDesc.eSoundChannel = CHANNEL_WATERSTREAM_0;
		CHull_Damage_Point* pDamagePoint = static_cast<CHull_Damage_Point*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Hull_Damage_Point"), &DecalDesc));
		if (nullptr == pDamagePoint)
			return E_FAIL;

		pDamagePoint->Activate();

		m_vecDamagePoints.push_back(pDamagePoint);
	}

	{
		XMStoreFloat4x4(&DecalDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(90.f)) * XMMatrixTranslation(2.6f, 0.f, 3.f));
		DecalDesc.eDecalType = CHull_Damage_Point::HULL_DAMAGE_DECAL_04;
		CHull_Damage_Point* pDamagePoint = static_cast<CHull_Damage_Point*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Hull_Damage_Point"), &DecalDesc));
		if (nullptr == pDamagePoint)
			return E_FAIL;
		//pDamagePoint->Activate();
		m_vecDamagePoints.push_back(pDamagePoint);
	}

	{
		XMStoreFloat4x4(&DecalDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(90.f)) * XMMatrixTranslation(2.f, 0.f, 6.f));
		DecalDesc.eDecalType = CHull_Damage_Point::HULL_DAMAGE_DECAL_04;
		CHull_Damage_Point* pDamagePoint = static_cast<CHull_Damage_Point*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Hull_Damage_Point"), &DecalDesc));
		if (nullptr == pDamagePoint)
			return E_FAIL;
		//pDamagePoint->Activate();
		m_vecDamagePoints.push_back(pDamagePoint);
	}

	{
		XMStoreFloat4x4(&DecalDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(-90.f)) * XMMatrixTranslation(-2.f, 0.f, 6.f));
		DecalDesc.eDecalType = CHull_Damage_Point::HULL_DAMAGE_DECAL_04;
		CHull_Damage_Point* pDamagePoint = static_cast<CHull_Damage_Point*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Hull_Damage_Point"), &DecalDesc));
		if (nullptr == pDamagePoint)
			return E_FAIL;
		//pDamagePoint->Activate();
		m_vecDamagePoints.push_back(pDamagePoint);
	}

	{
		XMStoreFloat4x4(&DecalDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(90.f)) * XMMatrixTranslation(1.5f, 0.f, 7.5f));
		DecalDesc.eDecalType = CHull_Damage_Point::HULL_DAMAGE_DECAL_04;
		CHull_Damage_Point* pDamagePoint = static_cast<CHull_Damage_Point*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Hull_Damage_Point"), &DecalDesc));
		if (nullptr == pDamagePoint)
			return E_FAIL;

		m_vecDamagePoints.push_back(pDamagePoint);
	}

	{
		XMStoreFloat4x4(&DecalDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(-90.f)) * XMMatrixTranslation(-1.5f, 0.f, 7.5f));
		DecalDesc.eDecalType = CHull_Damage_Point::HULL_DAMAGE_DECAL_04;
		CHull_Damage_Point* pDamagePoint = static_cast<CHull_Damage_Point*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Hull_Damage_Point"), &DecalDesc));
		if (nullptr == pDamagePoint)
			return E_FAIL;

		m_vecDamagePoints.push_back(pDamagePoint);
	}

	{
		XMStoreFloat4x4(&DecalDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(-90.f)) * XMMatrixTranslation(-2.7f, 0.f, 2.f));
		DecalDesc.eDecalType = CHull_Damage_Point::HULL_DAMAGE_DECAL_04;
		CHull_Damage_Point* pDamagePoint = static_cast<CHull_Damage_Point*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Hull_Damage_Point"), &DecalDesc));
		if (nullptr == pDamagePoint)
			return E_FAIL;

		m_vecDamagePoints.push_back(pDamagePoint);
	}

	{
		XMStoreFloat4x4(&DecalDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(90.f)) * XMMatrixTranslation(3.f, 1.f, -5.f));
		DecalDesc.eDecalType = CHull_Damage_Point::HULL_DAMAGE_DECAL_04;
		CHull_Damage_Point* pDamagePoint = static_cast<CHull_Damage_Point*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Hull_Damage_Point"), &DecalDesc));
		if (nullptr == pDamagePoint)
			return E_FAIL;

		m_vecDamagePoints.push_back(pDamagePoint);
	}

	{
		XMStoreFloat4x4(&DecalDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(90.f)) * XMMatrixTranslation(2.6f, 1.f, -8.f));
		DecalDesc.eDecalType = CHull_Damage_Point::HULL_DAMAGE_DECAL_04;
		CHull_Damage_Point* pDamagePoint = static_cast<CHull_Damage_Point*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Hull_Damage_Point"), &DecalDesc));
		if (nullptr == pDamagePoint)
			return E_FAIL;

		m_vecDamagePoints.push_back(pDamagePoint);
	}

	{
		XMStoreFloat4x4(&DecalDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(-90.f)) * XMMatrixTranslation(-3.f, 1.f, -5.f));
		DecalDesc.eDecalType = CHull_Damage_Point::HULL_DAMAGE_DECAL_04;
		CHull_Damage_Point* pDamagePoint = static_cast<CHull_Damage_Point*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Hull_Damage_Point"), &DecalDesc));
		if (nullptr == pDamagePoint)
			return E_FAIL;

		m_vecDamagePoints.push_back(pDamagePoint);
	}

	{
		XMStoreFloat4x4(&DecalDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(-90.f)) * XMMatrixTranslation(-2.6f, 1.f, -8.f));
		DecalDesc.eDecalType = CHull_Damage_Point::HULL_DAMAGE_DECAL_04;
		CHull_Damage_Point* pDamagePoint = static_cast<CHull_Damage_Point*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Hull_Damage_Point"), &DecalDesc));
		if (nullptr == pDamagePoint)
			return E_FAIL;

		m_vecDamagePoints.push_back(pDamagePoint);
	}

#pragma endregion

#pragma region WaterPlane
	CWaterPlane::WATERPLANEDESC WaterPlaneDesc;
	WaterPlaneDesc.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();
	WaterPlaneDesc.vScale = _float3(10.f, 20.f, 1.f);
	WaterPlaneDesc.vPivotPosition = _float4(0.f, -1.f, 0.f, 1.f);
	WaterPlaneDesc.eGeometryType = CWaterPlane::RECT;
	WaterPlaneDesc.eRenderType = CWaterPlane::Water_Cull_Occ;

	m_pWaterPlane = static_cast<CWaterPlane*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_WaterPlane"), &WaterPlaneDesc));
	if (nullptr == m_pWaterPlane)
		return E_FAIL;

#pragma endregion

	Safe_Release(pGameInstance);

	return S_OK;
}

HRESULT CSloop::Add_WaterSplashPoints()
{
	CGameInstance* pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);

	CWater_Splash_Point::SPLASHPOINTDESC SplashPoint;
	SplashPoint.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();

	{
		XMStoreFloat4x4(&SplashPoint.PivotMatrix, XMMatrixTranslation(-3.f, -3.f, 10.f));
		SplashPoint.eSoundChannel = CHANNEL_WATER_SPLASH_0;
		CWater_Splash_Point* pSplashPoint = static_cast<CWater_Splash_Point*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Water_Splash_Point"), &SplashPoint));
		if (nullptr == pSplashPoint)
			return E_FAIL;

		m_vecWaterSplashPoints.push_back(pSplashPoint);
	}

	{
		XMStoreFloat4x4(&SplashPoint.PivotMatrix, XMMatrixTranslation(3.f, -3.f, 10.f));
		SplashPoint.eSoundChannel = CHANNEL_WATER_SPLASH_1;
		CWater_Splash_Point* pSplashPoint = static_cast<CWater_Splash_Point*>(pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Water_Splash_Point"), &SplashPoint));
		if (nullptr == pSplashPoint)
			return E_FAIL;

		m_vecWaterSplashPoints.push_back(pSplashPoint);
	}

	Safe_Release(pGameInstance);

	return S_OK;
}

HRESULT CSloop::Add_Pet()
{
	CGameInstance* pGameInstance = CGameInstance::Get_Instance();
	Safe_AddRef(pGameInstance);

	CFox::FOXDESC			FoxDesc;
	FoxDesc.pParentMatrix = m_pRigidbodyCom->Get_WorldMatrixPtr();
	XMStoreFloat4x4(&FoxDesc.PivotMatrix, XMMatrixRotationY(XMConvertToRadians(180.f))
		* XMMatrixTranslation(0.5f, 1.6f, 5.f));

	CGameObject* pFox = pGameInstance->Clone_GameObject(TEXT("Prototype_GameObject_Fox"), &FoxDesc);
	if (nullptr == pFox)
		return E_FAIL;

	m_pPet = static_cast<CFox*>(pFox);

	Safe_Release(pGameInstance);

	return S_OK;
}

HRESULT CSloop::Add_Ship_Models()
{
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Bell_Support"),
		TEXT("Com_Model_Sloop_Bell_Support"), (CComponent**)&m_pSloopModelComs[SLOOP_BELL_SUPPORT])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Brig"),
		TEXT("Com_Model_Sloop_Brig"), (CComponent**)&m_pSloopModelComs[SLOOP_BRIG])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Canopy"),
		TEXT("Com_Model_Sloop_Canopy"), (CComponent**)&m_pSloopModelComs[SLOOP_CANOPY])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Cap_Table"),
		TEXT("Com_Model_Sloop_Cap_Table"), (CComponent**)&m_pSloopModelComs[SLOOP_CAP_TABLE])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Crows_Nest"),
		TEXT("Com_Model_Sloop_Crows_Nest"), (CComponent**)&m_pSloopModelComs[SLOOP_CROWS_NEST])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Hatch"),
		TEXT("Com_Model_Sloop_Hatch"), (CComponent**)&m_pSloopModelComs[SLOOP_HATCH])))
		return E_FAIL;
	
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Hull"),
		TEXT("Com_Model_Sloop_Hull"), (CComponent**)&m_pSloopModelComs[SLOOP_HULL])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Hull_Damage"),
		TEXT("Com_Model_Sloop_Hull_Damage"), (CComponent**)&m_pSloopModelComs[SLOOP_HULL_DAMAGE])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Hull_Rope"),
		TEXT("Com_Model_Sloop_Hull_Rope"), (CComponent**)&m_pSloopModelComs[SLOOP_HULL_ROPE])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Lower_Deck"),
		TEXT("Com_Model_Sloop_Lower_Deck"), (CComponent**)&m_pSloopModelComs[SLOOP_LOWER_DECK])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Lower_Deck_Rope"),
		TEXT("Com_Model_Sloop_Lower_Deck_Rope"), (CComponent**)&m_pSloopModelComs[SLOOP_LOWER_DECK_ROPE])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Map_Table"),
		TEXT("Com_Model_Sloop_Map_Table"), (CComponent**)&m_pSloopModelComs[SLOOP_MAP_TABLE])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Mast"),
		TEXT("Com_Model_Sloop_Mast"), (CComponent**)&m_pSloopModelComs[SLOOP_MAST])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Mast_Crossbeam"),
		TEXT("Com_Model_Sloop_Mast_Crossbeam"), (CComponent**)&m_pSloopModelComs[SLOOP_MAST_CROSSBEAM])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Mast_Ladder"),
		TEXT("Com_Model_Sloop_Mast_Ladder"), (CComponent**)&m_pSloopModelComs[SLOOP_MAST_LADDER])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Rudder"),
		TEXT("Com_Model_Sloop_Rudder"), (CComponent**)&m_pSloopModelComs[SLOOP_RUDDER])))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sloop_Stairs"),
		TEXT("Com_Model_Sloop_Stairs"), (CComponent**)&m_pSloopModelComs[SLOOP_STAIRS])))
		return E_FAIL;

	MODELDESC ModelDesc;
	ModelDesc.bStaticModel = false;

	_matrix LocalMatrix = XMMatrixTranslation(0.f, -0.9f, 3.8f);
	XMStoreFloat4x4(&ModelDesc.LocalMatrix, LocalMatrix);
	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Barrel_Salvage"),
		TEXT("Com_Model_Sloop_Barrel_A"), (CComponent**)&m_pSloopModelComs[SLOOP_BARREL_SALVAGE_A], &ModelDesc)))
		return E_FAIL;

	if (FAILED(__super::Add_Component(LEVEL_GAMEPLAY, TEXT("Prototype_Component_Model_Sail"),
		TEXT("Com_Model_Sail"), (CComponent**)&m_pSloopAnimModelComs[SLOOP_SAIL])))
		return E_FAIL;
	
	return S_OK;
}

CSloop* CSloop::Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	CSloop* pInstance = new CSloop(pDevice, pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Created : CSloop");
	}

	return pInstance;
}

CGameObject* CSloop::Clone(void* pArg)
{
	CSloop* pInstance = new CSloop(*this);

	if (FAILED(pInstance->Initialize(pArg)))
	{
		Safe_Release(pInstance);
		MSG_BOX("Failed to Cloned : CSloop");
	}

	return pInstance;
}

void CSloop::Free()
{
	__super::Free();

	Safe_Release(m_pColliderCom);
	Safe_Release(m_pRendererCom);
	Safe_Release(m_pRigidbodyCom);
	Safe_Release(m_pMeshShaderCom);

	for (_uint i = 0; i < SLOOP_MODEL_END; ++i)
		Safe_Release(m_pSloopModelComs[i]);

	for (_uint i = 0; i < SLOOP_ANIM_MODEL_END; ++i)
		Safe_Release(m_pSloopAnimModelComs[i]);

	Safe_Release(m_pVehicleColliderComs);

	Safe_Release(m_pAnimMeshShaderCom);

	for (auto& Pair : m_ShipParts)
		Safe_Release(Pair.second);

	m_ShipParts.clear();

	Safe_Release(m_pPet);

	Safe_Release(m_pGroupCollider);

	Safe_Release(m_pSloopWaterOcc);

	for (auto* pDamagePoint : m_vecDamagePoints)
	{
		Safe_Release(pDamagePoint);
	}

	Safe_Release(m_pWaterPlane);

	for (auto* pPoint : m_vecWaterSplashPoints)
	{
		Safe_Release(pPoint);
	}

#ifdef _DEBUG

	for (_uint i = 0; i < POINT_END; ++i)
		Safe_Release(m_pBuoyancyPointColliderCom[i]);

#endif
}
