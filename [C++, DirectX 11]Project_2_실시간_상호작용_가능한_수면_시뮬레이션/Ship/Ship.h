#pragma once
#include "Client_Defines.h"
#include "GameObject.h"

BEGIN(Engine)
class CShader;
class CRenderer;
class CRigidbody;
class CModel;
class CCollider;
END

BEGIN(Client)

class CShip abstract : public CGameObject
{
public:
	enum BUOYANCY_POINT { POINT_COB/*부심*/, POINT_BOW/*선수*/, POINT_STERN/*선미*/, POINT_PORT/*좌현*/, POINT_STARBOARD/*우현*/, POINT_END };

public:
	enum SAIL_ANIMATION
	{
		ANIMATION_SAIL_BILLOWED_FOLD,
		ANIMATION_SAIL_BILLOWED_TURN,
		ANIMATION_SAIL_BILLOWED_WIND,
		ANIMATION_SAIL_HALF_BILLOWED_TURN,
		ANIMATION_SAIL_LAX_FOLD,
		ANIMATION_SAIL_LAX_TURN,
		ANIMATION_SAIL_LAX_WIND,
		ANIMATION_SAIL_END,
	};

protected:
	CShip(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CShip(const CShip& rhs);
	virtual ~CShip() = default;

public:
	virtual HRESULT Initialize_Prototype();
	virtual HRESULT Initialize(void* pArg);
	virtual void Tick(_float fTimeDelta);
	virtual void Late_Tick(_float fTimeDelta);
	virtual HRESULT Render();
	virtual HRESULT Render_ShadowDepth() override;

public:
	virtual void Enable_Standing_Colliders(_bool bEnable) {}
	GEOMETRYDESC& Get_Geometry() { return m_GeometryDesc; }

protected:
	void Update_Geometry();

protected:
	CCollider* m_pColliderCom = { nullptr };
	CRenderer* m_pRendererCom = { nullptr };
	CRigidbody* m_pRigidbodyCom = { nullptr };

protected:
	std::map<const wstring, CGameObject*>	m_ShipParts;

#ifdef _DEBUG 
protected:
	// 부력 계산용 정점 위치 포인트
	CCollider* m_pBuoyancyPointColliderCom[POINT_END] = { nullptr, };
#endif

private:
	class COcean* m_pOcean = { nullptr };

protected:
	_float3 m_fGeometryScale = {};
	GEOMETRYDESC m_GeometryDesc = {};

private:
	virtual HRESULT Add_Components() = 0;
	virtual HRESULT Add_ShipParts() = 0;
	virtual HRESULT Add_BattleProps() = 0;

public:
	virtual void Free() override;
};

END