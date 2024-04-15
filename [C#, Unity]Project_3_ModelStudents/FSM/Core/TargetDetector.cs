using System.Collections.Generic;
using UnityEngine;
using ProbyEditor;
using System;

namespace ModelStudents
{
    [Author("Yoon")]
    public class TargetDetector
    {
        private const int MAX_TARGET_COUNT = 16;
        private static readonly Collider[] TARGET_COLLIDERS = new Collider[MAX_TARGET_COUNT];

        private Transform _targetTransform;
        private Transform _ownerTransform;

        private List<IDetectableNotify> _detectableStates = new List<IDetectableNotify>();

        private bool _isExploringAllDirections;

        public TargetDetector(Transform ownerTransform)
        {
            _ownerTransform = ownerTransform;

            _isExploringAllDirections = false;
        }

        public void DetectTarget(DetectableStateData detectableStateData)
        {
            _targetTransform = null;

            Physics.OverlapSphereNonAlloc(_ownerTransform.position, detectableStateData.DetectionDistance, TARGET_COLLIDERS, detectableStateData.DetectionLayer);

            float minDistance = float.MaxValue;
            for (int i = 0; i < MAX_TARGET_COUNT; ++i)
            {
                if (TARGET_COLLIDERS[i] == null)
                {
                    continue;
                }

                Transform nearCollider = TARGET_COLLIDERS[i].transform;
                Vector3 ownerPosition = _ownerTransform.position;
                Vector3 targetPosition = nearCollider.position;

                Vector3 targetDirection = (targetPosition - ownerPosition).normalized;

                if (_isExploringAllDirections)
                {
                    FindNearTarget();
                }
                else
                {
                    float cosTheta = Vector3.Dot(_ownerTransform.forward, targetDirection);
                    float detectionCosTheta = Mathf.Abs(Mathf.Cos(Mathf.Deg2Rad * detectableStateData.DetectionAngle));

                    if (cosTheta >= detectionCosTheta)
                    {
                        FindNearTarget();
                    }
                }

                TARGET_COLLIDERS[i] = null;

                void FindNearTarget()
                {
                    float distance = Vector3.SqrMagnitude(targetPosition - ownerPosition);
                    if (distance < minDistance)
                    {
                        minDistance = distance;

                        if (DetectObstacle(targetPosition, detectableStateData.ObstacleLayer) == false)
                        {
                            _targetTransform = nearCollider;
                        }
                    }
                }
            }

            NotifyDetectedTarget();
        }

        public void SetExploringAllDirections(bool isAllDirection)
        {
            _isExploringAllDirections = isAllDirection;
        }

        public void AttachDetectableState(IDetectableNotify detect)
        {
            _detectableStates.Add(detect);
        }

        private void NotifyDetectedTarget()
        {
            for (int i = 0; i < _detectableStates.Count; ++i)
            {
                _detectableStates[i].OnTargetUpdate(_targetTransform);
            }
        }

        private bool DetectObstacle(Vector3 targetPosition, LayerMask obstacleLayer)
        {
            Vector3 ownerPosition = _ownerTransform.position;
            float distance = Vector3.Distance(ownerPosition, targetPosition);

            return Physics.Raycast(ownerPosition, (targetPosition - ownerPosition).normalized, distance, obstacleLayer);
        }
    }

    [Author("Yoon")]
    [System.Serializable]
    public class DetectableStateData : IStateData
    {
        public float DetectionDistance;
        public float DetectionAngle;

        public LayerMask DetectionLayer;
        public LayerMask ObstacleLayer;
    }

    [Author("Yoon")]
    public interface IDetectableNotify
    {
        public void OnTargetUpdate(Transform targetTransform);
    }
}