#include <iostream>
#include "Application.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

bool shouldLog = true;


// TODO:
// Check against eyrie of crows
// VisualizeLookingPositions more self-explanatory

extern void ChildProcessFunction();


int main(int argc, char *argv[]) {
    if (argc > 1 && _stricmp(argv[1], "child") == 0) {
        ChildProcessFunction();
        return 0;
    }

    AppSettings settings = ReadAppSettingsFromFile("Settings.txt");

    //Application app = Application(settings, "ApplicationFrameHost.exe", "Media Player");
    Application app = Application(settings, "DeadByDaylight-Win64-Shipping.exe");

    if (app.FindApplication())
    {
		if (!settings.ShowConsole)
		{
			FreeConsole();
			shouldLog = false;
		}

		app.Run();
    }
}
