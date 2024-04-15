using System;
using Fusion;
using ProbyEditor;
using UnityEngine;
using System.Collections.Generic;

namespace ModelStudents
{
    [Author("Yoon")]
    public abstract class AAIBehaviour : NetworkBehaviour
    {
        public bool EnableExecuteState { get; set; } = true;

        public Action BeginStateInitial;
        public Action StateInitial;
        public Action EndStateInitial;

        private EState _currentState;
        private InstanceState _currentStateInstance;

        private List<AStateContainer> _staticStates = new List<AStateContainer>();
        private int _staticStatesCount = 0;

        public override void Spawned()
        {
            if (Runner.IsServer == false)
            {
                GetComponent<RiflemanAnimationController>().enabled = false;
            }

            InitializeStateMachine();
        }

        public abstract void InitializeStateMachine();

        public void InitializeState(AStateContainer state = null)
        {
            _currentStateInstance = state == null ? new InstanceState(this) : new InstanceState(this, state);
        }

        public void AddStaticState(AStateContainer state)
        {
            _staticStates.Add(state);
            ++_staticStatesCount;
        }

        public void RemoveStaticState(AStateContainer state)
        {
            _staticStates.Remove(state);
        }

        public void ClearStaticState()
        {
            _staticStates.Clear();
            _staticStatesCount = 0;
        }

        public bool StateEquals(EState state)
        {
            return _currentState == state;
        }

        public void TransferState(EState state, AStateContainer newState)
        {
            _currentStateInstance.ExecuteEndState();

            _currentState = state;
            _currentStateInstance.TransferState(newState);
            _currentStateInstance.ExecuteBeginState();
        }

        public void RemoveCurrentState()
        {
            _currentStateInstance.ExecuteEndState();

            _currentState = EState.None;
            _currentStateInstance = new InstanceState(this);
        }

        private void Update()
        {
            if (EnableExecuteState == false)
            {
                return;
            }
            
            for (int i = 0; i < _staticStatesCount; ++i)
            {
                _staticStates[i].StateAction();
            }

            _currentStateInstance?.ExecuteState();
        }
    }

    [Author("Yoon")]
    public enum EState
    {
        None,
        DetectionState,
        GroupLeaderState,
        GroupChaseState,
        SoldierAttackState,
        RetreatState,
        RandomTargetAttackState,
        RandomLocationAttackState,
        EmissionState,
        DetectingExplosiveState,
        MultipleSpiralState,
        JumpState,
        AttackableJumpState,
        SuicideState,
        ChasableSuicideState,
    }
}