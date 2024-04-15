using UnityEngine;
using ProbyEditor;

namespace ModelStudents
{
    [Author("Yoon")]
    public class SkinnedMeshRagdoll : MonoBehaviour
    {
        [Header("Head Physics Settings")]
        [SerializeField]
        private Transform _headTransform;
        [SerializeField]
        private Rigidbody _headRigidbody;
        [SerializeField]
        private SkinnedMeshRenderer _headSkinnedMeshRenderer;

        [Header("LowerBody Phisics Settings")]
        [SerializeField]
        private Transform _lowerBodyTransform;
        [SerializeField]
        private Rigidbody _lowerBodyRigidbody;
        [SerializeField]
        private SkinnedMeshRenderer _lowerBodyMeshRenderer;

        [Header("Root Mesh Object")]
        [SerializeField]
        private GameObject _rootMeshObject;

        private Transform[] _headSkinnedMeshBonesOrigin;
        private Transform _headSkinnedMeshRootBoneOrigin;

        private Transform[] _lowerBodySkinnedMeshBonesOrigin;
        private Transform _lowerBodySkinnedMeshRootBoneOrigin;

        private Collider[] _characterColliders;
        private Rigidbody[] _characterRigidbodies;

        public void RemoveSkinnedMeshBones()
        {
            // Remove SkinnnedMeshBones
            _headSkinnedMeshRenderer.bones = null;
            _headSkinnedMeshRenderer.rootBone = _headSkinnedMeshRenderer.transform;
            _headSkinnedMeshRenderer.transform.position = _headTransform.position;

            _lowerBodyMeshRenderer.bones = null;
            _lowerBodyMeshRenderer.rootBone = _lowerBodyMeshRenderer.transform;
            _lowerBodyMeshRenderer.transform.position = _lowerBodyTransform.position;

            // Play Ragdoll
            _headRigidbody.isKinematic = false;
            _lowerBodyRigidbody.isKinematic = false;

            for (int i = 0; i < _characterColliders.Length; ++i)
            {
                _characterColliders[i].enabled = true;
            }
            for (int i = 0; i < _characterRigidbodies.Length; ++i)
            {
                _characterRigidbodies[i].isKinematic = false;
            }
        }

        public void ResetSkinnedMeshBones()
        {
            // Reset SkinnedMeshBones
            _headSkinnedMeshRenderer.bones = _headSkinnedMeshBonesOrigin;
            _headSkinnedMeshRenderer.rootBone = _headSkinnedMeshRootBoneOrigin;

            _lowerBodyMeshRenderer.bones = _lowerBodySkinnedMeshBonesOrigin;
            _lowerBodyMeshRenderer.rootBone = _lowerBodySkinnedMeshRootBoneOrigin;

            // Stop Ragdoll
            _headRigidbody.isKinematic = true;
            _lowerBodyRigidbody.isKinematic = true;

            for (int i = 0; i < _characterColliders.Length; ++i)
            {
                _characterColliders[i].enabled = false;
            }
            for (int i = 0; i < _characterRigidbodies.Length; ++i)
            {
                _characterRigidbodies[i].isKinematic = true;
            }
        }

        private void Awake()
        {
            _headSkinnedMeshBonesOrigin = _headSkinnedMeshRenderer.bones;
            _headSkinnedMeshRootBoneOrigin = _headSkinnedMeshRenderer.rootBone;

            _lowerBodySkinnedMeshBonesOrigin = _lowerBodyMeshRenderer.bones;
            _lowerBodySkinnedMeshRootBoneOrigin = _lowerBodyMeshRenderer.rootBone;

            _characterColliders = _rootMeshObject.GetComponentsInChildren<Collider>();
            _characterRigidbodies = _rootMeshObject.GetComponentsInChildren<Rigidbody>();

            _headRigidbody.isKinematic = true;
            _lowerBodyRigidbody.isKinematic = true;

            for (int i = 0; i < _characterColliders.Length; ++i)
            {
                _characterColliders[i].enabled = false;
            }
            for (int i = 0; i < _characterRigidbodies.Length; ++i)
            {
                _characterRigidbodies[i].isKinematic = true;
            }
        }
    }
}
