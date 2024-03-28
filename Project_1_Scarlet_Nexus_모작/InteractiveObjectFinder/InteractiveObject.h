#pragma once
#include "Engine_Define.h"
#include "GameObject.h"

BEGIN(Engine)

class ENGINE_DLL CInteractiveObject : public CGameObject
{
protected:
	explicit CInteractiveObject(ComPtr<ID3D11Device>, ComPtr<ID3D11DeviceContext>);
	explicit CInteractiveObject(const CInteractiveObject&);
	virtual ~CInteractiveObject() DEFAULT;

public:
	virtual HRESULT												Initialize(any = g_aNull);
	virtual void												Tick(_float _fTimeDelta);
	virtual void												Late_Tick(_float _fTimeDelta);
	virtual HRESULT												Render();
	virtual HRESULT												Render(_uint _iPassIndex, _bool _bOrthographic = false);

public:
	void														Add_RenderObject_Outline() { Add_RenderObject(RENDER_GROUP::OUTLINE); }

public:
	_float														Get_Weight() { return m_fWeight; }
	void														Set_Weight(_float _fWeight) { m_fWeight = _fWeight; }

public:
	_bool														Get_Focused() { return m_bFocused; }
	void														On_Focusing() { m_bFocused = true; }

	_bool														Get_Interactivable() { return m_bInteractivable; }

public:
	void														Create_Light(shared_ptr<class CTransform>);
	virtual void												Interactive_PhychoKinesis(_bool _bInteractive);

protected:
	virtual HRESULT												Ready_Components() override;

protected:
	_bool														m_bFocused = { false };
	_bool														m_bInteractivable = { true };
private:
	_float														m_fWeight = { 3.5f };

protected:
	_float														m_fOutLineSize = { 0.03f };
	_float3														m_vOutlineColor = { 1.f, 1.f, 1.f };

protected:
	shared_ptr<class CLight>									m_pLight;

private:
	_bool														m_bEnableLight = { false };

public:
	virtual shared_ptr<CGameObject>								Clone(any = g_aNull);
};

END