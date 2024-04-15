using DG.Tweening;
using Fusion;
using ProbyEditor;
using System;
using System.Collections.Generic;
using UnityEngine;
using Random = UnityEngine.Random;

namespace ModelStudents
{
    [Author("Yoon")]
    public class AttackableJumpState : ARandomAttackState
    {
        private List<Transform> _players = new List<Transform>();

        private Transform _owner;
        private float _jumpDuration;

        private float _fallDamage;
        private float _fallDamageRange;

        private NetworkId _networkId;

        private Action _endOfState;

        public void InitState(Transform owner, NetworkId networkId, NetworkRunner runner, AttackableJumpStateData attackableJumpStateData, Action endOfState)
        {
            base.InitState(runner, attackableJumpStateData);
            
            _owner = owner;

            _networkId = networkId;

            _jumpDuration = attackableJumpStateData.JumpDuration;

            _fallDamage = attackableJumpStateData.FallDamage;
            _fallDamageRange = attackableJumpStateData.FallDamageRange;

            _endOfState = endOfState;
        }

        public override void ExecuteBeginState()
        {
            base.ExecuteBeginState();

            foreach (var player in PlayerManager.GetPlayerControllers())
            {
                _players.Add(player.transform);
            }

            int index = Random.Range(0, _players.Count);
            _owner.DOJump(_players[index].position, 10, 1, _jumpDuration).OnComplete(() => OnJumpCompleted());
        }

        protected override bool DetectTarget()
        {
            return true;
        }

        protected override void Attack()
        {
            if (_currentCirculationCount >= _maxCirculationCount)
            {
                _currentCirculationCount = 0;

                return;
            }

            int index = Random.Range(0, _players.Count);
            _weapon.Fire(_runner, _players[index].position);
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
    public class AttackableJumpStateData : RandomAttackStateData
    {
        public float JumpDuration;
        public float FallDamage;
        public float FallDamageRange;
    }
}