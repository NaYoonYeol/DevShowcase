using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using ProbyEditor;
using System;
using Pathfinding;

namespace ModelStudents
{
    [RequireComponent(typeof(AAIBehaviour), typeof(NetworkAnimator))]
    [Author("Yoon")]
    public class RiflemanAnimationController : AEnemyAnimationController
    {
        private const string ANIMATION_NAME_CHASE = "chase";
        private const string ANIMATION_NAME_RETREAT = "retreat";

        private const string ANIMATION_NAME_SHOOT = "shoot";
        private const string ANIMATION_NAME_RELOAD = "reload";

        [SerializeField]
        private float _reloadDuration;
        private WaitForSeconds _reloadWaitForSeconds;

        private bool _shootTrigger;
        private bool _reloadTrigger;

        private void Awake()
        {
            _reloadWaitForSeconds = new WaitForSeconds(_reloadDuration);

            NetworkTime.EnemyTimeRateChanged += _networkAnimator.RPC_SetAnimatorSpeed;
        }

        private void OnDisable()
        {
            NetworkTime.EnemyTimeRateChanged -= _networkAnimator.RPC_SetAnimatorSpeed;
        }

        private void Update()
        {
            if (_shootTrigger && _reloadTrigger == false)
            {
                PlayShootAnimation();
            }

            Vector3 velocity = _aiPath.velocity;
            if (MathHelper.IsNearZeroVector(velocity))
            {
                SetBool(EnemyAnimationState.Idle);
            }
            else
            {
                if (MathHelper.FrontDiscrimination(transform.forward, _aiPath.velocity))
                {
                    SetBool(EnemyAnimationState.Chase);
                }
                else
                {
                    SetBool(EnemyAnimationState.Retreat);
                }
            }
        }

        public override void SetTrigger(EnemyAnimationTrigger animation, Action endCallback = null)
        {
            switch (animation)
            {
                case EnemyAnimationTrigger.Shoot:
                    _shootTrigger = true;
                    break;

                case EnemyAnimationTrigger.Reload:
                    _networkAnimator.SetTrigger(ANIMATION_NAME_RELOAD);

                    _reloadTrigger = true;
                    StartCoroutine(CheckEndOfReloading(endCallback));
                    break;

                default:
                    Debug.LogError("There is no such element in EnemyAnimationTrigger(enum).");
                    break;
            }
        }

        protected override void SetBool(EnemyAnimationState animation)
        {
            switch (animation)
            {
                case EnemyAnimationState.Idle:
                    _networkAnimator.SetBool(ANIMATION_NAME_CHASE, false);
                    _networkAnimator.SetBool(ANIMATION_NAME_RETREAT, false);
                    break;

                case EnemyAnimationState.Chase:
                    _networkAnimator.SetBool(ANIMATION_NAME_CHASE, true);
                    _networkAnimator.SetBool(ANIMATION_NAME_RETREAT, false);
                    break;

                case EnemyAnimationState.Retreat:
                    _networkAnimator.SetBool(ANIMATION_NAME_RETREAT, true);
                    _networkAnimator.SetBool(ANIMATION_NAME_CHASE, false);
                    break;

                default:
                    Debug.LogError("There is no such element in EnemyAnimationState(enum).");
                    break;
            }
        }

        private void PlayShootAnimation()
        {
            _networkAnimator.SetTrigger(ANIMATION_NAME_SHOOT);
            _shootTrigger = false;
        }

        private IEnumerator CheckEndOfReloading(Action EndOfReloading)
        {
            yield return _reloadWaitForSeconds;

            _reloadTrigger = false;
            EndOfReloading();
        }
    }
}