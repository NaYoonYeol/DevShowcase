using System.Collections;
using Fusion;
using UnityEngine;
using ProbyEditor;

namespace ModelStudents
{
    [Author("Yoon")]
    [System.Serializable]
    public class MultipleGrenadeLauncher : AEnemyWeapon
    {
        [SerializeField]
        private Weapon _grenadeLauncher;

        [MessageBox("Fire in the order provided.")]
        [SerializeField]
        private Transform[] _launcherPoint;
        private int _firePointIndex;

        public override void AppendTime(float deltaTime)
        {
        }

        public override void Fire(NetworkRunner runner, Vector3 targetPosition)
        {
            _grenadeLauncher.FirePoint = _launcherPoint[_firePointIndex++];
            _firePointIndex %= _launcherPoint.Length;

            _grenadeLauncher.Fire(runner, runner.LocalPlayer, targetPosition);
        }

        public override bool IsEmpty()
        {
            return _grenadeLauncher.IsEmpty;
        }

        public override void Reload()
        {
            _grenadeLauncher.Reload();
        }
    }
}
