#pragma once
#include "Engine_Define.h"

BEGIN(Engine)

class CInteractiveObjectFinder final : public CSingleton<CInteractiveObjectFinder>
{
private:
	explicit CInteractiveObjectFinder() DEFAULT;
	virtual ~CInteractiveObjectFinder() DEFAULT;

public:
	void										Initialize(_uint _iWinSizeX, _uint _iWinSizeY);
	
public:
	std::shared_ptr<class CInteractiveObject>	Get_FocusedObject();
	std::shared_ptr<class CInteractiveObject>	Get_FocusedSpecialObject();
	
public:
	HRESULT										Append_InteractiveObject(const SCENE _eScene, const wstring& _wstrLayerTag);
	void										Add_InteractiveObject(const std::shared_ptr<class CInteractiveObject>&);
	
public:
	void										Tick();
	void										Clear();

private:
	void										Tick_Objects();
	
private:
	std::shared_ptr<class CInteractiveObject>				m_pFocusedObject;
	std::vector<std::shared_ptr<class CInteractiveObject>>	m_vecInteractiveObject;

	vector<shared_ptr<class CInteractiveObject>>			m_vecCapturedObjects;

private:
	_uint													m_iWinSizeX = { 0 };
	_uint													m_iWinSizeY = { 0 };

	_float2													m_vScreenCenterPosition = {};

private:
	_float													fScreenWeight = { 1.f };
	_float													fWorldWeight = { 1.5f };

private:
	_float m_fFocusingDistanceThreshold = { 30.f };

public:
	friend CSingleton<CInteractiveObjectFinder>;
	void EraseObject(shared_ptr<CInteractiveObject> _obj);
};

END