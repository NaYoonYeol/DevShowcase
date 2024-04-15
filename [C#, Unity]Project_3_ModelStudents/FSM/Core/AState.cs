using ProbyEditor;
using System;

namespace ModelStudents
{
    [Author("Yoon")]
    public abstract class AState
    {
        public abstract void ExecuteBeginState();
        public abstract void ExecuteState();
        public abstract void ExecuteEndState();
    }

    [Author("Yoon")]
    public class InstanceState : AState
    {
        private AAIBehaviour _instance;

        public InstanceState(AAIBehaviour instance)
        {
            _instance = instance;
        }

        public InstanceState(AAIBehaviour instance, AStateContainer stateContainer)
        {
            _instance = instance;

            _instance.BeginStateInitial = stateContainer.BeginStateAction;
            _instance.StateInitial = stateContainer.StateAction;
            _instance.EndStateInitial = stateContainer.EndStateAction;
        }

        public void TransferState(AStateContainer stateContainer)
        {
            _instance.BeginStateInitial = stateContainer?.BeginStateAction;
            _instance.StateInitial = stateContainer?.StateAction;
            _instance.EndStateInitial = stateContainer?.EndStateAction;
        }

        public override void ExecuteBeginState()
        {
            _instance.BeginStateInitial?.Invoke();
        }

        public override void ExecuteState()
        {
            _instance.StateInitial?.Invoke();
        }

        public override void ExecuteEndState()
        {
            _instance.EndStateInitial?.Invoke();
        }
    }
}