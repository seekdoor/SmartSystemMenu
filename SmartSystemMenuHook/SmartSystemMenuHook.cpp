// SmartSystemMenuHook.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "SmartSystemMenuHook.h"

#pragma data_seg(".Shared")
HWND  hwndMain = NULL;
HHOOK hookCbt = NULL;
HHOOK hookShell = NULL;
HHOOK hookCallWndProc = NULL;
HHOOK hookGetMsg = NULL;
#pragma data_seg()
#pragma comment(linker, "/section:.Shared,rws")

//
// Store the application instance of this module to pass to
// hook initialization. This is set in DLLMain().
//
HINSTANCE g_appInstance = NULL;

typedef void (CALLBACK *HookProc)(int nCode, WPARAM wParam, LPARAM lParam);

static LRESULT CALLBACK CbtHookCallback(int nCode, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK ShellHookCallback(int nCode, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK CallWndProcHookCallback(int nCode, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK GetMsgHookCallback(int nCode, WPARAM wParam, LPARAM lParam);
static LRESULT CALLBACK WndProcMinMax(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

HWND GetTopLevelWindow(HWND hWnd)
{
    HWND result = hWnd;
    while (GetParent(result) != 0)
    { 
        result = GetParent(result);
    }
    return result;
}

DLLEXPORT bool __stdcall InitializeCbtHook(int threadID, HWND destination)
{
    if (g_appInstance == NULL)
    {
        return false;
    }

    hwndMain = destination;
    hookCbt = SetWindowsHookEx(WH_CBT, (HOOKPROC)CbtHookCallback, g_appInstance, threadID);
    return hookCbt != NULL;
}

DLLEXPORT void __stdcall UninitializeCbtHook()
{
    if (hookCbt != NULL)
    {
        UnhookWindowsHookEx(hookCbt);
    }
    hookCbt = NULL;
}

static LRESULT CALLBACK CbtHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        UINT msg = 0;

        if (nCode == HCBT_CREATEWND)
            msg = WM_SSM_HOOK_HCBT_CREATEWND;
        else if (nCode == HCBT_DESTROYWND)
            msg = WM_SSM_HOOK_HCBT_DESTROYWND;
        else if (nCode == HCBT_MINMAX)
            msg = WM_SSM_HOOK_HCBT_MINMAX;
        else if (nCode == HCBT_MOVESIZE)
            msg = WM_SSM_HOOK_HCBT_MOVESIZE;
        else if (nCode == HCBT_ACTIVATE)
            msg = WM_SSM_HOOK_HCBT_ACTIVATE;

        if (msg != 0 && GetSystemMenu((HWND)wParam, false) != NULL)
        {
            SendNotifyMessage(hwndMain, msg, wParam, lParam);
        }
    }

    return CallNextHookEx(hookCbt, nCode, wParam, lParam);
}

DLLEXPORT bool __stdcall InitializeShellHook(int threadID, HWND destination)
{
    if (g_appInstance == NULL)
    {
        return false;
    }

    hwndMain = destination;
    hookShell = SetWindowsHookEx(WH_SHELL, (HOOKPROC)ShellHookCallback, g_appInstance, threadID);
    return hookShell != NULL;
}

DLLEXPORT void __stdcall UninitializeShellHook()
{
    if (hookShell != NULL)
    {
        UnhookWindowsHookEx(hookShell);
    }
    hookShell = NULL;
}

static LRESULT CALLBACK ShellHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        UINT msg = 0;

        if (nCode == HSHELL_WINDOWCREATED)
            msg = WM_SSM_HOOK_HSHELL_WINDOWCREATED;
        else if (nCode == HSHELL_WINDOWDESTROYED)
            msg = WM_SSM_HOOK_HSHELL_WINDOWDESTROYED;

        if (msg != 0 && GetSystemMenu((HWND)wParam, false) != NULL)
        {
            SendNotifyMessage(hwndMain, msg, wParam, lParam);
        }
    }

    return CallNextHookEx(hookShell, nCode, wParam, lParam);
}

DLLEXPORT bool __stdcall InitializeCallWndProcHook(int threadID, HWND destination)
{
    if (g_appInstance == NULL)
    {
        return false;
    }

    hwndMain = destination;
    hookCallWndProc = SetWindowsHookEx(WH_CALLWNDPROC, (HOOKPROC)CallWndProcHookCallback, g_appInstance, threadID);
    return hookCallWndProc != NULL;
}

DLLEXPORT void __stdcall UninitializeCallWndProcHook()
{
    if (hookCallWndProc != NULL)
    {
        UnhookWindowsHookEx(hookCallWndProc);
    }
    hookCallWndProc = NULL;
}

