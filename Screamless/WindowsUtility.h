#pragma once
#include "Utility.h"
#include <string>
#include <iostream>
#include <tlhelp32.h>
#include <windows.h>

static DWORD tempCorrectPID;
static BOOL CALLBACK EnumWindowsFindPID(HWND hwnd, LPARAM lParam) {
    if (hwnd != (HWND)lParam)
    {
        return TRUE; // Continue enumerating windows
    }

	DWORD processId;
	GetWindowThreadProcessId(hwnd, &processId);
    tempCorrectPID = processId;

	return FALSE;
}
static DWORD FindFocusedWindowPID()
{
	HWND handle = GetForegroundWindow();

	// Do a cashe test: Is it the same as last time
	DWORD casheTest;
	GetWindowThreadProcessId(handle, &casheTest);
	if (casheTest == tempCorrectPID)
		return casheTest;

	// If not, enumerate through all and find it
	EnumWindows(EnumWindowsFindPID, (LPARAM)handle);
    return tempCorrectPID;
}
static void FindFocusedWindowInfo(DWORD* pid, char* windowName256SizeArray, char* className256SizeArray)
{
    *pid = FindFocusedWindowPID();
    HWND handle = GetForegroundWindow();

	GetWindowTextA(handle, windowName256SizeArray, 256);
	GetClassNameA(handle, className256SizeArray, 256);
}

static DWORD FindPIDByName(const std::string& name)
{
    std::wstring wideName = std::wstring(name.begin(), name.end());

    // Take a snapshot of all processes in the system
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        Log("Failed to create snapshot");
        return 0;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process
    if (!Process32First(hSnapshot, &pe32)) {
        Log("Failed to retrieve first process");
        CloseHandle(hSnapshot);
        return 0;
    }

    // Now walk the snapshot of processes
    do {
        if (!(bool)wcscmp(wideName.c_str(), (const wchar_t*)pe32.szExeFile)) {
            CloseHandle(hSnapshot);
            return pe32.th32ProcessID;
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return 0; // Process not found
}

static HWND _tempCorrectHandle;
static char _tempNeededWindowTitle[256];
static bool _tempPrintAllHandles = false;
static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
	DWORD processId;
	GetWindowThreadProcessId(hwnd, &processId);

	if (processId == (DWORD)lParam && IsWindow(hwnd)) {

		// Get window title
        char windowTitle[256];
        GetWindowTextA(hwnd, windowTitle, sizeof(windowTitle));

        // Get window class name
        char className[256];
        GetClassNameA(hwnd, className, sizeof(className));

        // Print out the window title and class name for debugging
        if (_tempPrintAllHandles)
			Log("Found window - Title: ", windowTitle, ", Class: ", className);

        bool doesExist = _tempNeededWindowTitle[0] != '\0';
        if (!doesExist || strcmp(windowTitle, _tempNeededWindowTitle) == 0)
        {
			_tempCorrectHandle = hwnd;
			return _tempPrintAllHandles; // Stop enumerating windows
        }
	}

	return TRUE; // Continue enumerating windows
}

static void* GetWindowHandle(const std::string& executableName, const std::string& appName)
{
    _tempPrintAllHandles = false;

    DWORD pid = FindPIDByName(executableName);

    strcpy_s<256>(_tempNeededWindowTitle, appName.c_str());
    EnumWindows(EnumWindowsProc, (LPARAM)pid);
    return *(void**)&_tempCorrectHandle;
}
static void PrintWindowHandles(const std::string& executableName)
{
    _tempPrintAllHandles = true;
    _tempNeededWindowTitle[0] = '\0';

    DWORD pid = FindPIDByName(executableName);
    EnumWindows(EnumWindowsProc, (LPARAM)pid);
}
