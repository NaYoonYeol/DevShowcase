#include "stdafx.h"
#include "Ship.h"

#include "GameInstance.h"
#include "Ocean.h"
#include "Model.h"

#include "Gui.h"

CShip::CShip(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
	: CGameObject(pDevice, pContext)
{
}

CShip::CShip(const CShip& rhs)
	: CGameObject(rhs)
{
}

HRESULT CShip::Initialize_Prototype()
{
	if (FAILED(__super::Initialize_Prototype()))
		return E_FAIL;

	return S_OK;
}

HRESULT CShip::Initialize(void* pArg)
{
	if (FAILED(__super::Initialize(pArg)))
		return E_FAIL;

	// ¹Ù´Ù Ä³½Ì
	m_pOcean = static_cast<COcean*>(CGameInstance::Get_Instance()->Find_GameObject(TEXT("Ocean")));

	return S_OK;
}

void CShip::Tick(_float fTimeDelta)
{
	__super::Tick(fTimeDelta);

	if (nullptr != m_pOcean)
	{
		m_pOcean->Animate_Ship(m_pRigidbodyCom, m_GeometryDesc);
	}
}

void CShip::Late_Tick(_float fTimeDelta)
{
	__super::Late_Tick(fTimeDelta);
}

HRESULT CShip::Render()
{
	if (FAILED(__super::Render()))
		return E_FAIL;
	
	return S_OK;
}

HRESULT CShip::Render_ShadowDepth()
{
	if (FAILED(__super::Render_ShadowDepth()))
		return E_FAIL;

	return S_OK;
}

void CShip::Update_Geometry()
{
	constexpr _uint iVertexCount = 8;

	auto& GeometryVertices = m_GeometryDesc.vecVertices;
	GeometryVertices.clear();
	GeometryVertices.reserve(iVertexCount);

	_float3 vNormal;
	XMStoreFloat3(&vNormal, XMVector3Normalize(XMVectorSet(m_fGeometryScale.x, m_fGeometryScale.y, m_fGeometryScale.z, 0.f)));
	GeometryVertices.emplace_back(VTXPOSNORTEX{ XMFLOAT3(m_fGeometryScale.x, m_fGeometryScale.y, m_fGeometryScale.z),  vNormal, XMFLOAT2(0, 0) });
	XMStoreFloat3(&vNormal, XMVector3Normalize(XMVectorSet(-m_fGeometryScale.x, m_fGeometryScale.y, m_fGeometryScale.z, 0.f)));
	GeometryVertices.emplace_back(VTXPOSNORTEX{ XMFLOAT3(-m_fGeometryScale.x, m_fGeometryScale.y, m_fGeometryScale.z),  vNormal, XMFLOAT2(1, 0) });
	XMStoreFloat3(&vNormal, XMVector3Normalize(XMVectorSet(-m_fGeometryScale.x, -m_fGeometryScale.y, m_fGeometryScale.z, 0.f)));
	GeometryVertices.emplace_back(VTXPOSNORTEX{ XMFLOAT3(-m_fGeometryScale.x, -m_fGeometryScale.y, m_fGeometryScale.z), vNormal, XMFLOAT2(1, 1) });
	XMStoreFloat3(&vNormal, XMVector3Normalize(XMVectorSet(m_fGeometryScale.x, -m_fGeometryScale.y, m_fGeometryScale.z, 0.f)));
	GeometryVertices.emplace_back(VTXPOSNORTEX{ XMFLOAT3(m_fGeometryScale.x, -m_fGeometryScale.y, m_fGeometryScale.z), vNormal, XMFLOAT2(0, 1) });
	XMStoreFloat3(&vNormal, XMVector3Normalize(XMVectorSet(m_fGeometryScale.x, m_fGeometryScale.y, -m_fGeometryScale.z, 0.f)));
	GeometryVertices.emplace_back(VTXPOSNORTEX{ XMFLOAT3(m_fGeometryScale.x, m_fGeometryScale.y, -m_fGeometryScale.z),  vNormal, XMFLOAT2(0, 0) });
	XMStoreFloat3(&vNormal, XMVector3Normalize(XMVectorSet(m_fGeometryScale.x, -m_fGeometryScale.y, -m_fGeometryScale.z, 0.f)));
	GeometryVertices.emplace_back(VTXPOSNORTEX{ XMFLOAT3(m_fGeometryScale.x, -m_fGeometryScale.y, -m_fGeometryScale.z), vNormal, XMFLOAT2(1, 0) });
	XMStoreFloat3(&vNormal, XMVector3Normalize(XMVectorSet(-m_fGeometryScale.x, -m_fGeometryScale.y, -m_fGeometryScale.z, 0.f)));
	GeometryVertices.emplace_back(VTXPOSNORTEX{ XMFLOAT3(-m_fGeometryScale.x, -m_fGeometryScale.y, -m_fGeometryScale.z), vNormal, XMFLOAT2(1, 1) });
	XMStoreFloat3(&vNormal, XMVector3Normalize(XMVectorSet(-m_fGeometryScale.x, m_fGeometryScale.y, -m_fGeometryScale.z, 0.f)));
	GeometryVertices.emplace_back(VTXPOSNORTEX{ XMFLOAT3(-m_fGeometryScale.x, m_fGeometryScale.y, -m_fGeometryScale.z), vNormal, XMFLOAT2(0, 1) });

	auto& GeometryBuoyancyWeights = m_GeometryDesc.vecBuoyancyWeights;
	GeometryBuoyancyWeights.clear();
	GeometryBuoyancyWeights.resize(iVertexCount, (2 * ((m_fGeometryScale.x * 2) * (m_fGeometryScale.y * 2) + (m_fGeometryScale.x * 2) * (m_fGeometryScale.z * 2) + (m_fGeometryScale.y * 2) * (m_fGeometryScale.z * 2)) / 8));
}

void CShip::Free()
{
	__super::Free();
}