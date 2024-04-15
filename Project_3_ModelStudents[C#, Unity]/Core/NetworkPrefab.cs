using UnityEngine;
using ProbyEditor;
using Fusion;

namespace ModelStudents
{
    [Author("Yoon")]
    public class NetworkPrefab : MonoBehaviour
    {
        [MessageBox("ENetworkPrefab을 먼저 수정하고 인스펙터에서 NetworkObject를 넣으세요 ^^")]
        [SerializeField]
        [GeneralEnumList(typeof(ENetworkPrefab), 120f)]
        private NetworkPrefabRef[] _networkPrefabRefs;

        private static NetworkPrefab _instance;

        private void Awake()
        {
            _instance = this;
        }

        private void OnDestroy()
        {
            _instance = null;
        }

        public static NetworkPrefabRef GetPrefab(ENetworkPrefab prefabType)
        {
            if (_instance == null)
            {
                Debug.LogWarning("No NetworkPrefab instace error. NetworkPrefab is singleton class but instance is not yet initialized." +
                    "Prefab you were looking for: " + prefabType.ToString());
            }

            return _instance._networkPrefabRefs[(int)prefabType];
        }
    }
}