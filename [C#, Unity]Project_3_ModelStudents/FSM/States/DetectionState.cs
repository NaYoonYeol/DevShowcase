using UnityEngine;
using ProbyEditor;
using UnityEditor;
using DG.Tweening;

namespace ModelStudents
{
    [Author("Yoon")]
    public class DetectionState : AStateContainer, IDetectableNotify
    {
        private const float TWEEN_SPEED = 0.1f;

        private float _retreatRange;
        private bool _isRetreable;

        private float _detectionInterval;
        private float _detectionDelay;

        private Transform _target;
        private Transform _ownerTransform;

        private TargetDetector _detectionTable;
        private DetectionStateData _detectionStateData;

        public DetectionState()
        {
        }

        public DetectionState(Transform ownerTransform, TargetDetector targetDetector, DetectionStateData detectionStateData)
        {
            _detectionStateData = detectionStateData;
            _detectionInterval = _detectionStateData.DetectionInterval;

            _retreatRange = _detectionStateData.RetreatRange;
            _isRetreable = _detectionStateData.IsRetreatable;

            _ownerTransform = ownerTransform;
            _detectionTable = targetDetector;
        }

        public void InitState(Transform ownerTransform, TargetDetector targetDetector, DetectionStateData detectionStateData)
        {
            _detectionStateData = detectionStateData;
            _detectionInterval = _detectionStateData.DetectionInterval;

            _retreatRange = _detectionStateData.RetreatRange;
            _isRetreable = _detectionStateData.IsRetreatable;

            _ownerTransform = ownerTransform;
            _detectionTable = targetDetector;
        }

        public override void ExecuteBeginState()
        {
            _target = null;

            _detectionDelay = 0.0f;
        }

        public override void ExecuteState()
        {
            _detectionDelay += Time.deltaTime;
            if (_detectionDelay < _detectionInterval)
            {
                return;
            }

            _detectionTable.DetectTarget(_detectionStateData);
            if (_target != null)
            {
                _ownerTransform.DOLookAt(_target.position, TWEEN_SPEED, AxisConstraint.Y);

                if (_isRetreable)
                {
                    if (TryRetreat() == false)
                    {
                        CreateGroup();
                    }
                }
                else
                {
                    CreateGroup();
                }
            }

            _detectionDelay = 0.0f;
        }

        public override void ExecuteEndState()
        {
        }

        public void OnTargetUpdate(Transform targetTransform)
        {
            _target = targetTransform;
        }

        private bool TryRetreat()
        {
            float distance = Vector3.Distance(_ownerTransform.position, _target.position);
            if (distance < _retreatRange)
            {
                RetreatRangeEnter();

                return true;
            }

            return false;
        }

        private void RetreatRangeEnter()
        {
            IDetectable owner = _ownerTransform.GetComponent<AAIBehaviour>() as IDetectable;
            if (owner != null)
            {
                owner.OnRetreatRangeEnter();
            }
        }

        private void CreateGroup()
        {
            IGroupLeader owner = _ownerTransform.GetComponent<AAIBehaviour>() as IGroupLeader;
            if (owner != null)
            {
                owner.CreateGroup();
            }
        }
    }

    [Author("Yoon")]
    [System.Serializable]
    public class DetectionStateData : DetectableStateData
    {
        public float DetectionInterval;

        public bool IsRetreatable;
        public float RetreatRange;

#if UNITY_EDITOR
        private const float RETREAT_BUFFER_ZONE = 1.0f;

        public void DrawDetectionRangeGizmo(Transform transform)
        {
            // Detection Gizmo
            {
                Handles.color = new Color(1, 0, 0, 0.2f);

                Vector3 direction = Quaternion.AngleAxis((180 - DetectionAngle) * 0.5f, Vector3.up) * -transform.right;
                Handles.DrawSolidArc(transform.position, transform.up, direction, DetectionAngle, DetectionDistance);
            }

            // Retreat Range Gizmo
            {
                Handles.color = new Color(0, 0, 1, 0.2f);

                Vector3 direction = Quaternion.AngleAxis((180 - DetectionAngle) * 0.5f, Vector3.up) * -transform.right;
                Handles.DrawSolidArc(transform.position, transform.up, direction, DetectionAngle, RetreatRange - RETREAT_BUFFER_ZONE);
            }
        }
#endif
    }

    [Author("Yoon")]
    public interface IDetectable
    {
        DetectionState DetectionState { get; }

        void OnTargetDetected(Vector3 direction);

        void OnRetreatRangeEnter();
    }
}