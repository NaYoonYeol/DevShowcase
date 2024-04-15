using Pathfinding;
using UnityEngine;
using ProbyEditor;

namespace ModelStudents
{
    [Author("Yoon")]
    public class RetreatState : AStateContainer, IDetectableNotify
    {
        private readonly float _retreatBufferZone;

        private Transform _targetTransform;

        private RichAI _aiPath;

        private Transform _ownerTransform;
        private float _retreatRange;
        private float _retreatSpeed;

        private bool _isUpdated = false;

        public RetreatState(Transform ownerTransform, RichAI aiPath, float retreatRange, RetreatStateData retreatStateData)
        {
            _ownerTransform = ownerTransform;

            _retreatRange = retreatRange;
            _retreatBufferZone = _retreatRange - 1;

            _retreatSpeed = retreatStateData.RetreatSpeed;

            _aiPath = aiPath;
        }

        public override void ExecuteBeginState()
        {
            _isUpdated = false;

            _aiPath.maxSpeed = _retreatSpeed * NetworkTime.EnemyTimeRate;
            NetworkTime.EnemyTimeRateChanged += OnEnemyTimeRateChanged;
        }

        public override void ExecuteState()
        {
            if (_isUpdated == false || _targetTransform == null)
            {
                return;
            }

            Vector3 ownerPosition = _ownerTransform.position;
            Vector3 targetPosition = _targetTransform.position;
            float distance = Vector3.Distance(ownerPosition, targetPosition);
            if (distance < _retreatRange)
            {
                if (distance >= _retreatBufferZone)
                {
                    _aiPath.isStopped = true;

                    return;
                }

                Vector3 destination = ownerPosition + ((ownerPosition - targetPosition).normalized * _retreatRange);
                destination.y = ownerPosition.y;

                _aiPath.destination = destination;

                _aiPath.SearchPath();
                _aiPath.isStopped = false;
            }

            _isUpdated = false;
        }

        public override void ExecuteEndState()
        {
            NetworkTime.EnemyTimeRateChanged -= OnEnemyTimeRateChanged;
        }

        public void OnTargetUpdate(Transform targetTransform)
        {
            _targetTransform = targetTransform;

            _isUpdated = true;
        }

        private void OnEnemyTimeRateChanged(float timeRage)
        {
            _aiPath.maxSpeed = _retreatSpeed * NetworkTime.EnemyTimeRate;
        }
    }

    [Author("Yoon")]
    [System.Serializable]
    public class RetreatStateData : IStateData
    {
        [Title("Retreat Settings")]
        public float RetreatSpeed;
    }
}