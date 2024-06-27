#include <iostream>
#include "Application.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

bool shouldLog = true;

int main() {
    AppSettings settings = ReadAppSettingsFromFile("Settings.txt");

    //Application app = Application(settings, "ApplicationFrameHost.exe", "Media Player");
    Application app = Application(settings, "DeadByDaylight-Win64-Shipping.exe");

    app.FindApplication();

    if (!settings.ShowConsole)
    {
        FreeConsole();
        shouldLog = false;
    }

    app.Run();

}
