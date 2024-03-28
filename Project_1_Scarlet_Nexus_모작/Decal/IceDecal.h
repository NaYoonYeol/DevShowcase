#pragma once
#include "Client_Define.h"
#include "Decal.h"

BEGIN(Client)

class CIceDecal : public CDecal
{
private:
	explicit CIceDecal(ComPtr<ID3D11Device>, ComPtr<ID3D11DeviceContext>);
	explicit CIceDecal(const CDecal&);
	virtual ~CIceDecal() DEFAULT;

public:
	virtual HRESULT						Initialize_Prototype() override;
	virtual HRESULT						Initialize(any = g_aNull) override;
	virtual void						Tick(_float _fTimeDelta) override;
	virtual void						Late_Tick(_float _fTimeDelta) override;
	virtual HRESULT						Render() override;

	virtual HRESULT						Render_Bloom();
	virtual HRESULT						Render_Neon();

public:
	virtual HRESULT						Fetch(any = g_aNull) override;

private:
	virtual HRESULT						Ready_Components() override;

private:
	shared_ptr<CTexture>				m_pTexture;
	shared_ptr<CTexture>				m_pMaskTexture;

private:
	_float								m_fTilingFactor = { 1.f };
	
	_float								m_fDuration;
	_float								m_fAccTime;

	_float								m_fTimeScale;

public:
	static shared_ptr<CIceDecal>		Create(ComPtr<ID3D11Device>, ComPtr<ID3D11DeviceContext>);
	virtual shared_ptr<CGameObject>		Clone(any = g_aNull) override;
};

END