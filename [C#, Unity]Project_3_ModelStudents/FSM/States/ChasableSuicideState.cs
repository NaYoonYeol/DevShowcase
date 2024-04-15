using Fusion;
using Pathfinding;
using ProbyEditor;
using System;
using System.Collections.Generic;
using UnityEngine;
using Random = UnityEngine.Random;

namespace ModelStudents
{
    [Author("Yoon")]
    public class ChasableSuicideState : SuicideState
    {
        private float _maxChaseTime;
        private float _chasingTime;

        private float _chasingSpeed;

        private IAstarAI _aiPath;

        private Transform _targetTransform;

        public void InitState(Transform owner, IAstarAI aiPath, MechaBossAnimationController animationController, NetworkId networkId,
            ChasableSuicideStateData chasableSuicideStateData, Action endOfState)
        {
            _maxChaseTime = chasableSuicideStateData.ChaseTime;
            _chasingTime = 0;

            _chasingSpeed = chasableSuicideStateData.ChaseSpeed;

            _aiPath = aiPath;
            _aiPath.isStopped = false;
            _aiPath.maxSpeed = _chasingSpeed * NetworkTime.EnemyTimeRate;

            List<PlayerController> players = PlayerManager.GetPlayerControllers();
            int targetedPlayerIndex = Random.Range(0, players.Count);

            _targetTransform = players[targetedPlayerIndex].GetComponent<Transform>();

            NetworkTime.EnemyTimeRateChanged += OnEnemyTimeRateChanged;

            InitState(owner, animationController, networkId, chasableSuicideStateData, endOfState);
        }

        protected override bool ReadyToExplode()
        {
            _chasingTime += Time.deltaTime * NetworkTime.EnemyTimeRate;
            if (_chasingTime >= _maxChaseTime)
            {
                _aiPath.isStopped = true;

                NetworkTime.EnemyTimeRateChanged -= OnEnemyTimeRateChanged;

                return base.ReadyToExplode();
            }

            _aiPath.destination = _targetTransform.position;
            _aiPath.SearchPath();

            return false;
        }

        private void OnEnemyTimeRateChanged(float timeRage)
        {
            _aiPath.maxSpeed = _chasingSpeed * NetworkTime.EnemyTimeRate;
        }
    }

    [Author("Yoon")]
    public class ChasableSuicideStateData : SuicideStateData
    {
        public float ChaseTime;
        public float ChaseSpeed;
    }
}