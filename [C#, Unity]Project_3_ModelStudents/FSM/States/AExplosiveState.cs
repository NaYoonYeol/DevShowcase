using Fusion;
using ProbyEditor;
using System;
using System.Collections.Generic;
using UnityEngine;

namespace ModelStudents
{
    [Author("Yoon")]
    public abstract class AExplosiveState : AStateContainer
    {
        protected List<Transform> _players = new List<Transform>();
        protected int _playerCount;

        protected Transform _ownerTransform;

        // Once damaged, do not damage the target again.
        private List<Transform> _damagedPlayers = new List<Transform>();
        
        private NetworkId _networkId;
        private EVFX _vfx;

        private float _chargingWaitTime;
        private float _chargingTime;

        private AnimationCurve _radiusCurve;
        private float _emissionTime;

        private float _maxRadius;
        private float _duration;

        private float _damage;

        private int _maxRepetition;
        private int _currentRepetition;

        private bool _isEmissionOnce;
        private bool _isReadyToExplode;

        private event Action _endOfState;

        public void InitState(Transform ownerTransform, NetworkId networkId,
            ExplosiveStateData explosiveStateData, Action endOfState)
        {
            List<PlayerController> controllers = PlayerManager.GetPlayerControllers();
            _playerCount = controllers.Count;
            for (int i = 0; i < _playerCount; ++i)
            {
                _players.Add(controllers[i].transform);
            }

            _ownerTransform = ownerTransform;
            _networkId = networkId;
            _vfx = explosiveStateData.EmissionVFX;

            _chargingWaitTime = explosiveStateData.ChargingTime;
            _chargingTime = 0;

            _radiusCurve = explosiveStateData.RadiusSizeCurve;
            _emissionTime = 0;

            _maxRadius = explosiveStateData.MaxRadius;
            _duration = explosiveStateData.Duration;

            _damage = explosiveStateData.Damage;

            _maxRepetition = explosiveStateData.Repetition;
            _currentRepetition = 0;

            _endOfState = endOfState;
        }

        public override void ExecuteBeginState()
        {
            _isEmissionOnce = true;
            _isReadyToExplode = false;
        }

        public override void ExecuteState()
        {
            _chargingTime += Time.deltaTime * NetworkTime.EnemyTimeRate;
            if (_chargingTime < _chargingWaitTime)
            {
                // TODO : 에너지 모으는 이펙트 들어가야 함.
                return;
            }

            if (_isReadyToExplode == false)
            {
                _isReadyToExplode = ReadyToExplode();

                return;
            }

            if (_isReadyToExplode && _isEmissionOnce)
            {
                VFXManager.RPC_Play(_vfx, _ownerTransform.position + Vector3.up, Quaternion.identity);
                _isEmissionOnce = false;
            }

            _emissionTime += Time.deltaTime * NetworkTime.EnemyTimeRate;
            float currentRadius = _maxRadius * _radiusCurve.Evaluate(_emissionTime);

            DamageWithinRange(currentRadius, EHitType.Magnetic, true);

            if (_emissionTime > _duration)
            {
                if (++_currentRepetition >= _maxRepetition)
                {
                    _endOfState?.Invoke();

                    return;
                }

                _isEmissionOnce = true;
                _isReadyToExplode = false;

                _chargingTime = 0;
                _emissionTime = 0;
                _damagedPlayers.Clear();
            }
        }

        public override void ExecuteEndState()
        {
            _chargingTime = 0;
            _emissionTime = 0;
            _currentRepetition = 0;

            _damagedPlayers.Clear();
        }

        protected abstract bool ReadyToExplode();

        private void DamageWithinRange(float currentRadius, EHitType hitType, bool isHitOnce)
        {
            for (int i = 0; i < _playerCount; ++i)
            {
                if (_damagedPlayers.IndexOf(_players[i]) >= 0)
                {
                    continue;
                }

                float dist = Vector3.Distance(_ownerTransform.position, _players[i].position);
                if (dist < currentRadius)
                {
                    INetworkDamageable target = CacheTable.GetNetworkDamageable(_players[i].gameObject);
                    if (target != null)
                    {
                        SDamageInfo damageInfo = new SDamageInfo
                        {
                            Damage = _damage,
                            HitPoint = _players[i].position,
                            HitType = (sbyte)EHitType.Magnetic,
                            HitCount = 1
                        };

                        PlayerRef damagedPlayer = _players[i].GetComponent<NetworkObject>().InputAuthority;

                        target.RPC_Damaged(damagedPlayer, _networkId, damageInfo);

                        if (isHitOnce)
                        {
                            _damagedPlayers.Add(_players[i]);
                        }
                    }
                }
            }
        }
    }

    [Author("Yoon")]
    [System.Serializable]
    public class ExplosiveStateData : IStateData
    {
        public EVFX EmissionVFX;
        public float ChargingTime;
        public AnimationCurve RadiusSizeCurve;
        public float MaxRadius;
        public float Duration;
        public float Damage;
        public int Repetition;

        // Not yet implemented.
        public float SlowdownAmount;
        public float SlowdownDuration;
    }
}