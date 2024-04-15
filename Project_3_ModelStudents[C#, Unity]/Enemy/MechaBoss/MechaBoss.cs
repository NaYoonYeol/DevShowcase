using Fusion;
using Pathfinding;
using ProbyEditor;
using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

namespace ModelStudents
{
    [Author("Yoon")]
    public class MechaBoss : APhaseAIBehavior
    {
        // TODO : �ӽ÷� ��� ������ �����! ���߿� ���� �������� �������� ��� ����� �������� ���� �Ǵ��ؾ��� ~,~
        private const int CURRENT_STAGE = 4;

        private float _nextPhasePercentage = 100;
        private float _dividedHealth;
        //

        private RichAI _pathFinder;
        private MechaBossAnimationController _mechaBossAnimationController;

        private StateContainerList _currentPhase;
        private int _currentPhaseIndex = 0;
        private int _lastStateIndex = 0;

        private EnemyHealth _enemyHealth;

        public override void Despawned(NetworkRunner runner, bool hasState)
        {
            base.Despawned(runner, hasState);

            _enemyHealth.HealthChanged -= OnHealthChanged;
        }

        public override void InitializeStateMachine()
        {
            InitializeState();

            _pathFinder = GetComponent<RichAI>();
            _mechaBossAnimationController = GetComponent<MechaBossAnimationController>();

            _currentPhase = _phaseStateDataPairs[_currentPhaseIndex];
            InitStateList(_currentPhase);

            InitializeState(_currentPhase.ContainerDataPairs[_lastStateIndex].Container);

            _enemyHealth = GetComponent<EnemyHealth>();
            _enemyHealth.HealthChanged += OnHealthChanged;

            _dividedHealth = _nextPhasePercentage / CURRENT_STAGE;
            _nextPhasePercentage -= _dividedHealth;
        }

        private void InitState(ContainerDataPair stateContainer, Action goToNextState)
        {
            switch (stateContainer.State)
            {
                case EState.RandomTargetAttackState:
                    RandomTargetAttackState randomTargetAttackState = (RandomTargetAttackState)stateContainer.Container;
                    randomTargetAttackState.InitState(Runner, (RandomAttackStateData)stateContainer.Data, goToNextState);
                    break;

                case EState.RandomLocationAttackState:
                    RandomLocationAttackState randomLocationAttackState = (RandomLocationAttackState)stateContainer.Container;
                    randomLocationAttackState.InitState(transform, Runner, (RandomLocationAttackStateData)stateContainer.Data, goToNextState);
                    break;

                case EState.EmissionState:
                    EmissionState emissionState = (EmissionState)stateContainer.Container;
                    emissionState.InitState(transform, GetComponent<NetworkObject>().Id, 
                        (ExplosiveStateData)stateContainer.Data, goToNextState);
                    break;

                case EState.DetectingExplosiveState:
                    ExplosiveState explosiveState = (ExplosiveState)stateContainer.Container;
                    explosiveState.InitState(transform, GetComponent<NetworkObject>().Id, 
                        (DetectingExplosiveStateData)stateContainer.Data, goToNextState);
                    break;

                case EState.MultipleSpiralState:
                    MultipleSpiralState multipleSpiralState = (MultipleSpiralState)stateContainer.Container;
                    multipleSpiralState.InitState(Runner, transform, (MultipleSprialStateData)stateContainer.Data, goToNextState);
                    break;

                case EState.JumpState:
                    JumpState jumpState = (JumpState)stateContainer.Container;
                    jumpState.InitState(transform, GetComponent<NetworkObject>().Id, (JumpStateData)stateContainer.Data, goToNextState);
                    break;

                case EState.AttackableJumpState:
                    AttackableJumpState attackableJumpState = (AttackableJumpState)stateContainer.Container;
                    attackableJumpState.InitState(transform, GetComponent<NetworkObject>().Id, Runner, 
                        (AttackableJumpStateData)stateContainer.Data, goToNextState);
                    break;

                case EState.SuicideState:
                    SuicideState suicideState = (SuicideState)stateContainer.Container;
                    suicideState.InitState(transform, _mechaBossAnimationController, GetComponent<NetworkObject>().Id, 
                        (SuicideStateData)stateContainer.Data, null);
                    break;

                case EState.ChasableSuicideState:
                    ChasableSuicideState chasableSuicideState = (ChasableSuicideState)stateContainer.Container;
                    chasableSuicideState.InitState(transform, _pathFinder, _mechaBossAnimationController, GetComponent<NetworkObject>().Id,
                        (ChasableSuicideStateData)stateContainer.Data, null);
                    break;

                default:
                    break;
            }
        }

        private void InitStateList(StateContainerList stateContainerList)
        {
            InitState(stateContainerList.TransferState, GotoNextState);

            foreach (var stateContainer in stateContainerList.ContainerDataPairs)
            {
                InitState(stateContainer, GotoNextState);
            }

            foreach (var staticStateContainer in stateContainerList.StaticContainerDataPairs)
            {
                InitState(staticStateContainer, null);
                AddStaticState(staticStateContainer.Container);
            }
        }

        private void GotoNextState()
        {
            ++_lastStateIndex;

            int totalIndex = _currentPhase.ContainerDataPairs.Count;
            if (totalIndex > 0)
                _lastStateIndex %= totalIndex;

            ContainerDataPair pair = _currentPhase.ContainerDataPairs[_lastStateIndex];
            TransferState(pair.State, pair.Container);
        }

        private void OnHealthChanged(float currentHealth, float maxHealth)
        {
            float percentage = currentHealth / maxHealth * 100;
            if (percentage <= _nextPhasePercentage)
            {
                SwitchToNestPhase();
            }

            void SwitchToNestPhase()
            {
                ClearStaticState();

                _currentPhase = _phaseStateDataPairs[++_currentPhaseIndex];
                InitStateList(_currentPhase);

                ContainerDataPair pair;
                if (_currentPhase.TransferState.State == EState.None)
                {
                    _lastStateIndex = 0;
                    pair = _currentPhase.ContainerDataPairs[_lastStateIndex];
                }
                else
                {
                    pair = _currentPhase.TransferState;
                    _lastStateIndex = -1;
                }

                TransferState(pair.State, pair.Container);

                _nextPhasePercentage -= _dividedHealth;
            }
        }
    }
}