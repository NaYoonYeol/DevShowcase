using System.Threading.Tasks;
using Microsoft.Azure.Cosmos.Table;
using System.Linq;
using ProbyEditor;
using System;

namespace ModelStudents
{
    [Author("Yoon")]
    public class AzureCloudStorageConnector
    {
        private const string CONNECTION_STRING = "DefaultEndpointsProtocol=https;AccountName=modelstudent;" +
            "AccountKey=m1p8uaP70CDstd10AIlzZ5PmRzOFU1XpNGYeYzc7asviSyRPKqXZlMlzEZGbz0DFHMHYfGrbj0TtFRuqfIr2HQ==;" +
            "TableEndpoint=https://modelstudent.table.cosmos.azure.com:443/;";

        private const string EQUIPMENT_INFO_TABLE_NAME = "EquipmentInfoTable";
        private const string ITEM_DROP_TABLE_NAME = "ItemDropTable";
        private const string PLAYER_STAT_TABLE_NAME = "PlayerStat";
        private const string REQUIRED_EXP_TABLE_NAME = "RequiredExpTable";
        private const string PLAYER_SKILL_TABLE_NAME = "PlayerSkillTable";

        public async Task<EquipmentItemData[]> GetEquipmentTableAsync()
        {
            TableQuerySegment<EquipmentItemData> elements = await GetTableData<EquipmentItemData>(EQUIPMENT_INFO_TABLE_NAME);
            foreach(var element in elements)
            {
                element.ItemType = (EItemType)Enum.Parse(typeof(EItemType), element.PartitionKey);
            }

            return elements.ToArray();
        }

        public async Task<DroppableItemData[]> GetItemDropTableAsync()
        {
            TableQuerySegment<DroppableItemData> elements = await GetTableData<DroppableItemData>(ITEM_DROP_TABLE_NAME);

            return elements.ToArray();
        }

        public async Task<DefaultPlayerStat> GetPlayerDefaultStatAsync()
        {
            TableQuerySegment<PlayerStatData> elements = await GetTableData<PlayerStatData>(PLAYER_STAT_TABLE_NAME);
            DefaultPlayerStat defaultPlayerStat = new DefaultPlayerStat
            {
                Health = (float)elements.ElementAt(0).Value,
                HealthRegen = (float)elements.ElementAt(1).Value
            };

            return defaultPlayerStat;
        }

        public async Task<RequiredExpData[]> GetRequiredExpTableAsync()
        {
            TableQuerySegment<RequiredExpData> elements = await GetTableData<RequiredExpData>(REQUIRED_EXP_TABLE_NAME);

            return elements.ToArray();
        }

        public async Task<PlayerSkillData[]> GetPlayerSkillTableAsync()
        {
            TableQuerySegment<PlayerSkillData> elements = await GetTableData<PlayerSkillData>(PLAYER_SKILL_TABLE_NAME);
            foreach(var element in elements)
            {
                element.Type = (ESkillType)Enum.Parse(typeof(ESkillType), element.PartitionKey);
            }

            return elements.ToArray();
        }

        private async Task<TableQuerySegment<T>> GetTableData<T>(string tableName) where T : TableEntity, new()
        {
            CloudStorageAccount storageAccount = CloudStorageAccount.Parse(CONNECTION_STRING);
            CloudTableClient tableClient = storageAccount.CreateCloudTableClient();
            CloudTable table = tableClient.GetTableReference(tableName);

            return await table.ExecuteQuerySegmentedAsync(new TableQuery<T>(), null);
        }
    }

    [Author("Yoon")]
    public class EquipmentItemData : TableEntity
    {
        public EItemType ItemType { get; set; }
        public string Description { get; set; }
        public int BuyPrice { get; set; }
        public int SellPrice { get; set; }
        public double Durability { get; set; }
        public string SpecialAbility { get; set; }
        public double Damage { get; set; }
        public double AttackSpeed { get; set; }
        public string IconName { get; set; }
    }

    [Author("Yoon")]
    public class DroppableItemData : TableEntity
    {
        public bool IsAutoAcquisition { get; set; }
        public int MaxAmount { get; set; }
        public int MinAmount { get; set; }
        public int Probability { get; set; }
    }

    [Author("Yoon")]
    public class PlayerStatData : TableEntity
    {
        public double Value { get; set; }
    }

    [Author("Yoon")]
    public class RequiredExpData : TableEntity
    {
        public int RequiredExp { get; set; }
    }

    [Author("Yoon")]
    public class PlayerSkillData : TableEntity
    {
        public ESkillType Type { get; set; }
        public string Name => RowKey;
        public string Description { get; set; }
        public string IconName { get; set; }
    }
}