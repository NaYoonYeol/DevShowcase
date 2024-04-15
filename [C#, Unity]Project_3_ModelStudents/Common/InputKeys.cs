using UnityEngine;
using ProbyEditor;

namespace ModelStudents
{
    [Author("Common")]
    public static class InputKeys
    {
        //Move Inputs
        public static KeyCode Forward = KeyCode.W;
        public static KeyCode Back = KeyCode.S;
        public static KeyCode Left = KeyCode.A;
        public static KeyCode Right = KeyCode.D;
        public static KeyCode Blink = KeyCode.Space;

        //Interaction
        public static KeyCode Zoom = KeyCode.BackQuote;

        //UI
        public static KeyCode Chat = KeyCode.Return;
        public static KeyCode Inventory = KeyCode.I;
        public static KeyCode Quit = KeyCode.Escape;
    }
}