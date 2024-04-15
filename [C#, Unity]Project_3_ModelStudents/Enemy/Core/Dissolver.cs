using UnityEngine;
using DG.Tweening;
using System;

namespace ModelStudents
{
    public class Dissolver : MonoBehaviour
    {
        private static readonly int CUTOUT_PROPERTY_ID = Shader.PropertyToID("_Cutout");
        private static readonly int OUTLINE_COLOR_PROPERTY_ID = Shader.PropertyToID("_OutlineColor");

        [SerializeField]
        private float _dissolveDuration;

        private Renderer[] _renderers = null;
        private MaterialPropertyBlock _propertyBlock = null;

        private void Awake()
        {
            _renderers = GetComponentsInChildren<Renderer>();
            _propertyBlock = new MaterialPropertyBlock();
        }

        public void Dissolve(Action onDissolveCompleted)
        {
            DOTween.To(ToGetter, ToSetter, 1f, _dissolveDuration).OnComplete(OnComplete);

            float ToGetter()
            {
                return 0;
            }

            void ToSetter(float value)
            {
                ExecuteDissolveShader(value, Mathf.Min(1f, value * 2) * Color.red);
            }

            void OnComplete()
            {
                onDissolveCompleted();
            }
        }

        public void ResetShader()
        {
            ExecuteDissolveShader(0, Color.black);
        }

        private void ExecuteDissolveShader(float amount, Color outlineColor)
        {
            _propertyBlock.SetFloat(CUTOUT_PROPERTY_ID, amount);
            _propertyBlock.SetColor(OUTLINE_COLOR_PROPERTY_ID, outlineColor);

            foreach (var renderer in _renderers)
            {
                renderer.SetPropertyBlock(_propertyBlock);
            }
        }
    }
}