using Fusion;
using ProbyEditor;
using System;
using UnityEngine;

namespace ModelStudents
{
    [Author("Yoon")]
    public class SuicideState : AExplosiveState
    {
        private MechaBossAnimationController _animationController;

        private bool _shouldBreak;

        public void InitState(Transform owner, MechaBossAnimationController animationController, NetworkId networkId,
            SuicideStateData suicideStateData, Action endOfState)
        {
            InitState(owner, networkId, suicideStateData, endOfState);

            _animationController = animationController;

            _shouldBreak = suicideStateData.ShouldBreak;
        }

        protected override bool ReadyToExplode()
        {
            _animationController.SetTrigger(EnemyAnimationTrigger.Dead);

            if (_shouldBreak)
            {
                // TODO: 죽었을 때 어떻게 부술지 기획이 나온 후 진행
                // 임시로 부수는 코드
                var physics = _ownerTransform.GetComponentsInChildren<Rigidbody>();
                foreach (var rigidbody in physics)
                {
                    rigidbody.useGravity = true;
                    rigidbody.isKinematic = false;
                }
            }

            return true;
        }
    }

    [Author("Yoon")]
    [System.Serializable]
    public class SuicideStateData : ExplosiveStateData
    {
        public bool ShouldBreak;
    }
}