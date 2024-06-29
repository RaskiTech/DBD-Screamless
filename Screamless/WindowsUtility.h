#pragma once
#include "Utility.h"
#include <string>
#include <iostream>
#include <tlhelp32.h>
#include <windows.h>
#include <tchar.h>

//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
static std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0) {
        return "No information provided."; //No error message has been recorded
    }
    
    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    
    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);
    
    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);
            
    return message;
}


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
        LogErr("Failed to create snapshot");
        return 0;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    // Retrieve information about the first process
    if (!Process32First(hSnapshot, &pe32)) {
        LogErr("Failed to retrieve first process");
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

static PROCESS_INFORMATION LaunchSameProgramAsChildAndStartDebugging()
{
    // Get the path to the current executable
    TCHAR szPath[MAX_PATH];
    if (!GetModuleFileName(NULL, szPath, MAX_PATH))
    {
        LogErr("Failed to get current executable path. Error: ", GetLastError());
        return {};
    }

    // Prepare the command line for the new process
    TCHAR szCommandLine[MAX_PATH];
    _stprintf_s(szCommandLine, _T("\"%s\" child"), szPath);
    
    // Initialize structures for process creation
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    // Create the new process
    if (!CreateProcess(NULL, szCommandLine, NULL, NULL, FALSE, DEBUG_ONLY_THIS_PROCESS, NULL, NULL, &si, &pi))
    {
        LogErr("Failed to create process. Error: ", GetLastErrorAsString());
        return {};
    }

    // Wait for the process to become available
    WaitForInputIdle(pi.hProcess, INFINITE);

    return pi;
}
static void StopChildProcess(int childProcessHandle)
{
    if (!DebugActiveProcessStop(childProcessHandle))
        LogErr("Couldn't stop debugging. You may need to close the program window manually: ", GetLastError());
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

