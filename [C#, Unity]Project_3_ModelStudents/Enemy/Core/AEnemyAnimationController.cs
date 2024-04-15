using Pathfinding;
using ProbyEditor;
using System;
using UnityEngine;

namespace ModelStudents
{
    [Author("Yoon")]
    public enum EnemyAnimationTrigger
    {
        Shoot,
        Reload,
        Dead,
    }

    [Author("Yoon")]
    public enum EnemyAnimationState
    {
        Idle,
        Chase,
        Retreat,
    }

    [Author("Yoon")]
    public abstract class AEnemyAnimationController : MonoBehaviour
    {
        public bool IsAnimatorEnabled
        {
            get
            {
                return _isAnimatorEnabled;
            }
            set
            {
                _isAnimatorEnabled = value;

                _animator.enabled = _isAnimatorEnabled;
                _networkAnimator.enabled = _isAnimatorEnabled;
            }
        }
        private bool _isAnimatorEnabled = true;

        [SerializeField]
        protected Animator _animator;
        protected NetworkAnimator _networkAnimator;

        [SerializeField]
        protected IAstarAI _aiPath;

        private void Awake()
        {
            _networkAnimator = GetComponent<NetworkAnimator>();

            _aiPath = GetComponent<IAstarAI>();
        }

        public abstract void SetTrigger(EnemyAnimationTrigger animation, Action endCallback = null);
        protected abstract void SetBool(EnemyAnimationState animation);
    }
}
