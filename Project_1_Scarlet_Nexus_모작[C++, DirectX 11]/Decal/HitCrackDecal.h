#pragma once
#include "Client_Define.h"
#include "Decal.h"

BEGIN(Client)

class CHitCrackDecal : public CDecal
{
private:
	explicit CHitCrackDecal(ComPtr<ID3D11Device>, ComPtr<ID3D11DeviceContext>);
	explicit CHitCrackDecal(const CDecal&);
	virtual ~CHitCrackDecal() DEFAULT;

public:
	virtual HRESULT						Initialize_Prototype() override;
	virtual HRESULT						Initialize(any = g_aNull) override;
	virtual void						Tick(_float _fTimeDelta) override;
	virtual void						Late_Tick(_float _fTimeDelta) override;
	virtual HRESULT						Render() override;
	virtual HRESULT						Render_Neon() override;

public:
	virtual HRESULT						Fetch(any = g_aNull) override;

private:
	virtual HRESULT						Ready_Components() override;

private:
	shared_ptr<CTexture>				m_pHitCrackTexture;
	shared_ptr<CTexture>				m_pHitCrackMaskTexture;
	shared_ptr<CTexture>				m_pWoundMaskTexture;

private:
	weak_ptr<CTransform>				m_pTargetTransform;

	_float4x4							m_matHitTransform;
	_float3								m_vPivotPosition;

private:
	_float								m_fLifeTime = { 1.f };
	_float								m_fMaxLifeTime = { 1.f };

private:
	_float3								m_vWoundColor = { 0.721f * 3.f, 0.239f * 3.f, 0.729f * 3.f };
	_float								m_fBloomStrength = { 0.f };

public:
	static shared_ptr<CHitCrackDecal>	Create(ComPtr<ID3D11Device>, ComPtr<ID3D11DeviceContext>);
	virtual shared_ptr<CGameObject>		Clone(any = g_aNull) override;
};

END
