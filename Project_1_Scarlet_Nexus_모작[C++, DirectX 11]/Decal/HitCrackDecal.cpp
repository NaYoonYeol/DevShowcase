#include "ClientPCH.h"
#include "HitCrackDecal.h"

#include "GameInstance.h"

CHitCrackDecal::CHitCrackDecal(ComPtr<ID3D11Device> _pDevice, ComPtr<ID3D11DeviceContext> _pContext)
	: CDecal(_pDevice, _pContext)
{
}

CHitCrackDecal::CHitCrackDecal(const CDecal& rhs)
	: CDecal(rhs)
{
}

HRESULT CHitCrackDecal::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Initialize_Prototype", "Falied to CDecal::Initialize_Prototype");
	}

	return S_OK;
}

HRESULT CHitCrackDecal::Initialize(any)
{
	if (FAILED(__super::Initialize()))
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Initialize", "Failed to CDecal::Initialize");
	}

	m_tMaterialDesc.vDiffuse = _float4(0.75f, 0.f, 0.f, 1.f);

	return S_OK;
}

void CHitCrackDecal::Tick(_float _fTimeDelta)
{
	if (m_bFetched)
	{
		__super::Tick(_fTimeDelta);

		auto pTargetTransform = m_pTargetTransform.lock();
		if (pTargetTransform)
		{
			m_pTransform->Set_Matrix(m_matHitTransform);
			m_pTransform->Set_State(TRANSFORM::POSITION, pTargetTransform->Get_State(TRANSFORM::POSITION) + m_vPivotPosition);
			m_pTransform->Set_Scale(XMVectorSet(5.f, 1.f, 5.f, 0.f));

			m_fLifeTime += _fTimeDelta;
			if (m_fLifeTime >= m_fMaxLifeTime)
			{
				auto pGameInstance = CGameInstance::Get_Instance();
				pGameInstance->Find_Pool(pGameInstance->Current_Scene(), POOLOBJECT_DECAL_HITCRACK)->Push(shared_from_gameobject());

				m_bFetched = false;
			}
		}
		else
		{
			auto pGameInstance = CGameInstance::Get_Instance();
			pGameInstance->Find_Pool(pGameInstance->Current_Scene(), POOLOBJECT_DECAL_HITCRACK)->Push(shared_from_gameobject());

			m_bFetched = false;
		}
	}
}

void CHitCrackDecal::Late_Tick(_float _fTimeDelta)
{
	if (m_bFetched)
	{
		__super::Late_Tick(_fTimeDelta);

		Add_RenderObject(RENDER_GROUP::DECAL);
		Add_RenderObject(RENDER_GROUP::NEON);
	}
}

HRESULT CHitCrackDecal::Render()
{
	if (FAILED(m_pHitCrackTexture->Bind_ShaderResourceView(m_pShader, aiTextureType_DIFFUSE, SHADER_TEXDIFFUSE)))
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Render", "Failed to Bind_ShaderResourceView: Decal Texture");
	}
	if (FAILED(m_pHitCrackMaskTexture->Bind_ShaderResourceView(m_pShader, aiTextureType_DIFFUSE, "g_texDecalMask")))
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Render", "Failed to Bind_ShaderResourceView: Decal Mask Texture");
	}

	_float fNormalizedLiftTime = m_fLifeTime / m_fMaxLifeTime;
	if (FAILED(m_pShader->Bind_Float("g_fNormalizedLifeTime", fNormalizedLiftTime)))
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Render", "Failed to Bind_Float");
	}
	
	if (FAILED(__super::Render(2)))
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Render", "CDecal::Render");
	}

	return S_OK;
}

