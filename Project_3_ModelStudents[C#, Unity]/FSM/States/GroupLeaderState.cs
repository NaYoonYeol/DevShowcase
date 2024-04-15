using System.Collections.Generic;
using Pathfinding;
using UnityEngine;
using ProbyEditor;
using UnityEditor;

namespace ModelStudents
{
    [Author("Yoon")]
    public class GroupLeaderState : AStateContainer, IDetectableNotify
    {
        private Transform _targetTransform;

        private AAIBehaviour _leader;
        private Transform _leaderTransform;

        private List<AAIBehaviour> _nearAllies;
        private List<IGroupChaseable> _groupMembers;

        private IAstarAI _aiPath;
        private bool _canChasing;

        private float _targetInformRange;

        private float _maxChasingTime;
        private float _chasingTime;

        private float _chasingSpeed;

        private float _searchPathInterval;
        private float _searchPathTime;

        public GroupLeaderState(AAIBehaviour leader, IAstarAI aiPath, GroupChaseStateData groupChaseStateData)
        {
            _aiPath = aiPath;

            _leader = leader;
            _leaderTransform = leader.transform;

            _groupMembers = new List<IGroupChaseable>();

            _targetInformRange = groupChaseStateData.SearchRange;
            _maxChasingTime = groupChaseStateData.MaxChasingTime;
            _chasingSpeed = groupChaseStateData.ChasingSpeed;

            _searchPathInterval = groupChaseStateData.SearchPathInterval;
        }

        public override void ExecuteBeginState()
        {
            _nearAllies = EnemyManager.FindNearbyEnemies(_leaderTransform.position, _targetInformRange);

            foreach (var nearByEnemy in _nearAllies)
            {
                // If nearby enemies are already in a group, do not join the group.
                if (nearByEnemy.StateEquals(EState.GroupChaseState) || nearByEnemy.StateEquals(EState.GroupLeaderState))
                {
                    continue;
                }

                // Incorporate neighbor allies into group members.
                if (nearByEnemy is IGroupChaseable groupMember)
                {
                    groupMember.JoinGroup(_leader as IGroupLeader, _chasingSpeed);
                    _groupMembers.Add(groupMember);
                }
            }
            _nearAllies = null;

            _canChasing = true;
            _chasingTime = 0;
            _searchPathTime = 0;

            _aiPath.maxSpeed = _chasingSpeed * NetworkTime.EnemyTimeRate;
            NetworkTime.EnemyTimeRateChanged += OnEnemyTimeRateChanged;

            DestinationUpdate();
        }

        public override void ExecuteState()
        {
            if (_aiPath.reachedEndOfPath)
            {
                _aiPath.isStopped = true;
                StopChasing();

                DestroyGroup();
            }

            if (_canChasing == false)
            {
                return;
            }

            float deltaTime = Time.deltaTime;

            _chasingTime += deltaTime;
            if (_chasingTime >= _maxChasingTime)
            {
                StopChasing();

                return;
            }

            _searchPathTime += deltaTime;
            if (_searchPathTime >= _searchPathInterval)
            {
                DestinationUpdate();

                _searchPathTime = 0;
            }
        }

        public override void ExecuteEndState()
        {
            _groupMembers.Clear();

            NetworkTime.EnemyTimeRateChanged -= OnEnemyTimeRateChanged;
        }

        public void ReleaseGroupMember(IGroupChaseable member)
        {
            int memberIndex = _groupMembers.IndexOf(member);
            if (memberIndex > -1)
            {
                _groupMembers.RemoveAt(memberIndex);
            }
        }

        public void StopChasing()
        {
            foreach (var groupMember in _groupMembers)
            {
                groupMember.StopGroupChasing();
            }

            _canChasing = false;
            _chasingTime = 0;
        }

        public void OnTargetUpdate(Transform targetTransform)
        {
            if (targetTransform == null)
            {
                return;
            }

            _targetTransform = targetTransform;
        }

        private void DestinationUpdate()
        {
            if (_targetTransform == null || _aiPath == null)
            {
                return;
            }

            _aiPath.destination = _targetTransform.position;
            _aiPath.SearchPath();

            foreach (var groupMember in _groupMembers)
            {
                groupMember.SetGroupChasePosition(_targetTransform.position - _leaderTransform.position);
            }
        }

        private void DestroyGroup()
        {
            IGroupLeader leader = _leader as IGroupLeader;
            leader.DestroyGroup();
        }

        private void OnEnemyTimeRateChanged(float timeRage)
        {
            _aiPath.maxSpeed = _chasingSpeed * NetworkTime.EnemyTimeRate;
        }
    }

    [Author("Yoon")]
    [System.Serializable]
    public class GroupChaseStateData : IStateData
    {
        [Title("GroupChasing Settings")]
        public float SearchRange;
        public float MaxChasingTime;
        public float SearchPathInterval;
        public float ChasingSpeed;

#if UNITY_EDITOR
        public void DrawGizmo(Transform transform)
        {
            // Target Inform Range
            {
                Handles.color = new Color(0, 0, 1);
                Handles.DrawWireDisc(transform.position, transform.up, SearchRange);
            }
        }
#endif
    }

    [Author("Yoon")]
    public interface IGroupLeader
    {
        GroupLeaderState GroupLeaderState { get; }

        void CreateGroup();
        void DestroyGroup();

        void ReleaseGroupMember(IGroupChaseable member);
    }
}
