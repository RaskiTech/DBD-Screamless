#include <iostream>
#include <audiopolicy.h>
#include <tlhelp32.h>
#include <functional>
#include <windows.h>
#include "WindowsFocusController.h"
#include "WindowsUtility.h"
#include <thread>


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
    
    if (!desktop)
    {
        LogErr("Didn't get the desktop window");
        return;
    }

	SetLastError(0);
	bool success = SetForegroundWindow(desktop);
	if (!success)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		success = SetForegroundWindow(desktop);
	}

	if (!success)
		LogErr("Couldn't set desktop to foreground window: ", GetLastErrorAsString());
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
        LogErr("Can't take focus, we don't have a handle to the window.");
        return;
    }

	SetLastError(0);
    bool success = SetForegroundWindow(mWindowHandle);
	if (!success)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		success = SetForegroundWindow(mWindowHandle);
	}

    if (!success)
        LogErr("Couldn't set game window to foreground: ", GetLastErrorAsString());
}

