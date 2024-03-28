#pragma once

#include "State.h"
#include "ClientPCH.h"

template <class T>
class CStateTemplate : public CState
{
public:
	CStateTemplate() DEFAULT;
	CStateTemplate(const CStateTemplate& _other) DEFAULT;
	virtual ~CStateTemplate() DEFAULT;

	CStateTemplate& operator=(const CStateTemplate& _other) DEFAULT;

protected:
	typedef void (T::* PFNSTATE)(void);
	typedef void (T::* PFNEXCUTESTATE)(_float);
	weak_ptr<T> m_pInstance;
	PFNSTATE m_pfnBeginState = { nullptr };
	PFNEXCUTESTATE m_pfnState = { nullptr };
	PFNSTATE m_pfnEndState = { nullptr };

public:
	void Set(weak_ptr<T> _pInstance, PFNSTATE _pfnBeginState, PFNEXCUTESTATE _pfnState, PFNSTATE _pfnEndState)
	{
		assert(_pInstance.lock());
		m_pInstance = _pInstance;
		assert(_pfnBeginState);
		m_pfnBeginState = _pfnBeginState;
		assert(_pfnState);
		m_pfnState = _pfnState;
		assert(_pfnEndState);
		m_pfnEndState = _pfnEndState;
	}

	virtual void ExecuteBeginState()
	{
		assert(m_pInstance.lock() && m_pfnBeginState);
		(m_pInstance.lock().get()->*m_pfnBeginState)();
	}
	virtual void ExecuteState(_float _fTimeDelta)
	{
		assert(m_pInstance.lock() && m_pfnState);
		(m_pInstance.lock().get()->*m_pfnState)(_fTimeDelta);
	}
	virtual void ExecuteEndState()
	{
		assert(m_pInstance.lock() && m_pfnEndState);
		(m_pInstance.lock().get()->*m_pfnEndState)();
	}
};

