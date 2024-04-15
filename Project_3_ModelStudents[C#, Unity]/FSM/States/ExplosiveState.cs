using Fusion;
using ProbyEditor;
using System;
using UnityEngine;

namespace ModelStudents
{
    [Author("Yoon")]
    public class ExplosiveState : AExplosiveState
    {
        private float _detectingRadius;

        public void InitState(Transform ownerTransform, NetworkId networkId,
            DetectingExplosiveStateData explosiveStateData, Action endOfState)
        {
            base.InitState(ownerTransform, networkId, explosiveStateData, endOfState);

            _detectingRadius = explosiveStateData.DetectingRadius;
        }

        protected override bool ReadyToExplode()
        {
            for (int i = 0; i < _playerCount; ++i)
            {
                float dist = Vector3.Distance(_players[i].position, _ownerTransform.position);
                if (dist < _detectingRadius)
                {
                    return true;
                }
            }

            return false;
        }
    }

    [Author("Yoon")]
    [System.Serializable]
    public class DetectingExplosiveStateData : ExplosiveStateData
    {
        public float DetectingRadius;
    }
}