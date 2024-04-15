using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using DG.Tweening;
using ProbyEditor;
using Fusion;

namespace ModelStudents
{
    [Author("Yoon")]
    public class RangeIndicator : IndicatorManager.AIndicatorBehaviour
    {
        private static int GROUND_LAYER;

        [SerializeField]
        private float _radiusIncreaseDuration;
        [SerializeField]
        private float _radiusDescreaseDuration;

        private Vector3 _indicatorScale;
        private float _maxRadius;

        private void Awake()
        {
            GROUND_LAYER = 1 << LayerMask.NameToLayer("Ground");
        }

        public override void InitializeIndicator(in Vector3 targetposition, float explosionRadius)
        {
            _maxRadius = explosionRadius;

            FindPlacablePosition(targetposition);

            DOTween.To(ToGetter, ToSetter, _maxRadius, _radiusIncreaseDuration);

            float ToGetter()
            {
                return 0;
            }

            void ToSetter(float value)
            {
                _indicatorScale.x = value;
                _indicatorScale.y = value;
                transform.localScale = _indicatorScale;
            }
        }

        public override void DecreaseIndicator()
        {
            DOTween.To(ToGetter, ToSetter, 0f, _radiusDescreaseDuration).OnComplete(ToEnd);

            float ToGetter()
            {
                return _maxRadius;
            }

            void ToSetter(float value)
            {
                _indicatorScale.x = value;
                _indicatorScale.y = value;
                transform.localScale = _indicatorScale;
            }

            void ToEnd()
            {
                DestroyIndicator();
            }
        }

        private void FindPlacablePosition(Vector3 targetPosition)
        {
            Vector3 rayPosition = transform.position;
            rayPosition.y += 1;

            if (Physics.Raycast(rayPosition, Vector3.down, out RaycastHit hit, 5.0f, GROUND_LAYER))
            {
                transform.rotation = Quaternion.LookRotation(hit.normal);

                targetPosition.y += 0.01f;
                transform.position = targetPosition;
            }
        }
    }
}