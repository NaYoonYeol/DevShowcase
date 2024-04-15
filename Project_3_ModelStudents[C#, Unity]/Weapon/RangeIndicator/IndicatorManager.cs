using System.Collections.Generic;
using UnityEngine;
using ProbyEditor;
using System;
using Fusion;

namespace ModelStudents
{
    [Author("Yoon")]
    public class IndicatorManager : NetworkBehaviour
    {
        public abstract class AIndicatorBehaviour : MonoBehaviour
        {
            public abstract void InitializeIndicator(in Vector3 targetposition, float explosionRadius);
            public abstract void DecreaseIndicator();

            protected void DestroyIndicator()
            {
                DestroyRangeIndicator(this);
            }
        }

        private static readonly Stack<AIndicatorBehaviour> RANGE_INDICATORS = new Stack<AIndicatorBehaviour>();
        private static readonly List<KeyValuePair<uint, AIndicatorBehaviour>> ALIVE_RANGE_INDICATORS = new List<KeyValuePair<uint, AIndicatorBehaviour>>();

        [SerializeField]
        private RangeIndicator _rangeIndicatorPrefab;

        private static IndicatorManager _instance;
        private static uint _rangeIndicatorId = 0;

        private void Awake()
        {
            _instance = this;
        }

        private void OnDestroy()
        {
            RANGE_INDICATORS.Clear();
            ALIVE_RANGE_INDICATORS.Clear();

            _instance = null;
            _rangeIndicatorId = 0;
        }

        public static uint CreateRangeIndicatorId()
        {
            return _rangeIndicatorId++;
        }

        public static void CreateRangeIndicator(uint indicatorId, in Vector3 targetPosition, SShortFloat explosionRadius)
        {
            _instance.RPC_CreateRangeIndicator(indicatorId, targetPosition, explosionRadius);
        }

        public static void DecreaseRangeIndicator(uint indicatorId)
        {
            _instance.RPC_DecreaseRangeIndicator(indicatorId);
        }

        private static void DestroyRangeIndicator(AIndicatorBehaviour indicator)
        {
            indicator.gameObject.SetActive(false);
            RANGE_INDICATORS.Push(indicator);
        }

        [Rpc(RpcSources.StateAuthority, RpcTargets.All)]
        private void RPC_CreateRangeIndicator(uint indicatorId, Vector3 targetPosition, SShortFloat explosionRadius)
        {
            RangeIndicator rangeIndicator;
            if (RANGE_INDICATORS.Count == 0)
            {
                rangeIndicator = Instantiate(_instance._rangeIndicatorPrefab, Vector3.zero, Quaternion.Euler(90, 0, 0));
            }
            else
            {
                rangeIndicator = (RangeIndicator)RANGE_INDICATORS.Pop();
            }
            rangeIndicator.gameObject.SetActive(true);
            rangeIndicator.InitializeIndicator(in targetPosition, explosionRadius);

            ALIVE_RANGE_INDICATORS.Add(new KeyValuePair<uint, AIndicatorBehaviour>(indicatorId, rangeIndicator));
        }

        [Rpc(RpcSources.StateAuthority, RpcTargets.All)]
        private void RPC_DecreaseRangeIndicator(uint indicatorId)
        {
            int count = ALIVE_RANGE_INDICATORS.Count;
            for (int i = 0; i < count; ++i)
            {
                if (ALIVE_RANGE_INDICATORS[i].Key == indicatorId)
                {
                    RangeIndicator indicator = (RangeIndicator)ALIVE_RANGE_INDICATORS[i].Value;
                    indicator.DecreaseIndicator();

                    ALIVE_RANGE_INDICATORS.RemoveAt(i);

                    return;
                }
            }
        }
    }
}
