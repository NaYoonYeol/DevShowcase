using ProbyEditor;

namespace ModelStudents
{
    [Author("Yoon")]
    public class EmissionState : AExplosiveState
    {
        protected override bool ReadyToExplode()
        {
            return true;
        }
    }
}