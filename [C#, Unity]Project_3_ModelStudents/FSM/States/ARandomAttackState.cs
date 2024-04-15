using Fusion;
using ProbyEditor;

namespace ModelStudents
{
    [Author("Yoon")]
    public abstract class ARandomAttackState : AAttackState
    {
        protected NetworkRunner _runner;

        protected AEnemyWeapon _weapon;

        protected int _maxCirculationCount;
        protected int _currentCirculationCount;

        private float _circulationInterval;

        public void InitState(NetworkRunner runner, RandomAttackStateData attackStateData)
        {
            _runner = runner;
            _weapon = attackStateData.Weapon.GetComponent<AEnemyWeapon>();

            _maxCirculationCount = attackStateData.CirculationCount;
            _currentCirculationCount = 0;

            SetAttackInterval(attackStateData.AttackInterval);
            _circulationInterval = attackStateData.CirculationInterval;
        }

        public override void ExecuteBeginState()
        {
            _currentCirculationCount = 0;

            base.ExecuteBeginState();
        }

        protected override bool Reload()
        {
            if (_weapon.IsEmpty())
            {
                if (_attackDelay > _circulationInterval)
                {
                    _weapon.Reload();
                    ++_currentCirculationCount;
                }

                return true;
            }

            return false;
        }
    }

    [Author("Yoon")]
    [System.Serializable]
    public class RandomAttackStateData : IStateData
    {
        public float AttackInterval;
        public float CirculationInterval;
        public int CirculationCount;
        public AEnemyWeapon Weapon;
    }
}
