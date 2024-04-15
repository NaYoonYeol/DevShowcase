using UnityEngine;
using ProbyEditor;
using Pathfinding;
using Fusion;
using UnityEditor;

namespace ModelStudents
{
    [Author("Yoon")]
    public class SoldierAttackState : AAttackState, IDetectableNotify
    {
        private NetworkRunner _runner;
        private AEnemyAnimationController _enemyAnimationController;

        private AEnemyWeapon _weapon;
        private IAstarAI _aiPath;

        private Transform _targetTransform;

        private TargetDetector _detectionTable;
        private AttackStateData _attackStateData;

        public SoldierAttackState()
        {
        }

        public SoldierAttackState(NetworkRunner runner, AEnemyAnimationController animator, IAstarAI aiPath,
            AEnemyWeapon weapon, TargetDetector targetDetector, AttackStateData attackStateData)
        {
            _runner = runner;
            _enemyAnimationController = animator;

            _weapon = weapon;
            _aiPath = aiPath;

            _detectionTable = targetDetector;
            _attackStateData = attackStateData;

            SetAttackInterval(attackStateData.AttackInterval);
        }

        public void InitState(NetworkRunner runner, AEnemyAnimationController animator, IAstarAI aiPath,
            AEnemyWeapon weapon, TargetDetector targetDetector, AttackStateData attackStateData)
        {
            _runner = runner;
            _enemyAnimationController = animator;

            _weapon = weapon;
            _aiPath = aiPath;

            _detectionTable = targetDetector;
            _attackStateData = attackStateData;

            SetAttackInterval(attackStateData.AttackInterval);
        }

        protected override bool DetectTarget()
        {
            _detectionTable.DetectTarget(_attackStateData);
            if (_targetTransform == null)
            {
                _aiPath.isStopped = false;
                return false;
            }

            return true;
        }

        protected override void Attack()
        {
            _aiPath.isStopped = true;

            _weapon.AppendTime(_attackDelay);
            _weapon.Fire(_runner, _targetTransform.position);

            _enemyAnimationController.SetTrigger(EnemyAnimationTrigger.Shoot);
        }

        protected override bool Reload()
        {
            if (_weapon.IsEmpty())
            {
                _weapon.Reload();

                PauseState();
                _enemyAnimationController.SetTrigger(EnemyAnimationTrigger.Reload, OnEndofReloading);

                return true;
            }

            return false;
        }

        private void OnEndofReloading()
        {
            PlayState();
        }

        public void OnTargetUpdate(Transform targetTransform)
        {
            _targetTransform = targetTransform;
        }
    }

    [Author("Yoon")]
    [System.Serializable]
    public class AttackStateData : DetectableStateData
    {
        public float AttackInterval;

#if UNITY_EDITOR
        public void DrawAttackRangeGizmo(Transform transform)
        {
            // Attack Range Gizmo
            {
                Handles.color = new Color(0, 1, 0, 0.2f);
                
                Vector3 direction = Quaternion.AngleAxis((180 - DetectionAngle) * 0.5f, Vector3.up) * -transform.right;
                Handles.DrawSolidArc(transform.position, transform.up, direction, DetectionAngle, DetectionDistance);
            }
        }
#endif
    }
}