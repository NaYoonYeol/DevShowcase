using UnityEngine;
using ProbyEditor;
using System;

namespace ModelStudents
{
    [Author("Yoon")]
    public class FieldItem : MonoBehaviour
    {
        [field: SerializeField]
        public EItemId ItemId { get; private set; }

        [field: SerializeField]
        public float Amount { get; private set; }

        public void InitializeItem(EItemId itemId, int amount)
        {
            ItemId = itemId;
            Amount = amount;
        }

        public Equipment GetItemData()
        {
            return ItemManager.CreateEquipment(ItemId);
        }
    }
}