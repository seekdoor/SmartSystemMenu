using System;
using System.Linq;
using SmartSystemMenu.Native.Structs;
using SmartSystemMenu.Settings;
using SmartSystemMenu.Extensions;
using static SmartSystemMenu.Native.User32;
using static SmartSystemMenu.Native.Kernel32;
using static SmartSystemMenu.Native.Constants;


namespace SmartSystemMenu.HotKeys
{
    class HotKeyHook : IDisposable
    {
        private IntPtr _hookHandle;
        private KeyboardHookProc _hookProc;
        public ApplicationSettings Settings { get; set; }

        public event EventHandler<HotKeyEventArgs> MenuItemHooked;
        public event EventHandler<HotKeyEventArgs> MoveToHooked;

        public bool Start(ApplicationSettings settings, string moduleName)
        {
            Settings = settings;
            _hookProc = HookProc;
            var moduleHandle = GetModuleHandle(moduleName);
            _hookHandle = SetWindowsHookEx(WH_KEYBOARD_LL, _hookProc, moduleHandle, 0);
            var hookStarted = _hookHandle != IntPtr.Zero;
            return hookStarted;
        }

        public bool Stop()
        {
            if (_hookHandle == IntPtr.Zero)
            {
                return true;
            }
            var hookStoped = UnhookWindowsHookEx(_hookHandle);
            return hookStoped;
        }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                // get rid of managed resources
            }

            Stop();
        }

        ~HotKeyHook()
        {
            Dispose(false);
        }

        private int HookProc(int code, IntPtr wParam, ref KeyboardLLHookStruct lParam)
        {
            if (code == HC_ACTION)
            {
                if (wParam.ToInt32() == WM_KEYDOWN || wParam.ToInt32() == WM_SYSKEYDOWN)
                {
                    if ((int)Settings.NextMonitor.Key3 == lParam.vkCode)
                    {
                        var key1 = true;
                        var key2 = true;

                        if (Settings.NextMonitor.Key1 != VirtualKeyModifier.None)
                        {
                            var key1State = GetAsyncKeyState((int)Settings.NextMonitor.Key1) & 0x8000;
                            key1 = Convert.ToBoolean(key1State);
                        }

                        if (Settings.NextMonitor.Key2 != VirtualKeyModifier.None)
                        {
                            var key2State = GetAsyncKeyState((int)Settings.NextMonitor.Key2) & 0x8000;
                            key2 = Convert.ToBoolean(key2State);
                        }

                        if (key1 && key2)
                        {
                            var handler = MoveToHooked;
                            if (handler != null)
                            {
                                var eventArgs = new HotKeyEventArgs();
                                eventArgs.NextMonitor = true;
                                handler.Invoke(this, eventArgs);
                                if (eventArgs.Succeeded)
                                {
                                    return 1;
                                }
                            }
                        }
                    }

                    if ((int)Settings.PreviousMonitor.Key3 == lParam.vkCode)
                    {
                        var key1 = true;
                        var key2 = true;

                        if (Settings.PreviousMonitor.Key1 != VirtualKeyModifier.None)
                        {
                            var key1State = GetAsyncKeyState((int)Settings.PreviousMonitor.Key1) & 0x8000;
                            key1 = Convert.ToBoolean(key1State);
                        }

                        if (Settings.PreviousMonitor.Key2 != VirtualKeyModifier.None)
                        {
                            var key2State = GetAsyncKeyState((int)Settings.PreviousMonitor.Key2) & 0x8000;
                            key2 = Convert.ToBoolean(key2State);
                        }

                        if (key1 && key2)
                        {
                            var handler = MoveToHooked;
                            if (handler != null)
                            {
                                var eventArgs = new HotKeyEventArgs();
                                eventArgs.PreviousMonitor = true;
                                handler.Invoke(this, eventArgs);
                                if (eventArgs.Succeeded)
                                {
                                    return 1;
                                }
                            }
                        }
                    }

                    foreach (var item in Settings.MenuItems.Items.Flatten(x => x.Items).Where(x => x.Type == MenuItemType.Item))
                    {
                        if (item.Shortcut.Key3 == VirtualKey.None || lParam.vkCode != (int)item.Shortcut.Key3)
                        {
                            continue;
                        }

                        var key1 = true;
                        var key2 = true;

                        if (item.Shortcut.Key1 != VirtualKeyModifier.None)
                        {
                            var key1State = GetAsyncKeyState((int)item.Shortcut.Key1) & 0x8000;
                            key1 = Convert.ToBoolean(key1State);
                        }

                        if (item.Shortcut.Key2 != VirtualKeyModifier.None)
                        {
                            var key2State = GetAsyncKeyState((int)item.Shortcut.Key2) & 0x8000;
                            key2 = Convert.ToBoolean(key2State);
                        }

                        if (key1 && key2 && lParam.vkCode == (int)item.Shortcut.Key3)
                        {
                            var handler = MenuItemHooked;
                            if (handler != null)
                            {
                                var menuItemId = MenuItemId.GetId(item.Name);
                                var eventArgs = new HotKeyEventArgs(menuItemId);
                                handler.Invoke(this, eventArgs);
                                if (eventArgs.Succeeded)
                                {
                                    return 1;
                                }
                            }
                        }
                    }

                    foreach (var item in Settings.MenuItems.WindowSizeItems)
                    {
                        if (item.Shortcut.Key3 == VirtualKey.None || lParam.vkCode != (int)item.Shortcut.Key3)
                        {
                            continue;
                        }

                        var key1 = true;
                        var key2 = true;

                        if (item.Shortcut.Key1 != VirtualKeyModifier.None)
                        {
                            var key1State = GetAsyncKeyState((int)item.Shortcut.Key1) & 0x8000;
                            key1 = Convert.ToBoolean(key1State);
                        }

                        if (item.Shortcut.Key2 != VirtualKeyModifier.None)
                        {
                            var key2State = GetAsyncKeyState((int)item.Shortcut.Key2) & 0x8000;
                            key2 = Convert.ToBoolean(key2State);
                        }

                        if (key1 && key2 && lParam.vkCode == (int)item.Shortcut.Key3)
                        {
                            var handler = MenuItemHooked;
                            if (handler != null)
                            {
                                var eventArgs = new HotKeyEventArgs(item.Id);
                                handler.Invoke(this, eventArgs);
                                if (eventArgs.Succeeded)
                                {
                                    return 1;
                                }
                            }
                        }
                    }
                }
            }

            return CallNextHookEx(_hookHandle, code, wParam, ref lParam);
        }
    }
}
