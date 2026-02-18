using System;

namespace SmartSystemMenu.HotKeys
{
    class HotKeyEventArgs : EventArgs
    {
        public int MenuItemId { get; }

        public bool NextMonitor { get; set; }

        public bool PreviousMonitor { get; set; }

        public bool Succeeded { get; set; }

        public HotKeyEventArgs()
        {
        }

        public HotKeyEventArgs(int menuItemId)
        {
            MenuItemId = menuItemId;
        }
    }
}
