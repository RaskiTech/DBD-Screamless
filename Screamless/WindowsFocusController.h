#pragma once
#include <string>
#include <windows.h>


class FocusController
{
public:
	void Initialize(const std::string& appName);
	void Initialize(void* windowHandle);
	HWND GetHandle() { return mWindowHandle; }
	void Uninitialize();
	~FocusController() { Uninitialize(); }

	void LoseFocus();
	void TakeFocus();
	bool IsWindowFocused();

private:
	DWORD mPid = NULL;
	HWND mWindowHandle = NULL;
};
