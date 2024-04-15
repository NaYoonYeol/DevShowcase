#include "EnginePCH.h"
#include "InteractiveObject.h"

#include "GameInstance.h"
#include "Light.h"
#include "Light_Manager.h"

CInteractiveObject::CInteractiveObject(ComPtr<ID3D11Device> _pDevice, ComPtr<ID3D11DeviceContext> _pContext)
	: CGameObject(_pDevice, _pContext)
{
}

CInteractiveObject::CInteractiveObject(const CInteractiveObject& _rhs)
	: CGameObject(_rhs),
{
}

HRESULT CInteractiveObject::Initialize(any)
{
	if (FAILED(__super::Initialize()))
	{
		MSG_RETURN(E_FAIL, "CInteractiveObject::Initialize", "Failed to CInteractiveObject::Initialize");
	}

	return S_OK;
}

void CInteractiveObject::Tick(_float _fTimeDelta)
{
	__super::Tick(_fTimeDelta);
}

void CInteractiveObject::Late_Tick(_float _fTimeDelta)
{
	__super::Late_Tick(_fTimeDelta);
}

HRESULT CInteractiveObject::Render()
{
	return S_OK;
}

HRESULT CInteractiveObject::Render(_uint _iPassIndex, _bool _bOrthographic)
{
	if (FAILED(__super::Render(_iPassIndex, _bOrthographic)))
	{
		return E_FAIL;
	}

	return S_OK;
}

void CInteractiveObject::Create_Light(shared_ptr<class CTransform> _pTransform)
{
	LIGHTDESC tLightDesc;
	tLightDesc.eLightType = LIGHTTYPE::POINT;
	tLightDesc.fRange = 5.f;
	tLightDesc.vDiffuse = _float4(0.721f, 0.239f, 0.729f, 1.f);
	tLightDesc.vAmbient = _float4(0.721f, 0.239f, 0.729f, 1.f);
	tLightDesc.vSpecular = _float4(1.f, 1.f, 1.f, 1.f);
	m_pLight = CLight::Create(tLightDesc, _pTransform);
}

void CInteractiveObject::Interactive_PhychoKinesis(_bool _bInteractive)
{
	if (_bInteractive && false == m_bEnableLight)
	{
		m_bEnableLight = true;
		CLight_Manager::Get_Instance()->Add_Light(CGameInstance::Get_Instance()->Current_Scene(), m_pLight);
	}
	else if (false == _bInteractive && m_bEnableLight)
	{
		m_bEnableLight = false;
		CLight_Manager::Get_Instance()->Erase_Light(m_pLight);
	}
}

HRESULT CInteractiveObject::Ready_Components()
{
	if (FAILED(__super::Ready_Components()))
	{
		MSG_RETURN(E_FAIL, "CInteractiveObject::Ready_Components", "Failed to CGameObject::Ready_Components");
	}

	return S_OK;
}

shared_ptr<CGameObject> CInteractiveObject::Clone(any)
{
	shared_ptr<CInteractiveObject> pInstance = make_private_shared_copy(CInteractiveObject, *this);

	if (FAILED(pInstance->Initialize()))
	{
		MSG_RETURN(nullptr, "CInteractiveObject::Clone", "Failed to Initialize");
	}

	return pInstance;
}
