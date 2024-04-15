using Fusion;
using ProbyEditor;
using System;
using UnityEngine;

namespace ModelStudents
{
    [Author("Yoon")]
    public class MultipleSpiralState : AStateContainer
    {
        private Action _endOfState;

        private NetworkRunner _runner;
        private Transform _transform;
        private Weapon _multipleSpiralLauncher;

        private float _angularVelocity;
        private float _angle;

        private float _interval;
        private float _delay;

        private int _shootAngle;
        private int _bulletCount;

        private int _maxShootCount;
        private int _shootCount;

        private int _sfxClip;

        public void InitState(NetworkRunner runner, Transform _ownerTransform, MultipleSprialStateData stateData, Action endOfState)
        {
            _interval = stateData.Interval;
            _delay = 0;

            _angularVelocity = stateData.AngularVelocity;
            _angle = 0;

            _shootAngle = stateData.ShootAngle;
            _bulletCount = 360 / _shootAngle;

            _runner = runner;
            _transform = _ownerTransform;
            _multipleSpiralLauncher = stateData.MultipleSpiralLauncher;

            _maxShootCount = stateData.ShootCount;
            _shootCount = 0;

            _endOfState = endOfState;

            _sfxClip = stateData.SfxClip;
        }

        public override void ExecuteBeginState()
        {
            _delay = 0;
            _angle = 0;
            _shootCount = 0;
        }

        public override void ExecuteState()
        {
            _delay += Time.deltaTime * NetworkTime.EnemyTimeRate;
            if (_delay < _interval)
            {
                return;
            }

            if (_shootCount >= _maxShootCount)
            {
                _endOfState?.Invoke();

                return;
            }

            _angle += _angularVelocity;
            for (int i = 0; i < _bulletCount; ++i)
            {
                float theta = _shootAngle * i + _angle;
                Vector3 direction = new Vector3(Mathf.Cos(theta * Mathf.Deg2Rad), 0, Mathf.Sin(theta * Mathf.Deg2Rad));

                _multipleSpiralLauncher.Fire(_runner, _runner.LocalPlayer, _transform.position + direction * 100);
            }

            AudioManager.RPC_PlaySFX(_runner, _sfxClip, _transform.position);

            _delay = 0;
            ++_shootCount;
        }

        public override void ExecuteEndState()
        {
        }
    }

    [Author("Yoon")]
    [System.Serializable]
    public class MultipleSprialStateData : IStateData
    {
        public Weapon MultipleSpiralLauncher;
        public float Interval;
        public int ShootAngle;
        public int AngularVelocity;
        public int ShootCount;
        public int SfxClip;
    }
}

