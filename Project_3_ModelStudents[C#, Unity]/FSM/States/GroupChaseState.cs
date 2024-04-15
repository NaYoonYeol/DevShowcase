using UnityEngine;
using Pathfinding;
using ProbyEditor;

namespace ModelStudents
{
    [Author("Yoon")]
    public class GroupChaseState : AStateContainer
    {
        private IGroupLeader _leader;
        private float _chasingSpeed;

        private IAstarAI _aiPath;
        private IGroupChaseable _owner;

        public GroupChaseState(IGroupChaseable owner, IAstarAI aiPath)
        {
            _owner = owner;
            _aiPath = aiPath;
        }

        public override void ExecuteBeginState()
        {
            _aiPath.maxSpeed = _chasingSpeed * NetworkTime.EnemyTimeRate;
            NetworkTime.EnemyTimeRateChanged += OnEnemyTimeRateChanged;
        }

        public override void ExecuteState()
        {
            if (_aiPath.reachedEndOfPath)
            {
                _aiPath.isStopped = true;
            }
        }

        public override void ExecuteEndState()
        {
            NetworkTime.EnemyTimeRateChanged -= OnEnemyTimeRateChanged;
        }

        public void InitializeChaseState(IGroupLeader owner, float chasingSpeed)
        {
            _leader = owner;
            _chasingSpeed = chasingSpeed;
        }

        public void SearchPath(Vector3 targetPosition)
        {
            _aiPath.destination = targetPosition;
            _aiPath.SearchPath();
        }

        public void LeaveGroup()
        {
            if (_leader == null)
            {
                return;
            }

            _leader.ReleaseGroupMember(_owner);
            _leader = null;
        }

        private void OnEnemyTimeRateChanged(float timeRage)
        {
            _aiPath.maxSpeed = _chasingSpeed * NetworkTime.EnemyTimeRate;
        }
    }

    [Author("Yoon")]
    public interface IGroupChaseable
    {
        GroupChaseState GroupChaseState { get; }

        void SetGroupChasePosition(Vector3 direction);
        void StopGroupChasing();

        void JoinGroup(IGroupLeader leader, float chasingSpeed);
        void LeaveGroup();
    }
}
