using Fusion;
using ProbyEditor;
using System;
using System.Collections.Generic;
using Random = UnityEngine.Random;

namespace ModelStudents
{
    [Author("Yoon")]
    public class RandomTargetAttackState : ARandomAttackState
    {
        private List<PlayerController> _players;

        private Action _endOfState;

        public void InitState(NetworkRunner runner, RandomAttackStateData attackStateData, Action endOfState)
        {
            base.InitState(runner, attackStateData);

            _endOfState = endOfState;
        }

        protected override bool DetectTarget()
        {
            _players = PlayerManager.GetPlayerControllers();

            return true;
        }

        protected override void Attack()
        {
            if (_currentCirculationCount >= _maxCirculationCount)
            {
                _endOfState?.Invoke();

                return;
            }

            int index = Random.Range(0, _players.Count);
            _weapon.Fire(_runner, _players[index].transform.position);
        }
    }
}