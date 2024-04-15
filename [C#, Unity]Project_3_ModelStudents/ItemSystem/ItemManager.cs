using System.Collections.Generic;
using UnityEngine;
using ProbyEditor;
using System;
using Random = UnityEngine.Random;

namespace ModelStudents
{
    [Author("Yoon")]
    [HideMonoScript]
    public class ItemManager : MonoBehaviour
    {
        [SerializeField]
        [GeneralEnumList(typeof(EItemId), 100)]
        private List<FieldItem> _itemPrefabs;

        [SerializeField]
        [GeneralEnumList(typeof(EItemId), 100)]
        private List<EquipmentItem> _equipmentPrefabs;

        private static ItemManager _instance;

        private void Awake()
        {
            _instance = this;
        }

        public static void SpawnFieldItem(EEnemyId enemyId, Vector3 position, Quaternion rotation)
        {
            if (_instance == null)
            {
                Debug.LogError("ItemManage instance is not created");
                return;
            }

            foreach (var item in GameData.DroppableItemPairs)
            {
                EnemyItemIdKey key = item.Key;
                if (key.EnemyId != enemyId)
                {
                    continue;
                }

                DroppableItemData dropItem = item.Value;
                if (IsSpawnable(dropItem.Probability) == false)
                {
                    continue;
                }

                int amount = Random.Range(dropItem.MinAmount, dropItem.MaxAmount);
                if (dropItem.IsAutoAcquisition)
                {
                    if (key.ItemId == EItemId.Exp)
                    {
                        PlayerLevelManager.ObtainEXP(amount);
                    }
                    else if (key.ItemId == EItemId.Credit)
                    {
                        CreditManager.Add(amount);
                    }

                    continue;
                }

                Debug.Log(key.ItemId);

                FieldItem filedItem = Instantiate
                (
                    _instance._itemPrefabs[Convert.ToInt32(key.ItemId)],
                    position,
                    rotation
                );

                filedItem.InitializeItem(key.ItemId, amount);
            }

            bool IsSpawnable(int probability)
            {
                return probability >= Random.Range(1, 101) ? true : false;
            }
        }

        public static EquipmentItem InstantiateEquipmentItem(EItemId itemId)
        {
            EquipmentItem item = Instantiate(_instance._equipmentPrefabs[(int)itemId]);
            item.ItemID = itemId;

            return item;
        }

        public static Equipment CreateEquipment(EItemId itemId)
        {
            Equipment equipment = null;
            EquipmentInfoTable.Data equipmentInfo = GameData.EquipmentInfoTable.GetData(itemId);
            ItemInfoTable.Data itemInfo = GameData.ItemInfoTable.GetData(itemId);

            if (equipmentInfo == null || itemInfo == null)
            {
                Debug.LogError($"Equipment Create Fail : No item ID for {itemId}");
                return null;
            }

            CommonItemSettings settings = new CommonItemSettings
            {
                Name = itemInfo.ItemID.ToString(),
                Description = itemInfo.Description,
                Type = itemInfo.ItemType,
                Icon = GameSprite.Find(itemInfo.IconID),
                ID = Guid.NewGuid().ToString()
            };

            //Grade is temp data
            float durability = GetDurability(EGrade.Supply);
            int buyPrice = GetBuyPrice(equipmentInfo, EGrade.Supply);
            int sellPrice = GetSellPrice(equipmentInfo, EGrade.Supply);

            switch (itemInfo.ItemType)
            {
                case EItemType.Sword:
                    equipment = new SwordItem(settings, durability, equipmentInfo.Damage, equipmentInfo.AttackRate);
                    break;

                case EItemType.Sheath:
                    equipment = new SheathItem(settings, durability, equipmentInfo.Damage);
                    break;

                default:
                    Debug.LogError($"Unidentified Equipment Type: {itemInfo.ItemType}");
                    break;
            }

            equipment.ItemId = itemId;
            equipment.SetCount(1);
            equipment.SetPrice(buyPrice, sellPrice);

            return equipment;
        }

        private static float GetDurability(EGrade grade)
        {
            return GameData.EquipmentGradeTable.GetData(grade).Durability;
        }

        private static int GetBuyPrice(EquipmentInfoTable.Data equipmentInfo, EGrade grade)
        {
            return 100;
        }

        private static int GetSellPrice(EquipmentInfoTable.Data equipmentInfo, EGrade grade)
        {
            return GetBuyPrice(equipmentInfo, grade) / 3;
        }
    }
}

