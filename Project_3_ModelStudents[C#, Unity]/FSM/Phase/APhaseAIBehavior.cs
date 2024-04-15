using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using ProbyEditor;

namespace ModelStudents
{
    [Author("Yoon")]
    [System.Serializable]
    public class StateContainerList
    {
        [SerializeField]
        public List<ContainerDataPair> ContainerDataPairs = new List<ContainerDataPair>();
        [SerializeField]
        public List<ContainerDataPair> StaticContainerDataPairs = new List<ContainerDataPair>();

        [SerializeField]
        public ContainerDataPair TransferState;
    }

    [Author("Yoon")]
    [System.Serializable]
    public struct ContainerDataPair
    {
        [SerializeField]
        public EState State;
        [SerializeReference]
        public AStateContainer Container;
        [SerializeReference]
        public IStateData Data;

        public ContainerDataPair(EState state, AStateContainer container, IStateData data)
        {
            State = state;
            Container = container;
            Data = data;
        }
    }

    [Author("Yoon")]
    [System.Serializable]
    public abstract class APhaseAIBehavior : AAIBehaviour
    {
#if UNITY_EDITOR
        [SerializeField]
        public StateContainerList[] PhaseStateContainers => _phaseStateDataPairs;
#endif

        public const int PHASE_COUNT = 5;

        [SerializeReference]
        protected StateContainerList[] _phaseStateDataPairs = new StateContainerList[PHASE_COUNT]
        {
            new StateContainerList(),
            new StateContainerList(),
            new StateContainerList(),
            new StateContainerList(),
            new StateContainerList()
        };

#if UNITY_EDITOR
        [Header("Gizmo Settings")]
        [SerializeField]
        private bool _onDrawGizmo;

        private void OnDrawGizmos()
        {
            if (_onDrawGizmo == false)
            {
                return;
            }

            foreach (var state in _phaseStateDataPairs)
            {
                foreach (var container in state.ContainerDataPairs)
                {
                    switch (container.State)
                    {
                        case EState.RandomLocationAttackState:
                            ((RandomLocationAttackStateData)container.Data).DrawRandomLocationRangeGizmo(transform);
                            break;
                    }
                }
            }
        }
#endif
    }
}