using System.Collections.Generic;
using UnityEngine;
using ProbyEditor;
using Fusion;

namespace ModelStudents
{
    [Author("Yoon")]
    public class EnemyManager : NetworkBehaviour
    {
        private static EnemyManager _instance;

        private static readonly List<AAIBehaviour> NEARBY_ENEMIES_BUFFER = new List<AAIBehaviour>();
        private List<AAIBehaviour> _enemies;

        private void Awake()
        {
            _instance = this;

            _enemies = new List<AAIBehaviour>();
        }

        private void OnDestroy()
        {
            _instance = null;
        }

        public static void SpawnEnemy(ENetworkPrefab enemyType, Vector3 position, Quaternion rotation)
        {
            if (_instance == null)
            {
                Debug.LogWarning("No EnemyManager instace error. EnemyManager is singleton class but instance is not yet initialized");
            }

            NetworkPrefabRef prefab = NetworkPrefab.GetPrefab(enemyType);

            if (prefab != null)
            {
                NetworkObject enemy = _instance.Runner.Spawn(prefab, position, rotation, _instance.Runner.LocalPlayer);
                _instance._enemies.Add(enemy.GetComponent<AAIBehaviour>());
            }
        }

        public static void DespawnEnemy(NetworkObject enemy, AAIBehaviour aiBehaviour)
        {
            if (_instance == null)
            {
                Debug.LogWarning("No EnemyManager instace error. EnemyManager is singleton class but instance is not yet initialized");
            }

            _instance._enemies.Remove(aiBehaviour);
            _instance.Runner.Despawn(enemy);
        }

        public static List<AAIBehaviour> FindNearbyEnemies(Vector3 basePosition, float range)
        {
            if (_instance == null)
            {
                Debug.LogWarning("No EnemyManager instace error. EnemyManager is singleton class but instance is not yet initialized");
            }

            NEARBY_ENEMIES_BUFFER.Clear();
            foreach (var enemy in _instance._enemies)
            {
                if (Vector3.Distance(basePosition, enemy.transform.position) <= range)
                {
                    NEARBY_ENEMIES_BUFFER.Add(enemy);
                }
            }

            return NEARBY_ENEMIES_BUFFER;
        }
    }
}
