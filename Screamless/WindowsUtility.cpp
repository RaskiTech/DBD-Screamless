#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <tchar.h>
#include "WindowsUtility.h"

void ChangeToSTDColorRed()
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 4);
}
void ChangeToSTDColorDefault()
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7);
}

void ParentProcessFunction()
{
    /*
    From microsoft.com
	The system restricts which processes can set the foreground window. A process can set the foreground window by calling SetForegroundWindow only if:
	All of the following conditions are true:
		The calling process belongs to a desktop application, not a UWP app or a Windows Store app designed for Windows 8 or 8.1.
		The foreground process has not disabled calls to SetForegroundWindow by a previous call to the LockSetForegroundWindow function.
		The foreground lock time-out has expired (see SPI_GETFOREGROUNDLOCKTIMEOUT in SystemParametersInfo).
		No menus are active.
	Additionally, at least one of the following conditions is true:
		The calling process is the foreground process.
		The calling process was started by the foreground process.
		There is currently no foreground window, and thus no foreground process.
		The calling process received the last input event.
		Either the foreground process or the calling process is being debugged.

		So we start our own child process and start debugging it and thus the child (the actual process) gains access to all the windows in this system
    */

    PROCESS_INFORMATION pi = LaunchSameProgramAsChildAndStartDebugging();

	// Thanks for https://www.codeproject.com/Articles/43682/Writing-a-basic-Windows-debugger for reference on how to code a debugger

    DEBUG_EVENT event = { 0 };
	bool continueDebugging = true;
    while(continueDebugging)
    {
        WaitForDebugEvent(&event, INFINITE);

		switch (event.dwDebugEventCode)
		{
			case EXIT_PROCESS_DEBUG_EVENT:
			{
				std::cout << "Got exit thread event" << std::endl;
				continueDebugging = false;
				break;
			}
			case OUTPUT_DEBUG_STRING_EVENT:
			{
			    OUTPUT_DEBUG_STRING_INFO & DebugString = event.u.DebugString;

			    char* msg = new char[DebugString.nDebugStringLength];
			    // Don't care if string is ANSI, and we allocate double...

			    ReadProcessMemory(pi.hProcess,       // HANDLE to Debuggee
					 DebugString.lpDebugStringData, // Target process' valid pointer
					 msg,                           // Copy to this address space
					 DebugString.nDebugStringLength, NULL);

				if (!strcmp(msg, "FreeConsole"))
					FreeConsole();

			    delete []msg;
			    // Utilize strEventMessage
			}
		}

        ContinueDebugEvent(event.dwProcessId, event.dwThreadId, DBG_CONTINUE);
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
