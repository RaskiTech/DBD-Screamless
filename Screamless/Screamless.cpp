#include <iostream>
#include "Application.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

bool shouldLog = true;

// In debug mode we don't want to start another process (visual studio grants us the rights to set windows anyway)
#ifdef _DEBUG
	#define START_PROCESS_AS_DEBUGGEE 0
#else
	#define START_PROCESS_AS_DEBUGGEE 1
#endif

// TODO:
// Check against eyrie of crows
// VisualizeLookingPositions more self-explanatory

extern void ParentProcessFunction();


int main(int argc, char *argv[]) {
#if START_PROCESS_AS_DEBUGGEE
    // Firs time we start this program we'll go into this function
    if (!(argc > 1 && _stricmp(argv[1], "child") == 0)) {
        ParentProcessFunction();
        return 0;
    }
#endif

    AppSettings settings = ReadAppSettingsFromFile("Settings.txt");

    //Application app = Application(settings, "ApplicationFrameHost.exe", "Media Player");
    Application app = Application(settings, "DeadByDaylight-Win64-Shipping.exe");

    if (app.FindApplication())
    {
		if (!settings.ShowConsole)
		{
			FreeConsole();
            OutputDebugString((LPCWSTR)L"FreeConsole");
			shouldLog = false;
		}

		app.Run();
    }
}
