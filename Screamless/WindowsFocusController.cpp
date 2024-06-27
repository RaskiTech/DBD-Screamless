#include <iostream>
#include <audiopolicy.h>
#include <tlhelp32.h>
#include <functional>
#include <windows.h>
#include "WindowsFocusController.h"
#include "WindowsUtility.h"


void FocusController::Initialize(void* windowHandle)
{
    mWindowHandle = *(HWND*)&windowHandle;
}

void FocusController::Uninitialize()
{
    mPid = 0;
}

void FocusController::LoseFocus()
{
    HWND desktop = GetDesktopWindow();
    if (desktop)
    {
        SetForegroundWindow(desktop);
    }
    else
    {
        Log("Didn't get the desktop window");
    }
}


bool FocusController::IsWindowFocused()
{
    HWND handle = GetForegroundWindow();
    // std::cout << "Currently focused: " << handle << std::endl;
    return handle == mWindowHandle;
}
void FocusController::TakeFocus()
{
    if (mWindowHandle == NULL || !IsWindow(mWindowHandle))
    {
        mWindowHandle = NULL;
        return;
    }
    BOOL success = SetForegroundWindow(mWindowHandle);
}

