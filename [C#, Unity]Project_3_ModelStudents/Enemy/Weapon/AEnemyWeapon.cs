using UnityEngine;
using ProbyEditor;
using Fusion;

namespace ModelStudents
{
    [Author("Yoon")]
    public abstract class AEnemyWeapon : MonoBehaviour
    {
        public abstract void Fire(NetworkRunner runner, Vector3 targetPosition);

        public abstract bool IsEmpty();
        public abstract void Reload();

        public abstract void AppendTime(float deltaTime);
    }
}
