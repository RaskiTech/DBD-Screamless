#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <ostream>
#include <fstream>

// Every single one of these values should be 4 bytes
struct AppSettings
{
	float GameHUDScale = 1.0f;
	int   ManualMuteKeybindKeycode = 0x00;

	float ShowConsole = 1;
	float VisualizeLookingPositions = false;

	float KillerUIHookTreshold = 0.09f;
	float CarryImageTreshold = 0.08f;
	float ParticleEffectLeftTreshold = 0.03f;
	float ParticleEffectRightTreshold = 0.03f;

	float ApplicationSearchWaitTime = 2000;
	float SearchAllSurvivorsWaitTime = 3000 / 4;
	float SearchForSurvivorHookingWaitTime = 50;
	float HookingWaitTime = 500;
	float OnHookingTabOutTime = 130;
};
struct Image
{
	int width = 0;
	int height = 0;
	std::vector<uint8_t> buffer;
	
	bool ContainsImage() { return !(width == 0 || height == 0); }
};
struct BoundingBox
{
	BoundingBox(int x, int y, int width, int height) : x(x), y(y), width(width), height(height) {};
	BoundingBox() {};

	int x = 0;
	int y = 0;
	int width = 100;
	int height = 100;
};

extern void ChangeToSTDColorRed();
extern void ChangeToSTDColorDefault();

extern bool shouldLog;
template<typename... Args>
void Log(Args... args)
{
	if (!shouldLog)
		return;
	std::ostringstream oss;
	(oss << ... << args);
	std::cout << oss.str() << std::endl;
}
template<typename... Args>
void LogErr(Args... args)
{
	if (!shouldLog)
		return;
	std::ostringstream oss;
	(oss << ... << args);
	ChangeToSTDColorRed();
	std::cout << oss.str() << std::endl;
	ChangeToSTDColorDefault();
}

static AppSettings ReadAppSettingsFromFile(const std::string& path)
{
    std::ifstream file(path);
    std::string line;

	if (!file.is_open())
	{
		std::cout << "Couldn't find settings file: " << path << std::endl;
		return AppSettings();
	}

	AppSettings settings;
    
	int settingCount = 0;
    while (std::getline(file, line)) {

        // Ignore everything after the hashtag
        std::string::size_type hashPos = line.find('#');
        if (hashPos != std::string::npos) {
            line = line.substr(0, hashPos);
        }
        
        // Trim whitespace
        line.erase(line.find_last_not_of(" \t\n\r\f\v") + 1);

        if (line.empty())
            continue;

		if (sizeof(float) * settingCount >= sizeof(AppSettings))
		{
			std::cout << "Found more settings even after parsing all. First unparsed line: " << line << std::endl;
			break;
		}

		std::istringstream iss(line);
		float floatValue;
		iss >> floatValue;
		if (iss.fail()) {
			std::cerr << "Error reading settings: Failed to read value from line: " << line << std::endl;
			continue;
		}

		memcpy_s(((float*)&settings) + settingCount, sizeof(float), &floatValue, sizeof(float));
		settingCount++;
    }

	// Everything is read in as a float. Transform ints into ints

	settings.ManualMuteKeybindKeycode = (int)*(float*)&settings.ManualMuteKeybindKeycode;

    return settings;
}
