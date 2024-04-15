using Fusion;
using ProbyEditor;
using System;
using UnityEditor;
using UnityEngine;
using Random = UnityEngine.Random;

namespace ModelStudents
{
    [Author("Yoon")]
    public class RandomLocationAttackState : ARandomAttackState
    {
        private Vector3 _baseLocation;
        private float _radius;

        private Vector3 _randomLocation;

        private Transform _owner;

        private Action _endOfState;

        public void InitState(Transform owner, NetworkRunner runner, RandomLocationAttackStateData attackStateData, Action endOfState)
        {
            base.InitState(runner, attackStateData);

            _baseLocation = attackStateData.Center;
            _radius = attackStateData.Radius;

            _owner = owner;

            _endOfState = endOfState;
        }

        protected override bool DetectTarget()
        {
            _randomLocation = _owner.position + _baseLocation;
            _randomLocation.x += Random.Range(-_radius, _radius);
            _randomLocation.z += Random.Range(-_radius, _radius);

            return true;
        }

        protected override void Attack()
        {
            if (_currentCirculationCount >= _maxCirculationCount)
            {
                _currentCirculationCount = 0;
                _endOfState?.Invoke();

                return;
            }

            _weapon.Fire(_runner, _randomLocation);
        }
    }

    [Author("Yoon")]
    [System.Serializable]
    public class RandomLocationAttackStateData : RandomAttackStateData
    {
        public Vector3 Center;
        public float Radius;

#if UNITY_EDITOR
        public void DrawRandomLocationRangeGizmo(Transform transform)
        {
            // Detection Gizmo
            {
                Handles.color = new Color(1, 0, 0, 0.2f);

                Handles.DrawSolidDisc(transform.position + Center, Vector3.up, Radius);
            }
        }
#endif
    }
}

