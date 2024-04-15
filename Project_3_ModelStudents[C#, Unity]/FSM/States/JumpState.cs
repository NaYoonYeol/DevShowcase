using System;
using System.Collections.Generic;
using UnityEngine;
using DG.Tweening;
using Random = UnityEngine.Random;
using ProbyEditor;
using Fusion;

namespace ModelStudents
{
    [Author("Yoon")]
    public class JumpState : AStateContainer
    {
        private List<Transform> _players = new List<Transform>();

        private Transform _owner;
        private float _jumpDuration;

        private float _fallDamage;
        private float _fallDamageRange;

        private NetworkId _networkId;

        private Action _endOfState;

        public void InitState(Transform owner, NetworkId networkId, JumpStateData jumpStateData, Action endOfState)
        {
            _owner = owner;

            _networkId = networkId;

            _jumpDuration = jumpStateData.JumpDuration;

            _fallDamage = jumpStateData.FallDamage;
            _fallDamageRange = jumpStateData.FallDamageRange;

            _endOfState = endOfState;
        }

        public override void ExecuteBeginState()
        {
            foreach (var player in PlayerManager.GetPlayerControllers())
            {
                _players.Add(player.transform);
            }

            int index = Random.Range(0, _players.Count);
            _owner.DOJump(_players[index].position, 10, 1, _jumpDuration).OnComplete(() => OnJumpCompleted());
        }

        public override void ExecuteState()
        {
        }

        public override void ExecuteEndState()
        {
        }

        private void OnJumpCompleted()
        {
            foreach (var player in _players)
            {
                if (Vector3.Distance(player.position, _owner.position) <= _fallDamageRange)
                {
                    INetworkDamageable target = CacheTable.GetNetworkDamageable(player.gameObject);
                    if (target != null)
                    {
                        SDamageInfo damageInfo = new SDamageInfo
                        {
                            Damage = _fallDamage,
                            HitPoint = player.position,
                            HitType = (sbyte)EHitType.Magnetic,
                            HitCount = 1
                        };

                        PlayerRef damagedPlayer = player.GetComponent<NetworkObject>().InputAuthority;

                        target.RPC_Damaged(damagedPlayer, _networkId, damageInfo);
                    }
                }
            }

            _endOfState?.Invoke();
        }
    }

    [Author("Yoon")]
    [System.Serializable]
    public class JumpStateData : IStateData
    {
        public float JumpDuration;
        public float FallDamage;
        public float FallDamageRange;
    }
}
