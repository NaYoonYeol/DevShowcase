using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;
using ProbyEditor;
using Fusion;

namespace ModelStudents
{
    [Author("Yoon")]
    [System.Serializable]
    public class RifleMountGrenadeLauncher : AEnemyWeapon
    {
        [SerializeField]
        private Weapon _rifle;
        [SerializeField]
        private Weapon _grenadeLauncher;

        [SerializeField]
        private float _grenadeLauncherIntervalMin;
        [SerializeField]
        private float _grenadeLauncherIntervalMax;

        private float _grenadeLauncherInterval;
        private float _grenadeLauncherTime;

        public override void AppendTime(float deltaTime)
        {
            _grenadeLauncherTime += deltaTime;
        }

        public override void Fire(NetworkRunner runner, Vector3 targetPosition)
        {
            if (_grenadeLauncherTime < _grenadeLauncherInterval)
            {
                _rifle.Fire(runner, runner.LocalPlayer, targetPosition);

                return;
            }

            _grenadeLauncher.Fire(runner, runner.LocalPlayer, targetPosition);
            _grenadeLauncherInterval = UnityEngine.Random.Range(_grenadeLauncherIntervalMin, _grenadeLauncherIntervalMax);
            _grenadeLauncherTime = 0;
        }

        public override bool IsEmpty()
        {
            return _rifle.IsEmpty;
        }

        public override void Reload()
        {
            _rifle.Reload();
        }
    }
}
