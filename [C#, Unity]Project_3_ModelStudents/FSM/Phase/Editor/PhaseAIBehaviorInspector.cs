using System.Collections;
using System.Collections.Generic;
using UnityEditorInternal;
using UnityEditor;
using UnityEngine;
using ProbyEditor;

namespace ModelStudents
{
    [Author("Yoon")]
    [CustomEditor(typeof(APhaseAIBehavior), true)]
    public class PhaseAIBehaviorInspector : Editor
    {
        private bool[] _phaseFoldout = new bool[APhaseAIBehavior.PHASE_COUNT];
        private bool _finalEventFoldout = false;

        [SerializeField]
        private List<EState>[] _phaseStateContainerList = new List<EState>[APhaseAIBehavior.PHASE_COUNT];
        [SerializeField]
        private List<EState>[] _phaseStaticStateContainerList = new List<EState>[APhaseAIBehavior.PHASE_COUNT];

        private ReorderableList[] _stateContainers = new ReorderableList[APhaseAIBehavior.PHASE_COUNT + 1];
        private ReorderableList[] _staticStateContainers = new ReorderableList[APhaseAIBehavior.PHASE_COUNT + 1];

        private APhaseAIBehavior _phaseAIBehavior;

        private void OnEnable()
        {
            _phaseAIBehavior = (APhaseAIBehavior)target;

            for (int i = 0; i < APhaseAIBehavior.PHASE_COUNT; ++i)
            {
                _phaseStateContainerList[i] = new List<EState>();
                _stateContainers[i] = new ReorderableList(_phaseStateContainerList[i], typeof(EState), true, true, true, true);
                InitReorderableList(_phaseAIBehavior.PhaseStateContainers[i].ContainerDataPairs,
                    _stateContainers[i], _phaseStateContainerList[i], "States Settings");

                _phaseStaticStateContainerList[i] = new List<EState>();
                _staticStateContainers[i] = new ReorderableList(_phaseStaticStateContainerList[i], typeof(EState), true, true, true, true);
                InitReorderableList(_phaseAIBehavior.PhaseStateContainers[i].StaticContainerDataPairs,
                    _staticStateContainers[i], _phaseStaticStateContainerList[i], "Static State Settings");
            }
        }

        public override void OnInspectorGUI()
        {
            serializedObject.Update();

            EditorGUILayout.BeginVertical();
            EditorGUILayout.PropertyField(serializedObject.FindProperty("_interpolationDataSource"));
            EditorGUILayout.PropertyField(serializedObject.FindProperty("_onDrawGizmo"));

            for (int i = 0; i < APhaseAIBehavior.PHASE_COUNT - 1; ++i)
            {
                EditorGUILayout.BeginVertical(GUI.skin.box);
                _phaseFoldout[i] = EditorGUILayout.Foldout(_phaseFoldout[i], $"Phase {i + 1} Settings");
                if (_phaseFoldout[i])
                {
                    ShowTransferStateLayout(i);
                    _stateContainers[i].DoLayoutList();
                    _staticStateContainers[i].DoLayoutList();
                }
                EditorGUILayout.EndVertical();
            }

            EditorGUILayout.BeginVertical(GUI.skin.box);

            _finalEventFoldout = EditorGUILayout.Foldout(_finalEventFoldout, "Final Event Settings");
            if (_finalEventFoldout)
            {
                ShowTransferStateLayout(APhaseAIBehavior.PHASE_COUNT - 1);
                _stateContainers[APhaseAIBehavior.PHASE_COUNT - 1].DoLayoutList();
            }

            EditorGUILayout.EndVertical();

            EditorGUILayout.BeginVertical(GUI.skin.box);

            EditorGUILayout.EndVertical();

            EditorGUILayout.EndVertical();

            serializedObject.ApplyModifiedProperties();

            void ShowTransferStateLayout(int phaseIndex)
            {
                EditorGUILayout.BeginVertical(GUI.skin.box);
                EditorGUILayout.LabelField("Transfer State");
                EditorGUI.BeginChangeCheck();
                EState state = (EState)EditorGUILayout.EnumPopup(_phaseAIBehavior.PhaseStateContainers[phaseIndex].TransferState.State);
                if (EditorGUI.EndChangeCheck())
                {
                    _phaseAIBehavior.PhaseStateContainers[phaseIndex].TransferState = CreateState(state);
                    EditorUtility.SetDirty(target);
                }
                DrawDataProperty(state, _phaseAIBehavior.PhaseStateContainers[phaseIndex].TransferState);

                EditorGUILayout.EndVertical();
            }
        }

        private void InitReorderableList(List<ContainerDataPair> baseList, ReorderableList reorderableList, List<EState> stateList, string listName)
        {
            ReloadState(baseList, reorderableList);
            reorderableList.drawHeaderCallback = (rect) =>
            {
                EditorGUI.LabelField(rect, listName);
            };
            reorderableList.drawElementCallback = (rect, index, isActive, isFocused) =>
            {
                var element = stateList[index];
                rect.height -= 4;
                rect.y += 2;

                EditorGUI.BeginChangeCheck();
                stateList[index] = (EState)EditorGUI.EnumPopup(rect, element);
                if (EditorGUI.EndChangeCheck())
                {
                    ChangeState(baseList, stateList[index], index);
                    EditorUtility.SetDirty(target);
                }
                
                DrawDataListProperty(element, index, baseList);
            };
            reorderableList.onChangedCallback = (list) =>
            {
                ResynchronizeAllState(reorderableList, baseList);

                EditorUtility.SetDirty(target);
            };
        }

        private void DrawDataProperty(EState state, ContainerDataPair containerData)
        {
            EditorGUI.BeginChangeCheck();
            switch (state)
            {
                case EState.SoldierAttackState:
                    AttackStateData attackStateData = (AttackStateData)containerData.Data;

                    attackStateData.AttackInterval = EditorGUILayout.FloatField("Attack Interval", attackStateData.AttackInterval);
                    attackStateData.DetectionAngle = EditorGUILayout.FloatField("Detection Angle", attackStateData.DetectionAngle);
                    attackStateData.DetectionDistance = EditorGUILayout.FloatField("Detection Distance", attackStateData.DetectionDistance);
                    attackStateData.DetectionLayer = EditorGUILayout.LayerField("Detection Layer", attackStateData.DetectionLayer);
                    attackStateData.ObstacleLayer = EditorGUILayout.LayerField("Obstacle Layer", attackStateData.ObstacleLayer);
                    break;

                case EState.DetectionState:
                    DetectionStateData detectionStateData = (DetectionStateData)containerData.Data;

                    detectionStateData.DetectionAngle = EditorGUILayout.FloatField("Detection Angle", detectionStateData.DetectionAngle);
                    detectionStateData.DetectionDistance = EditorGUILayout.FloatField("Detection Distance", detectionStateData.DetectionDistance);
                    detectionStateData.DetectionInterval = EditorGUILayout.FloatField("Detection Interval", detectionStateData.DetectionInterval);
                    detectionStateData.DetectionLayer = EditorGUILayout.LayerField("Detection Layer", detectionStateData.DetectionLayer);
                    detectionStateData.IsRetreatable = EditorGUILayout.Toggle("Is Retreatable", detectionStateData.IsRetreatable);
                    detectionStateData.ObstacleLayer = EditorGUILayout.LayerField("Obstacle Layer", detectionStateData.ObstacleLayer);
                    detectionStateData.RetreatRange = EditorGUILayout.FloatField("Retreat Range", detectionStateData.RetreatRange);
                    break;

                case EState.RandomTargetAttackState:
                    RandomAttackStateData randomTargetAttackStateData = (RandomAttackStateData)containerData.Data;

                    randomTargetAttackStateData.AttackInterval = EditorGUILayout.FloatField("Attack Interval", randomTargetAttackStateData.AttackInterval);
                    randomTargetAttackStateData.CirculationInterval = EditorGUILayout.FloatField("Circulation Interval", randomTargetAttackStateData.CirculationInterval);
                    randomTargetAttackStateData.CirculationCount = EditorGUILayout.IntField("Circulation Count", randomTargetAttackStateData.CirculationCount);
                    randomTargetAttackStateData.Weapon = (AEnemyWeapon)EditorGUILayout.ObjectField("Weapon", randomTargetAttackStateData.Weapon, typeof(AEnemyWeapon), true);
                    break;

                case EState.RandomLocationAttackState:
                    RandomLocationAttackStateData randomLocationAttackStateData = (RandomLocationAttackStateData)containerData.Data;

                    randomLocationAttackStateData.AttackInterval = EditorGUILayout.FloatField("Attack Interval", randomLocationAttackStateData.AttackInterval);
                    randomLocationAttackStateData.CirculationInterval = EditorGUILayout.FloatField("Circulation Interval", randomLocationAttackStateData.CirculationInterval);
                    randomLocationAttackStateData.CirculationCount = EditorGUILayout.IntField("Circulation Count", randomLocationAttackStateData.CirculationCount);
                    randomLocationAttackStateData.Weapon = (AEnemyWeapon)EditorGUILayout.ObjectField("Weapon", randomLocationAttackStateData.Weapon, typeof(AEnemyWeapon), true);
                    randomLocationAttackStateData.Center = EditorGUILayout.Vector3Field("Center", randomLocationAttackStateData.Center);
                    randomLocationAttackStateData.Radius = EditorGUILayout.FloatField("Radius", randomLocationAttackStateData.Radius);
                    break;

                case EState.EmissionState:
                    ExplosiveStateData emissionStateData = (ExplosiveStateData)containerData.Data;

                    emissionStateData.EmissionVFX = (EVFX)EditorGUILayout.EnumPopup("Emission VFX", emissionStateData.EmissionVFX);
                    emissionStateData.ChargingTime = EditorGUILayout.FloatField("Charging Time", emissionStateData.ChargingTime);
                    emissionStateData.RadiusSizeCurve = EditorGUILayout.CurveField("Radius Size Curve", emissionStateData.RadiusSizeCurve);
                    emissionStateData.MaxRadius = EditorGUILayout.FloatField("Max Radius", emissionStateData.MaxRadius);
                    emissionStateData.Duration = EditorGUILayout.FloatField("Duration", emissionStateData.Duration);
                    emissionStateData.Damage = EditorGUILayout.FloatField("Damage", emissionStateData.Damage);
                    emissionStateData.Repetition = EditorGUILayout.IntField("Repetition", emissionStateData.Repetition);
                    break;

                case EState.DetectingExplosiveState:
                    DetectingExplosiveStateData detectingExplosiveStateData = (DetectingExplosiveStateData)containerData.Data;

                    detectingExplosiveStateData.EmissionVFX = (EVFX)EditorGUILayout.EnumPopup("Emission VFX", detectingExplosiveStateData.EmissionVFX);
                    detectingExplosiveStateData.ChargingTime = EditorGUILayout.FloatField("Charging Time", detectingExplosiveStateData.ChargingTime);
                    detectingExplosiveStateData.RadiusSizeCurve = EditorGUILayout.CurveField("Radius Size Curve", detectingExplosiveStateData.RadiusSizeCurve);
                    detectingExplosiveStateData.MaxRadius = EditorGUILayout.FloatField("Max Radius", detectingExplosiveStateData.MaxRadius);
                    detectingExplosiveStateData.Duration = EditorGUILayout.FloatField("Duration", detectingExplosiveStateData.Duration);
                    detectingExplosiveStateData.Damage = EditorGUILayout.FloatField("Damage", detectingExplosiveStateData.Damage);
                    detectingExplosiveStateData.Repetition = EditorGUILayout.IntField("Repetition", detectingExplosiveStateData.Repetition);

                    detectingExplosiveStateData.DetectingRadius = EditorGUILayout.FloatField("Detecting Radius", detectingExplosiveStateData.DetectingRadius);
                    break;

                case EState.MultipleSpiralState:
                    MultipleSprialStateData multipleSprialStateData = (MultipleSprialStateData)containerData.Data;

                    multipleSprialStateData.MultipleSpiralLauncher = (Weapon)EditorGUILayout.ObjectField("Weapon", multipleSprialStateData.MultipleSpiralLauncher, typeof(Weapon), true);
                    multipleSprialStateData.Interval = EditorGUILayout.FloatField("Interval", multipleSprialStateData.Interval);
                    multipleSprialStateData.ShootAngle = EditorGUILayout.IntField("Shoot Angle", multipleSprialStateData.ShootAngle);
                    multipleSprialStateData.AngularVelocity = EditorGUILayout.IntField("Angular Velocity", multipleSprialStateData.AngularVelocity);
                    multipleSprialStateData.ShootCount = EditorGUILayout.IntField("Shoot Count", multipleSprialStateData.ShootCount);
                    multipleSprialStateData.SfxClip = EditorGUILayout.IntField("Sfx Clip", multipleSprialStateData.SfxClip);
                    break;

                case EState.JumpState:
                    JumpStateData jumpStateData = (JumpStateData)containerData.Data;

                    jumpStateData.JumpDuration = EditorGUILayout.FloatField("Jump Duration", jumpStateData.JumpDuration);
                    jumpStateData.FallDamage = EditorGUILayout.FloatField("Fall Damage", jumpStateData.FallDamage);
                    jumpStateData.FallDamageRange = EditorGUILayout.FloatField("Fall Damage Range", jumpStateData.FallDamageRange);
                    break;

                case EState.AttackableJumpState:
                    AttackableJumpStateData attackableJumpStateData = (AttackableJumpStateData)containerData.Data;

                    attackableJumpStateData.AttackInterval = EditorGUILayout.FloatField("Attack Interval", attackableJumpStateData.AttackInterval);
                    attackableJumpStateData.CirculationInterval = EditorGUILayout.FloatField("Circulation Interval", attackableJumpStateData.CirculationInterval);
                    attackableJumpStateData.CirculationCount = EditorGUILayout.IntField("Circulation Count", attackableJumpStateData.CirculationCount);
                    attackableJumpStateData.Weapon = (AEnemyWeapon)EditorGUILayout.ObjectField("Weapon", attackableJumpStateData.Weapon, typeof(AEnemyWeapon), true);

                    attackableJumpStateData.JumpDuration = EditorGUILayout.FloatField("Jump Duration", attackableJumpStateData.JumpDuration);
                    attackableJumpStateData.FallDamage = EditorGUILayout.FloatField("Fall Damage", attackableJumpStateData.FallDamage);
                    attackableJumpStateData.FallDamageRange = EditorGUILayout.FloatField("Fall Damage Range", attackableJumpStateData.FallDamageRange);
                    break;

                case EState.SuicideState:
                    SuicideStateData suicideStateData = (SuicideStateData)containerData.Data;

                    suicideStateData.EmissionVFX = (EVFX)EditorGUILayout.EnumPopup("Emission VFX", suicideStateData.EmissionVFX);
                    suicideStateData.ChargingTime = EditorGUILayout.FloatField("Charging Time", suicideStateData.ChargingTime);
                    suicideStateData.RadiusSizeCurve = EditorGUILayout.CurveField("Radius Size Curve", suicideStateData.RadiusSizeCurve);
                    suicideStateData.MaxRadius = EditorGUILayout.FloatField("Max Radius", suicideStateData.MaxRadius);
                    suicideStateData.Duration = EditorGUILayout.FloatField("Duration", suicideStateData.Duration);
                    suicideStateData.Damage = EditorGUILayout.FloatField("Damage", suicideStateData.Damage);
                    suicideStateData.Repetition = EditorGUILayout.IntField("Repetition", suicideStateData.Repetition);

                    suicideStateData.ShouldBreak = EditorGUILayout.Toggle("Should Break", suicideStateData.ShouldBreak);
                    break;

                case EState.ChasableSuicideState:
                    ChasableSuicideStateData chasableSuicideStateData = (ChasableSuicideStateData)containerData.Data;

                    chasableSuicideStateData.EmissionVFX = (EVFX)EditorGUILayout.EnumPopup("Emission VFX", chasableSuicideStateData.EmissionVFX);
                    chasableSuicideStateData.ChargingTime = EditorGUILayout.FloatField("Charging Time", chasableSuicideStateData.ChargingTime);
                    chasableSuicideStateData.RadiusSizeCurve = EditorGUILayout.CurveField("Radius Size Curve", chasableSuicideStateData.RadiusSizeCurve);
                    chasableSuicideStateData.MaxRadius = EditorGUILayout.FloatField("Max Radius", chasableSuicideStateData.MaxRadius);
                    chasableSuicideStateData.Duration = EditorGUILayout.FloatField("Duration", chasableSuicideStateData.Duration);
                    chasableSuicideStateData.Damage = EditorGUILayout.FloatField("Damage", chasableSuicideStateData.Damage);
                    chasableSuicideStateData.Repetition = EditorGUILayout.IntField("Repetition", chasableSuicideStateData.Repetition);

                    chasableSuicideStateData.ShouldBreak = EditorGUILayout.Toggle("Should Break", chasableSuicideStateData.ShouldBreak);

                    chasableSuicideStateData.ChaseTime = EditorGUILayout.FloatField("Chase Time", chasableSuicideStateData.ChaseTime);
                    chasableSuicideStateData.ChaseSpeed = EditorGUILayout.FloatField("Chase Speed", chasableSuicideStateData.ChaseSpeed);
                    break;

                default:
                    break;
            }
            if (EditorGUI.EndChangeCheck())
            {
                EditorUtility.SetDirty(target);
            }
        }

        private void DrawDataListProperty(EState state, int elementIndex, List<ContainerDataPair> baseList)
        {
            EditorGUILayout.BeginVertical(GUI.skin.box);

            DrawDataProperty(state, baseList[elementIndex]);

            EditorGUILayout.EndVertical();
        }

        private void ReloadState(List<ContainerDataPair> baseList, ReorderableList targetList)
        {
            foreach (var state in baseList)
            {
                targetList.list.Add(state.State);
            }
        }

        private void ResynchronizeAllState(ReorderableList baseList, List<ContainerDataPair> targetList)
        {
            targetList.Clear();

            IList reorderableList = baseList.list;
            for (int i = 0; i < reorderableList.Count; ++i)
            {
                EState element = (EState)reorderableList[i];

                AddState(targetList, element);
            }
        }

        private void AddState(List<ContainerDataPair> targetList, EState state)
        {
            targetList.Add(CreateState(state));
        }

        private void ChangeState(List<ContainerDataPair> targetList, EState state, int index)
        {
            targetList[index] = CreateState(state);
        }

        private ContainerDataPair CreateState(EState state)
        {
            switch (state)
            {
                case EState.None:
                    return new ContainerDataPair(state, null, null);

                case EState.SoldierAttackState:
                    return new ContainerDataPair(state, new SoldierAttackState(), new AttackStateData());

                case EState.DetectionState:
                    return new ContainerDataPair(state, new DetectionState(), new DetectionStateData());

                case EState.RandomTargetAttackState:
                    return new ContainerDataPair(state, new RandomTargetAttackState(), new RandomAttackStateData());

                case EState.RandomLocationAttackState:
                    return new ContainerDataPair(state, new RandomLocationAttackState(), new RandomLocationAttackStateData());

                case EState.EmissionState:
                    return new ContainerDataPair(state, new EmissionState(), new ExplosiveStateData());

                case EState.DetectingExplosiveState:
                    return new ContainerDataPair(state, new ExplosiveState(), new DetectingExplosiveStateData());

                case EState.MultipleSpiralState:
                    return new ContainerDataPair(state, new MultipleSpiralState(), new MultipleSprialStateData());

                case EState.JumpState:
                    return new ContainerDataPair(state, new JumpState(), new JumpStateData());

                case EState.AttackableJumpState:
                    return new ContainerDataPair(state, new AttackableJumpState(), new AttackableJumpStateData());

                case EState.SuicideState:
                    return new ContainerDataPair(state, new SuicideState(), new SuicideStateData());

                case EState.ChasableSuicideState:
                    return new ContainerDataPair(state, new ChasableSuicideState(), new ChasableSuicideStateData());

                default:
                    Debug.LogError("The part that creates the state has not been created yet.");
                    return new ContainerDataPair(EState.None, null, null);
            }
        }
    }
}