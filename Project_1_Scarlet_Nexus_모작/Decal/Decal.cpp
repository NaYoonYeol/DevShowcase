#include "ClientPCH.h"
#include "Decal.h"

#include "GameInstance.h"
#include "ObjectPool.h"

CDecal::CDecal(ComPtr<ID3D11Device> _pDevice, ComPtr<ID3D11DeviceContext> _pContext)
	: CGameObject(_pDevice, _pContext)
{
}

CDecal::CDecal(const CDecal& _rhs)
	: CGameObject(_rhs)
{
}

HRESULT CDecal::Initialize_Prototype()
{
	m_bitComponent |= BIT(COMPONENT::RENDERER) | BIT(COMPONENT::TRANSFORM) | BIT(COMPONENT::SHADER) | BIT(COMPONENT::VIBUFFER_CUBE);

	m_umapComponentArg[COMPONENT::RENDERER] = make_pair(PROTOTYPE_COMPONENT_RENDERER_MAIN, g_aNull);
	m_umapComponentArg[COMPONENT::SHADER] = make_pair(PROTOTYPE_COMPONENT_SHADER_VTXPOSNORTEX_DECAL, g_aNull);
	m_umapComponentArg[COMPONENT::VIBUFFER_CUBE] = make_pair(PROTOTYPE_COMPONENT_VIBUFFER_CUBE, g_aNull);

	return S_OK;
}

HRESULT CDecal::Initialize(any)
{
	if (FAILED(__super::Initialize()))
	{
		MSG_RETURN(E_FAIL, "CDecal::Initialize", "Failed to CGameObject::Initialize");
	}

	m_tMaterialDesc.vDiffuse = _float4(1.f, 1.f, 1.f, 1.f);

	return S_OK;
}

void CDecal::Tick(_float _fTimeDelta)
{
	__super::Tick(_fTimeDelta);
}

void CDecal::Late_Tick(_float _fTimeDelta)
{
	__super::Late_Tick(_fTimeDelta);
}

HRESULT CDecal::Render()
{
	if (FAILED(m_pTransform->Bind_OnShader(m_pShader)))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Bind_OnShader: Transform");
	}

	auto pPipeLine = CPipeLine::Get_Instance();
	if (FAILED(m_pShader->Bind_Matrix("g_mWorldInv", XMMatrixInverse(nullptr, m_pTransform->Get_Matrix()))))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Bind_Matrix: g_mWorldInv");
	}
	if (FAILED(m_pShader->Bind_Matrix(SHADER_MATVIEWINV, XMMatrixInverse(nullptr, pPipeLine->Get_Transform(PIPELINE::VIEW)))))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Bind_Matrix: SHADER_MATVIEWINV");
	}
	if (FAILED(m_pShader->Bind_Matrix(SHADER_MATPROJINV, XMMatrixInverse(nullptr, pPipeLine->Get_Transform(PIPELINE::PROJECTION)))))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Bind_Matrix: SHADER_MATPROJINV");
	}

	auto pGameInstance = CGameInstance::Get_Instance();
	auto pDepthTarget = pGameInstance->Get_RenderTarget_ShaderResourceView(RENDERTARGET_DEPTH);
	if (FAILED(m_pShader->Bind_ShaderResourceView("g_texDepthTarget", pDepthTarget)))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Bind_ShaderResourceView: DepthTarget");
	}

	if (FAILED(m_pShader->Bind_RawValue(SHADER_MTRLDIF, &m_tMaterialDesc.vDiffuse, sizeof(_float4))))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Bind_RawValue");
	}

	if (FAILED(m_pVIBuffer_Cube->Render(m_pShader, 0)))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Render");
	}

	return S_OK;
}

HRESULT CDecal::Fetch(any)
{
	if (FAILED(__super::Fetch()))
	{
		MSG_RETURN(E_FAIL, "CDecal::Fetch", "Failed to CGameObject::Fetch");
	}

	return S_OK;
}

HRESULT CDecal::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
	{
		MSG_RETURN(E_FAIL, "CDecal::Ready_Components", "Failed to CGameObject::Ready_Components");
	}

	m_pTransform = Get_Component<CTransform>(COMPONENT::TRANSFORM);
	if (nullptr == m_pTransform)
	{
		MSG_RETURN(E_FAIL, "CDecal::Ready_Components", "Failed to Get_Component: TRANSFORM");
	}

	m_pShader = Get_Component<CShader>(COMPONENT::SHADER);
	if (nullptr == m_pShader)
	{
		MSG_RETURN(E_FAIL, "CDecal::Ready_Components", "Failed to Get_Component: SHADER");
	}

	m_pVIBuffer_Cube = Get_Component<CVIBuffer_Cube>(COMPONENT::VIBUFFER_CUBE);
	if (nullptr == m_pVIBuffer_Cube)
	{
		MSG_RETURN(E_FAIL, "CDecal::Ready_Components", "Failed to Get_Component: VIBUFFER_CUBE");
	}

	m_pTransform->Set_Matrix(g_mUnit);


	return S_OK;
}

HRESULT CDecal::Render(_uint _iPassIndex)
{
	if (FAILED(m_pTransform->Bind_OnShader(m_pShader)))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Bind_OnShader: Transform");
	}

	auto pPipeLine = CPipeLine::Get_Instance();
	if (FAILED(m_pShader->Bind_Matrix("g_mWorldInv", XMMatrixInverse(nullptr, m_pTransform->Get_Matrix()))))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Bind_Matrix: g_mWorldInv");
	}
	if (FAILED(m_pShader->Bind_Matrix(SHADER_MATVIEWINV, XMMatrixInverse(nullptr, pPipeLine->Get_Transform(PIPELINE::VIEW)))))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Bind_Matrix: SHADER_MATVIEWINV");
	}
	if (FAILED(m_pShader->Bind_Matrix(SHADER_MATPROJINV, XMMatrixInverse(nullptr, pPipeLine->Get_Transform(PIPELINE::PROJECTION)))))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Bind_Matrix: SHADER_MATPROJINV");
	}

	auto pGameInstance = CGameInstance::Get_Instance();
	auto pDepthTarget = pGameInstance->Get_RenderTarget_ShaderResourceView(RENDERTARGET_DEPTH);
	if (FAILED(m_pShader->Bind_ShaderResourceView("g_texDepthTarget", pDepthTarget)))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Bind_ShaderResourceView: DepthTarget");
	}

	if (FAILED(m_pShader->Bind_RawValue(SHADER_MTRLDIF, &m_tMaterialDesc.vDiffuse, sizeof(_float4))))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Bind_RawValue");
	}

	if (FAILED(m_pVIBuffer_Cube->Render(m_pShader, _iPassIndex)))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Render");
	}

#ifdef _DEBUG
	if (FAILED(m_pVIBuffer_Cube->Render(m_pShader, 1)))
	{
		MSG_RETURN(E_FAIL, "CDecal::Render", "Failed to Render");
	}
#endif

	return S_OK;
}
