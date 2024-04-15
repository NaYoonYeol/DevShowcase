using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using ProbyEditor;
using System;
using Pathfinding;

namespace ModelStudents
{
    [RequireComponent(typeof(AAIBehaviour))]
    [Author("Yoon")]
    public class MechaBossAnimationController : AEnemyAnimationController
    {
        private const string ANIMATION_NAME_DEAD = "dead";

        public override void SetTrigger(EnemyAnimationTrigger animation, Action endCallback = null)
        {
            switch (animation)
            {
                case EnemyAnimationTrigger.Dead:
                    _networkAnimator.SetTrigger(ANIMATION_NAME_DEAD);
                    break;

                default:
                    Debug.LogError("There is no such element in EnemyAnimationTrigger(enum).");
                    break;
            }
        }

        protected override void SetBool(EnemyAnimationState animation)
        {
        }
    }
}