HRESULT CHitCrackDecal::Render_Neon()
{
	if (FAILED(m_pWoundMaskTexture->Bind_ShaderResourceView(m_pShader, aiTextureType_DIFFUSE, "g_texDecalMask")))
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Render", "Failed to Bind_ShaderResourceView: Decal Mask Texture");
	}

	_float fNormalizedLiftTime = m_fLifeTime / m_fMaxLifeTime;
	if (FAILED(m_pShader->Bind_Float("g_fNormalizedLifeTime", fNormalizedLiftTime)))
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Render", "Failed to Bind_Float");
	}
	_float fBloomStrength = (1.f - fNormalizedLiftTime) * m_fBloomStrength;
	if (FAILED(m_pShader->Bind_Float("g_fBloomStrength", fBloomStrength)))
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Render", "Failed to Bind_Float");
	}

	if (FAILED(m_pShader->Bind_RawValue("g_vDecalColor", &m_vWoundColor, sizeof(_float3))))
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Render", "Failed to Bind Decal Color");
	}

	if (FAILED(__super::Render(3)))
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Render", "CDecal::Render");
	}

	return S_OK;
}

HRESULT CHitCrackDecal::Fetch(any _DecalDesc)
{
	if (FAILED(__super::Fetch()))
	{
		MSG_RETURN(E_FAIL, "CDecal::Fetch", "Failed to CGameObject::Fetch");
	}

	DECALDESC tDecalDesc = any_cast<DECALDESC>(_DecalDesc);
	m_pTargetTransform = tDecalDesc.pTargetTransform;
	m_matHitTransform = tDecalDesc.matHitTransform;

	m_fLifeTime = 0.f;
	m_fMaxLifeTime = tDecalDesc.fLifeTime;

	_float4 vHitPosition = *(_float4*)m_matHitTransform.m[3];
	m_vPivotPosition = vHitPosition - tDecalDesc.pTargetTransform->Get_State(TRANSFORM::POSITION);

	m_bFetched = true;

	return S_OK;
}

HRESULT CHitCrackDecal::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Ready_Components", "Failed to CDecal::Ready_Components");
	}

	auto pGameInstance = CGameInstance::Get_Instance();
	m_pHitCrackTexture = pGameInstance->Clone_Component<CTexture>(SCENE::STATIC, PROTOTYPE_COMPONENT_TEXTURE_DECAL_EM_APP_START);
	if (nullptr == m_pHitCrackTexture)
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Ready_Components", "Failed to Clone_Component: PROTOTYPE_COMPONENT_TEXTURE_DECAL_EM_APP_START");
	}

	m_pHitCrackMaskTexture = pGameInstance->Clone_Component<CTexture>(SCENE::STATIC, PROTOTYPE_COMPONENT_TEXTURE_DECAL_CLS_LIN_006);
	if (nullptr == m_pHitCrackMaskTexture)
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Ready_Components", "Failed to Clone_Component: PROTOTYPE_COMPONENT_TEXTURE_DECAL_CLS_LIN_006");
	}

	m_pWoundMaskTexture = pGameInstance->Clone_Component<CTexture>(SCENE::STATIC, PROTOTYPE_COMPONENT_TEXTURE_DECAL_EV_ONE_CIR_001);
	if (nullptr == m_pWoundMaskTexture)
	{
		MSG_RETURN(E_FAIL, "CHitCrackDecal::Ready_Components", "Failed to Clone_Component: PROTOTYPE_COMPONENT_TEXTURE_DECAL_EV_ONE_CIR_001");
	}

	return S_OK;
}

shared_ptr<CHitCrackDecal> CHitCrackDecal::Create(ComPtr<ID3D11Device> _pDevice, ComPtr<ID3D11DeviceContext> _pContext)
{
	shared_ptr<CHitCrackDecal> pInstance = make_private_shared(CHitCrackDecal, _pDevice, _pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_RETURN(nullptr, "CHitCrackDecal::Create", "Failed to Initialize_Prototype");
	}

	return pInstance;
}

shared_ptr<CGameObject> CHitCrackDecal::Clone(any)
{
	shared_ptr<CHitCrackDecal> pInstance = make_private_shared_copy(CHitCrackDecal, *this);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_RETURN(nullptr, "CHitCrackDecal::Clone", "Failed to Initialize");
	}

	return pInstance;
}
