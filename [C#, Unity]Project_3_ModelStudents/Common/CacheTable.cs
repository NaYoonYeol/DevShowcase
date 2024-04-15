using System.Collections.Generic;
using UnityEngine;
using Fusion;
using ProbyEditor;

namespace ModelStudents
{
    [Author("Common")]
    public static class CacheTable
    {
        private static readonly Dictionary<GameObject, INetworkDamageable> TABLE_NETWORK_DAMAGEABLE = null;
        private static readonly Dictionary<NetworkObject, AProjectile> TABLE_PROJECTILE = null;
        private static readonly Dictionary<NetworkId, NetworkObject> TABLE_NETWORK_OBJECT = null;

        static CacheTable()
        {
            TABLE_NETWORK_DAMAGEABLE = new Dictionary<GameObject, INetworkDamageable>();
            TABLE_PROJECTILE = new Dictionary<NetworkObject, AProjectile>();
            TABLE_NETWORK_OBJECT = new Dictionary<NetworkId, NetworkObject>();
        }

        public static INetworkDamageable GetNetworkDamageable(GameObject key)
        {
            if (TABLE_NETWORK_DAMAGEABLE.TryGetValue(key, out INetworkDamageable value) == false)
            {
                value = key.GetComponentInParent<INetworkDamageable>();
                TABLE_NETWORK_DAMAGEABLE.Add(key, value);
            }

            return value;
        }

        public static AProjectile GetProjectile(NetworkObject key)
        {
            if (TABLE_PROJECTILE.TryGetValue(key, out AProjectile value) == false)
            {
                value = key.GetComponentInParent<AProjectile>();
                TABLE_PROJECTILE.Add(key, value);
            }

            return value;
        }

        public static NetworkObject GetNetworkObject(NetworkId key)
        {
            if (TABLE_NETWORK_OBJECT.TryGetValue(key, out NetworkObject value) == false)
            {
                value = UserNetworkRunner.FindObject(key);
                TABLE_NETWORK_OBJECT.Add(key, value);
            }

            return value;
        }

        public static void Clear()
        {
            TABLE_NETWORK_DAMAGEABLE.Clear();
            TABLE_PROJECTILE.Clear();
            TABLE_NETWORK_OBJECT.Clear();
        }
    }
}