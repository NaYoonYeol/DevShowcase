#pragma once
#include "Client_Defines.h"
#include "Ship.h"

#include "CollisionManager.h"

BEGIN(Client)

class CSloop final : public CShip
{
public:
	enum SLOOP_MODEL
	{
		SLOOP_BELL_SUPPORT, // 그림자 제외
		SLOOP_BRIG, // 그림자 제외
		SLOOP_CANOPY, 
		SLOOP_CAP_TABLE, // 그림자 제외
		SLOOP_CROWS_NEST,
		SLOOP_HATCH, // 그림자 제외
		SLOOP_HULL,
		SLOOP_HULL_DAMAGE,
		SLOOP_HULL_ROPE, // 그림자 제외
		SLOOP_LOWER_DECK, // 그림자 제외
		SLOOP_LOWER_DECK_ROPE, // 그림자 제외
		SLOOP_MAP_TABLE, // 그림자 제외
		SLOOP_MAST,
		SLOOP_MAST_CROSSBEAM,
		SLOOP_MAST_LADDER, // 그림자 제외
		SLOOP_RUDDER, 
		SLOOP_STAIRS,
		SLOOP_BARREL_SALVAGE_A, // 그림자 제외
		SLOOP_MODEL_END,
	};

	enum SLOOP_ANIM_MODEL
	{
		SLOOP_SAIL,
		SLOOP_ANIM_MODEL_END,
	};
	
private:
	CSloop(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	CSloop(const CShip& rhs);
	virtual ~CSloop() = default;

public:
	virtual HRESULT Initialize_Prototype() override;
	virtual HRESULT Initialize(void* pArg) override;
	virtual void Tick(_float fTimeDelta) override;
	virtual void Late_Tick(_float fTimeDelta) override;
	virtual HRESULT Render() override;
	virtual HRESULT Render_ShadowDepth() override;

public:
	void Set_Movable(_bool bMovable) { m_bMovable = bMovable; }

public:
	virtual void Enable_Standing_Colliders(_bool bEnable) override;

public:
	_float Compute_WaterPlaneHeight(_fvector vPosition);

	_float Get_WaterPlaneHeight();
	void Add_SurfaceHeight(_float fAmount);

public:
	void Put_Pet();

private:
	void OnCollisionEnter(CCollider* pOwn, CCollider* pOther, CCollisionManager::COLLISION_MASK eMask);
	
	void OnVehicleCollisionEnter(CCollider* pOwn, CCollider* pOther, CCollisionManager::COLLISION_MASK eMask);

private:
	CShader* m_pMeshShaderCom = { nullptr };
	CModel* m_pSloopModelComs[SLOOP_MODEL_END] = { nullptr, };
	
	CModel* m_pSloopAnimModelComs[SLOOP_ANIM_MODEL_END] = { nullptr, };
	CShader* m_pAnimMeshShaderCom = { nullptr };

	CGameObject* m_pSloopWaterOcc = { nullptr };

private:
	std::vector<SLOOP_MODEL> m_ShadowMappingModels;

private:
	CCollider* m_pVehicleColliderComs = { nullptr, };

private:
	std::vector<class CHull_Damage_Point*> m_vecDamagePoints;
	class CWaterPlane* m_pWaterPlane = { nullptr };

	std::vector<class CWater_Splash_Point*> m_vecWaterSplashPoints;

private:
	class CGroupCollider* m_pGroupCollider = { nullptr };

private:
	_bool m_bOnHited = { false };

	_float m_fHitAngularVelocityAccTime = { 0.f };

private:
	class CFox* m_pPet = nullptr;
	_bool m_bPetEnable = { false };

private:
	_bool m_bMovable = { false };

private:
	virtual HRESULT Add_Components() override;
	virtual HRESULT Add_ShipParts() override;

	virtual HRESULT Add_BattleProps() override;

	HRESULT Add_WaterSplashPoints();
	HRESULT Add_Pet();
	
private:
	HRESULT Add_Ship_Models();
	
public:
	static CSloop* Create(ID3D11Device* pDevice, ID3D11DeviceContext* pContext);
	virtual CGameObject* Clone(void* pArg) override;
	virtual void Free() override;
};

END