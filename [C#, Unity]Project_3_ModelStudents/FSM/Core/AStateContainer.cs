using System;
using ProbyEditor;

namespace ModelStudents
{
    [Author("Yoon")]
    [Serializable]
    public abstract class AStateContainer : AState
    {
        public Action BeginStateAction;
        public Action StateAction;
        public Action EndStateAction;

        public AStateContainer()
        {
            BeginStateAction = ExecuteBeginState;
            StateAction = ExecuteState;
            EndStateAction = ExecuteEndState;
        }
    }

    public interface IStateData
    {
    }
}