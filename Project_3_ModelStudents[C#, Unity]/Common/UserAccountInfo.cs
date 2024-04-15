using System;
using ProbyEditor;

namespace ModelStudents
{
    [Author("Common")]
    public static class UserAccountInfo
    {
        public static AccountInfo Info { get; }

        static UserAccountInfo()
        {
            //Temporary user account settings
            Info = new AccountInfo(Environment.UserName, 0);
        }
    }

    [Author("Common")]
    public class AccountInfo
    {
        public string UserName = "";
        public byte CharacterAccountLevel = 0;

        public AccountInfo() { }

        public AccountInfo(string userName, byte characterAccountLevel)
        {
            UserName = userName;
            CharacterAccountLevel = characterAccountLevel;
        }
    }
}