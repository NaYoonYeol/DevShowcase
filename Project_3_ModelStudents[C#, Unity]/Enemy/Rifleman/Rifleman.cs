using UnityEngine;
using Pathfinding;
using ProbyEditor;
using Fusion;
using Pathfinding.RVO;

namespace ModelStudents
{
    [Author("Yoon")]
    public class Rifleman : AAIBehaviour, IDetectable, IGroupChaseable, IGroupLeader
    {
        [SerializeField]
        private DetectionStateData _detectionStateData;
        [SerializeField]
        private AttackStateData _attackStateData;

        [SerializeField]
        private GroupChaseStateData _groupChaseStateData;
        [SerializeField]
        private RetreatStateData _retreatStateData;

        [SerializeField]
        private RifleMountGrenadeLauncher _rifleMountGrenadeLauncher;

        [Header("Dissolve Settings")]
        [SerializeField]
        private Dissolver _enemyDissolver;

        public SoldierAttackState AttackState { get; private set; }
        public DetectionState DetectionState { get; private set; }
        public GroupLeaderState GroupLeaderState { get; private set; }
        public GroupChaseState GroupChaseState { get; private set; }
        public RetreatState RetreatState { get; private set; }

        private TargetDetector _targetDector;

        private RichAI _pathFinder;
        private RiflemanAnimationController _enemyAnimationController;

        private EnemyHealth _enemyHealth;

        private HitboxRoot _hitboxRoot;

        private RVOController _rvoController;
        private SkinnedMeshRagdoll _skinnedMeshRagdoll;

        public override void InitializeStateMachine()
        {
            _pathFinder = GetComponent<RichAI>();
            _enemyAnimationController = GetComponent<RiflemanAnimationController>();

            _hitboxRoot = GetComponent<HitboxRoot>();
            _rvoController = GetComponent<RVOController>();

            _enemyHealth = GetComponent<EnemyHealth>();
            _enemyHealth.OnDamaged += OnDamaged;
            _enemyHealth.OnDead += RPC_OnDeath;

            _targetDector = new TargetDetector(transform);

            DetectionState = new DetectionState(transform, _targetDector, _detectionStateData);
            AttackState = new SoldierAttackState(Runner, _enemyAnimationController, _pathFinder,
                _rifleMountGrenadeLauncher, _targetDector, _attackStateData);

            GroupChaseState = new GroupChaseState(this, _pathFinder);
            GroupLeaderState = new GroupLeaderState(this, _pathFinder, _groupChaseStateData);

            RetreatState = new RetreatState(transform, _pathFinder, _detectionStateData.RetreatRange, _retreatStateData);

            InitializeState();

            // Attach TargetTransform Observers
            _targetDector.AttachDetectableState(DetectionState);
            _targetDector.AttachDetectableState(GroupLeaderState);
            _targetDector.AttachDetectableState(RetreatState);
            _targetDector.AttachDetectableState(AttackState);

            AddStaticState(DetectionState);
            AddStaticState(AttackState);
        }

        public void ResetCharacter()
        {
            _hitboxRoot.HitboxRootActive = true;
            _rvoController.enabled = true;

            _pathFinder.enabled = true;
            _enemyAnimationController.IsAnimatorEnabled = true;

            EnableExecuteState = true;

            _skinnedMeshRagdoll.ResetSkinnedMeshBones();

            _rifleMountGrenadeLauncher.Reload();

            _enemyDissolver.ResetShader();
        }

        #region DetectionStateFunction
        public void OnTargetDetected(Vector3 direction)
        {
            // 고정포대 같은 놈들을 위해 남겨놓음!
        }
        #endregion

        #region GroupChaseFunction
        public void JoinGroup(IGroupLeader leader, float chasingSpeed)
        {
            GroupChaseState.InitializeChaseState(leader, chasingSpeed);
            TransferState(EState.GroupChaseState, GroupChaseState);

            _targetDector.SetExploringAllDirections(true);
        }

        public void LeaveGroup()
        {
            GroupChaseState.LeaveGroup();
            _targetDector.SetExploringAllDirections(false);
        }

        public void SetGroupChasePosition(Vector3 direction)
        {
            GroupChaseState.SearchPath(transform.position + direction);
        }

        public void StopGroupChasing()
        {
            _targetDector.SetExploringAllDirections(false);
        }
        #endregion

        #region GroupLeaderFunction
        public void CreateGroup()
        {
            LeaveGroup();

            TransferState(EState.GroupLeaderState, GroupLeaderState);

            _targetDector.SetExploringAllDirections(true);
        }

        public void DestroyGroup()
        {
            _targetDector.SetExploringAllDirections(false);

            RemoveCurrentState();
        }

        public void ReleaseGroupMember(IGroupChaseable member)
        {
            GroupLeaderState.ReleaseGroupMember(member);
        }
        #endregion

        #region RetreatStateFunction
        public void OnRetreatRangeEnter()
        {
            GroupLeaderState.StopChasing();
            if (StateEquals(EState.RetreatState) == true)
            {
                return;
            }

            TransferState(EState.RetreatState, RetreatState);
        }
        #endregion

        private void Awake()
        {
            _skinnedMeshRagdoll = GetComponent<SkinnedMeshRagdoll>();
        }

        private void OnDisable()
        {
            _enemyHealth.OnDamaged -= OnDamaged;
            _enemyHealth.OnDead -= RPC_OnDeath;
        }

        [Rpc(RpcSources.StateAuthority, RpcTargets.All)]
        private void RPC_OnDeath()
        {
            _hitboxRoot.HitboxRootActive = false;
            _rvoController.enabled = false;

            _pathFinder.enabled = false;
            _enemyAnimationController.IsAnimatorEnabled = false;

            RemoveCurrentState();
            EnableExecuteState = false;

            _skinnedMeshRagdoll.RemoveSkinnedMeshBones();

            _enemyDissolver.Dissolve(OnDissolveCompleted);

            ItemManager.SpawnFieldItem(EEnemyId.Rifleman, transform.position, transform.rotation);
        }

        private void OnDissolveCompleted()
        {
            EnemyManager.DespawnEnemy(Object, this);
        }

        private void OnDamaged()
        {
            CreateGroup();
        }

#if UNITY_EDITOR
        [Header("Gizmo Settings")]
        [SerializeField]
        private bool _onDrawGizmo;

        private void OnDrawGizmos()
        {
            if (_onDrawGizmo == false)
            {
                return;
            }

            _detectionStateData.DrawDetectionRangeGizmo(transform);
            _attackStateData.DrawAttackRangeGizmo(transform);
            _groupChaseStateData.DrawGizmo(transform);
        }
#endif
    }
}