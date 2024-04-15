#include "ClientPCH.h"
#include "IceDecal.h"

#include "GameInstance.h"
#include "ImGui_Manager.h"

CIceDecal::CIceDecal(ComPtr<ID3D11Device> _pDevice, ComPtr<ID3D11DeviceContext> _pContext)
	: CDecal(_pDevice, _pContext)
{
}

CIceDecal::CIceDecal(const CDecal& _rhs)
	: CDecal(_rhs)
{
}

HRESULT CIceDecal::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
	{
		MSG_RETURN(E_FAIL, "CIceDecal::Initialize_Prototype", "Falied to CDecal::Initialize_Prototype");
	}

	return S_OK;
}

HRESULT CIceDecal::Initialize(any)
{
	if (FAILED(__super::Initialize()))
	{
		MSG_RETURN(E_FAIL, "CDecal::Initialize", "Failed to CGameObject::Initialize");
	}

	m_pTransform->Rotate(TRANSFORM::RIGHT, XMConvertToRadians(90.f));

	return S_OK;
}

void CIceDecal::Tick(_float _fTimeDelta)
{
	if (m_bFetched)
	{
		__super::Tick(_fTimeDelta);

		m_fAccTime += _fTimeDelta * m_fTimeScale;
		if (m_fAccTime >= m_fDuration)
		{
			m_fAccTime = 0.f;
			m_bFetched = false;
		}

		m_tMaterialDesc.vDiffuse.w = pow(m_fAccTime, 3.f);
	}

#ifdef _DEBUG
	if (CImGui_Manager::Get_Instance()->Is_Enable())
	{
		ImGui::Begin("Ice Decal");
	
		_float3 vPosition = m_pTransform->Get_State(TRANSFORM::POSITION);
		_float3 vScale = m_pTransform->Get_Scale();
	
		_bool bTransformDirtyFlag = false;
		ImGui::SeparatorText("Decal Transform");
		if (ImGui::DragFloat3("Decal Position", &vPosition.x, 0.05f, -FLT_MAX, +FLT_MAX))
			bTransformDirtyFlag = true;
		if (ImGui::DragFloat3("Decal Scale", &vScale.x, 0.05f, -FLT_MAX, +FLT_MAX))
			bTransformDirtyFlag = true;

		ImGui::DragFloat4("Decal Color", &m_tMaterialDesc.vDiffuse.x, 0.05f, -FLT_MAX, +FLT_MAX);
	
		if (bTransformDirtyFlag)
		{
			m_pTransform->Set_State(TRANSFORM::POSITION, vPosition);
			m_pTransform->Set_Scale(vScale);
		}

		ImGui::DragFloat("Decal Tiling Factor", &m_fTilingFactor, 0.05f);
	
		ImGui::End();
	}
#endif
}

void CIceDecal::Late_Tick(_float _fTimeDelta)
{
	if (m_bFetched)
	{
		__super::Late_Tick(_fTimeDelta);

		Add_RenderObject(RENDER_GROUP::DECAL);
	}
}

HRESULT CIceDecal::Render()
{
	if (FAILED(m_pTexture->Bind_ShaderResourceView(m_pShader, aiTextureType_DIFFUSE, SHADER_TEXDIFFUSE)))
	{
		MSG_RETURN(E_FAIL, "CIceDecal::Render", "Failed to Bind_ShaderResourceView: Decal Texture");
	}
	if (FAILED(m_pMaskTexture->Bind_ShaderResourceView(m_pShader, aiTextureType_DIFFUSE, "g_texDecalMask")))
	{
		MSG_RETURN(E_FAIL, "CIceDecal::Render", "Failed to Bind_ShaderResourceView: Mask Decal Texture");
	}

	if (FAILED(m_pShader->Bind_Float("g_fTilingFactor", m_fTilingFactor)))
	{
		MSG_RETURN(E_FAIL, "CIceDecal::Render", "Failed to Bind TilingFactor");
	}
	
	if (FAILED(__super::Render(4)))
	{
		MSG_RETURN(E_FAIL, "CIceDecal::Render", "CDecal::Render");
	}

	return S_OK;
}

HRESULT CIceDecal::Render_Bloom()
{
	return S_OK;
}

HRESULT CIceDecal::Render_Neon()
{
	return S_OK;
}

HRESULT CIceDecal::Fetch(any _tIceDecalDesc)
{
	ICEDECALDESC tIceDesc = any_cast<ICEDECALDESC>(_tIceDecalDesc);

	m_tMaterialDesc.vDiffuse = tIceDesc.vDecalColor;
	m_pTransform->Set_Scale(tIceDesc.vScale);
	m_pTransform->Set_State(TRANSFORM::POSITION, tIceDesc.vPosition);

	m_fTilingFactor = tIceDesc.fTilingFactor;
	m_fDuration = tIceDesc.fDuration;
	m_fAccTime = 0.f;

	m_fTimeScale = tIceDesc.fTimeScale;

	m_bFetched = true;

	return S_OK;
}

HRESULT CIceDecal::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
	{
		MSG_RETURN(E_FAIL, "CIceDecal::Ready_Components", "Failed to CDecal::Ready_Components");
	}

	auto pGameInstance = CGameInstance::Get_Instance();
	m_pTexture = pGameInstance->Clone_Component<CTexture>(SCENE::STATIC, PROTOTYPE_COMPONENT_TEXTURE_DECAL_CLS_NOI_048);
	if (nullptr == m_pTexture)
	{
		MSG_RETURN(E_FAIL, "CIceDecal::Ready_Components", "Failed to Clone_Component: PROTOTYPE_COMPONENT_TEXTURE_DECAL_CLS_NOI_048");
	}

	m_pMaskTexture = pGameInstance->Clone_Component<CTexture>(SCENE::STATIC, PROTOTYPE_COMPONENT_TEXTURE_MASK_CIRCULAR);
	if (nullptr == m_pMaskTexture)
	{
		MSG_RETURN(E_FAIL, "CIceDecal::Ready_Components", "Failed to Clone_Component: PROTOTYPE_COMPONENT_TEXTURE_MASK_CIRCULAR");
	}

	return S_OK;
}

shared_ptr<CIceDecal> CIceDecal::Create(ComPtr<ID3D11Device> _pDevice, ComPtr<ID3D11DeviceContext> _pContext)
{
	shared_ptr<CIceDecal> pInstance = make_private_shared(CIceDecal, _pDevice, _pContext);

	if (FAILED(pInstance->Initialize_Prototype()))
	{
		MSG_RETURN(nullptr, "CIceDecal::Create", "Failed to Initialize_Prototype");
	}

	return pInstance;
}

shared_ptr<CGameObject> CIceDecal::Clone(any _tIceDecalDesc)
{
	shared_ptr<CIceDecal> pInstance = make_private_shared_copy(CIceDecal, *this);

	if (FAILED(pInstance->Initialize(_tIceDecalDesc)))
	{
		MSG_RETURN(nullptr, "CIceDecal::Clone", "Failed to Initialize");
	}

	return pInstance;
}
