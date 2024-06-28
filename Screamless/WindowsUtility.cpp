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

void ChildProcessFunction()
{
    // We will be debugging this child process, so the execution will stop.
    // We stall a bit to make sure the debugger gets enough time to connect.

    for (int i = 0; i < 10; i++)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "Child process exiting." << std::endl;
}
