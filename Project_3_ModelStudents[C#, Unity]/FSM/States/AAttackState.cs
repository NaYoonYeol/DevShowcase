using ProbyEditor;
using UnityEngine;

namespace ModelStudents
{
    [Author("Yoon")]
    public abstract class AAttackState : AStateContainer
    {
        protected float _attackDelay;
        private float _attackInterval;

        private bool _isPaused;

        public void SetAttackInterval(float attackInterval)
        {
            _attackInterval = attackInterval; 
        }

        public override void ExecuteBeginState()
        {
            Debug.Assert(_attackInterval > 0, "Attack Interval cannot be less than or equal to 0. " +
                "Verify that you have initialized attack Interval with the SetAttackInterval() function.");

            _isPaused = false;
            _attackDelay = 0f;
        }

        public override void ExecuteState()
        {
            if (_isPaused)
            {
                return;
            }

            _attackDelay += NetworkTime.EnemyTimeRate * Time.deltaTime;
            if (_attackDelay < _attackInterval)
            {
                return;
            }

            if (Reload())
            {
                return;
            }

            if (DetectTarget())
            {
                Attack();
                _attackDelay = 0.0f;
            }
        }

        public override void ExecuteEndState()
        {
        }

        protected void PlayState()
        {
            _isPaused = false;
        }

        protected void PauseState()
        {
            _isPaused = true;
        }

        protected abstract bool Reload();
        protected abstract void Attack();
        protected abstract bool DetectTarget();
    }
}
