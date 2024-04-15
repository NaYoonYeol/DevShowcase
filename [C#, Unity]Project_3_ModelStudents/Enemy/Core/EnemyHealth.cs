using Fusion;
using System;
using ProbyEditor;
using UnityEngine;

namespace ModelStudents
{
    [Author("Yoon")]
    [RequireComponent(typeof(HitboxRoot))]
    public class EnemyHealth : ANetworkHealth
    {
        public event Action OnDamaged;
        public event Action OnDead;

        [Header("VFX Settings")]
        [SerializeField]
        private float _hitVfxHeight = 1.75f;

        [SerializeField]
        private float _hitVfxRadius = 0.875f;

        protected override float GetDamage(PlayerRef damagedPlayer, SDamageInfo damageInfo)
        {
            OnDamaged?.Invoke();
            PlayHitTypeVFX(damageInfo);
            DamageIndicator.RPC_Display(transform.position, damageInfo.Damage, damageInfo.HitCount);

            return damageInfo.Damage * damageInfo.HitCount;
        }

        protected override void OnHealthExhausted(NetworkId attackerID)
        {
            OnDead?.Invoke();
        }

        private void PlayHitTypeVFX(SDamageInfo damageInfo)
        {
            switch (damageInfo.HitType)
            {
                case (sbyte)EHitType.Slash:
                    RPC_PlayVFX(damageInfo.HitEffect, damageInfo.HitPoint, damageInfo.HitCount);
                    break;

                default:
                    break;
            }
        }

        [Rpc(RpcSources.StateAuthority, RpcTargets.All)]
        private void RPC_PlayVFX(sbyte vfx, Vector3 position, sbyte hitCount)
        {
            for (int i = 0; i < hitCount; i++)
            {
                Vector3 hitPos = position + (Vector3.up * _hitVfxHeight);
                hitPos += UnityEngine.Random.onUnitSphere * _hitVfxRadius;

                VFXManager.Play((EVFX)vfx, hitPos, Quaternion.identity);
            }
        }
    }
}