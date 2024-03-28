#pragma once
#include "Client_Define.h"
#include "GameObject.h"

BEGIN(Engine)
class CVIBuffer_Cube;
class CObjectPool;
END

BEGIN(Client)

class CDecal abstract : public CGameObject
{
protected:
	explicit CDecal(ComPtr<ID3D11Device>, ComPtr<ID3D11DeviceContext>);
	explicit CDecal(const CDecal&);
	virtual ~CDecal() DEFAULT;

public:
	virtual HRESULT						Initialize_Prototype() override;
	virtual HRESULT						Initialize(any = g_aNull) override;
	virtual void						Tick(_float _fTimeDelta) override;
	virtual void						Late_Tick(_float _fTimeDelta) override;
	virtual HRESULT						Render() override;

public:
	virtual HRESULT						Fetch(any = g_aNull) override;

protected:
	virtual HRESULT						Ready_Components() override;
	HRESULT								Render(_uint _iPassIndex);

protected:
	shared_ptr<CTransform>				m_pTransform;
	shared_ptr<CShader>					m_pShader;
	shared_ptr<CVIBuffer_Cube>			m_pVIBuffer_Cube;

protected:
	_bool								m_bFetched = { false };

public:
	virtual shared_ptr<CGameObject>		Clone(any = g_aNull) PURE;
};

END