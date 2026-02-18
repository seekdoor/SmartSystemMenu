using System;

namespace SmartSystemMenu.Settings
{
    public class WindowSizeMenuItem : ICloneable
    {
        public MenuItemType Type { get; set; }

        public int Id { get; set; }

        public string Title { get; set; }

        public int? Left { get; set; }
        
        public int? Top { get; set; }

        public int? Width { get; set; }

        public int? Height { get; set; }

        public KeyboardShortcut Shortcut { get; set; }

        public WindowSizeMenuItem()
        {
            Type = MenuItemType.Item;
            Id = 0;
            Title = string.Empty;
            Left = null;
            Top = null;
            Width = null;
            Height = null;
            Shortcut = new KeyboardShortcut();
        }

        public object Clone()
        {
            var menuItemClone = (WindowSizeMenuItem)MemberwiseClone();
            menuItemClone.Shortcut = (KeyboardShortcut)Shortcut.Clone();
            return menuItemClone;
        }
    }
}