static LRESULT CALLBACK CallWndProcHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0)
    {
        auto pCwpStruct = (CWPSTRUCT*)lParam;
        switch (pCwpStruct->message)
        {
            case WM_SYSCOMMAND:
            {
                SendNotifyMessage(hwndMain, WM_SSM_HOOK_CALLWNDPROC_SYSCOMMAND, (WPARAM)pCwpStruct->hwnd, pCwpStruct->message);
                SendNotifyMessage(hwndMain, WM_SSM_HOOK_CALLWNDPROC_SYSCOMMAND_PARAMS, pCwpStruct->wParam, pCwpStruct->lParam);
            } break;

            case WM_INITMENUPOPUP:
            {
                SendNotifyMessage(hwndMain, WM_SSM_HOOK_CALLWNDPROC_INITMENU, (WPARAM)pCwpStruct->hwnd, pCwpStruct->message);
            } break;

            case WM_GETMINMAXINFO:
            {
                auto systemMenu = GetSystemMenu(pCwpStruct->hwnd, false);
                if (systemMenu)
                {
                    auto menuItemRollUpState = GetMenuState(systemMenu, SC_ROLLUP, MF_BYCOMMAND);
                    auto menuItemResizableState = GetMenuState(systemMenu, SC_RESIZABLE, MF_BYCOMMAND);
                    auto isMenuItemRollUpChecked = menuItemRollUpState != -1 && (menuItemRollUpState & MF_CHECKED) != 0;
                    auto isMenuItemResizableChecked = menuItemResizableState != -1 && (menuItemResizableState & MF_CHECKED) != 0;
                    if (isMenuItemRollUpChecked || isMenuItemResizableChecked)
                    {
                        LONG_PTR proc = SetWindowLongPtr(pCwpStruct->hwnd, GWLP_WNDPROC, (LONG_PTR)WndProcMinMax);
                        SetProp(pCwpStruct->hwnd, L"_ssm_old_wnd_proc_", (HANDLE)(proc));
                    }
                }
            } break;
        }
    }

    return CallNextHookEx(hookCallWndProc, nCode, wParam, lParam);
}

DLLEXPORT bool __stdcall InitializeGetMsgHook(int threadID, HWND destination)
{
    if (g_appInstance == NULL)
    {
        return false;
    }

    hwndMain = destination;
    hookGetMsg = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)GetMsgHookCallback, g_appInstance, threadID);
    return hookGetMsg != NULL;
}

DLLEXPORT void __stdcall UninitializeGetMsgHook()
{
    if (hookGetMsg != NULL)
    {
        UnhookWindowsHookEx(hookGetMsg);
    }
    hookGetMsg = NULL;
}

static LRESULT CALLBACK GetMsgHookCallback(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && wParam == PM_REMOVE)
    {
        auto pMsg = (MSG*)lParam;
        switch (pMsg->message)
        {
            case WM_SYSCOMMAND:
            {
                SendNotifyMessage(hwndMain, WM_SSM_HOOK_GETMSG_SYSCOMMAND, (WPARAM)pMsg->hwnd, pMsg->message);
                SendNotifyMessage(hwndMain, WM_SSM_HOOK_GETMSG_SYSCOMMAND_PARAMS, pMsg->wParam, pMsg->lParam);
            } break;

            case WM_INITMENUPOPUP:
            {
                SendNotifyMessage(hwndMain, WM_SSM_HOOK_GETMSG_INITMENU, (WPARAM)pMsg->hwnd, pMsg->message);
            } break;

            case WM_LBUTTONDOWN:
            {
                auto hwnd = GetTopLevelWindow(pMsg->hwnd);
                auto systemMenu = GetSystemMenu(hwnd, false);
                if (systemMenu)
                {
                    auto menuItemDragByMouseState = GetMenuState(systemMenu, SC_DRAG_BY_MOUSE, MF_BYCOMMAND);
                    auto isMenuItemDragByMouseChecked = menuItemDragByMouseState != -1 && (menuItemDragByMouseState & MF_CHECKED) != 0;
                    if (isMenuItemDragByMouseChecked)
                    {
                        ReleaseCapture();
                        SendMessage(hwnd, WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
                    }
                }
            } break;
        }
    }

    return CallNextHookEx(hookGetMsg, nCode, wParam, lParam);
}

static LRESULT CALLBACK WndProcMinMax(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HANDLE hdl = GetProp(hWnd, L"_ssm_old_wnd_proc_");
    RemoveProp(hWnd, L"_ssm_old_wnd_proc_");
    SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)(hdl));

    switch (uMsg)
    {
        case WM_GETMINMAXINFO:
        {
            auto minmax = (MINMAXINFO*)(lParam);
            minmax->ptMinTrackSize.x = 0;
            minmax->ptMinTrackSize.y = 0;
            minmax->ptMaxTrackSize.x = LONG_MAX;
            minmax->ptMaxTrackSize.y = LONG_MAX;
            /*TCHAR buf[255];
            wsprintf(buf, L"MINMAXINFO Hwnd = %p, x = %ld, y = %ld", hwnd, minmax->ptMinTrackSize.x, minmax->ptMinTrackSize.y);
            OutputDebugString(buf);*/
        } break;

        case WM_WINDOWPOSCHANGING:
        {
            auto wpos = (WINDOWPOS*)(lParam);
            wpos->cx = 0;
            wpos->cy = 0;
        } break;

        case WM_WINDOWPOSCHANGED:
            break;

        default:
            return CallWindowProc((WNDPROC)(hdl), hWnd, uMsg, wParam, lParam);
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}