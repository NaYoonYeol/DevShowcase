#include "EnginePCH.h"
#include "FSM.h"

void CFSM::Tick(_float _fTimeDelta)
{
	if (m_pNewState)
	{
		m_pCurrentState->ExecuteEndState();
		m_pCurrentState = m_pNewState;
		m_pNewState = nullptr;
		m_pCurrentState->ExecuteBeginState();
	}
	m_pCurrentState->ExecuteState(_fTimeDelta);

	ExecutePersistent(_fTimeDelta);
}

bool CFSM::IsState(const shared_ptr<CState>& _pState)
{
	return (m_pCurrentState == _pState);
}

bool CFSM::GotoState(const shared_ptr<CState> _pNewState)
{
	m_pNewState = _pNewState;
	return true;
}


