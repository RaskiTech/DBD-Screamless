#pragma once

#include "WindowsFocusController.h"
#include "VolumeController.h"
#include "DisplayManager.h"
#include "ImageHandler.h"
#include "Utility.h"
#include <chrono>
#include <thread>


class Application
{
public:
	Application(AppSettings settings, const std::string& executableName, const std::string& appName = "") : mSettings(settings), mAppName(appName), mExecutableName(executableName) {};
	~Application();

	enum SurvivorState
	{
		FreeOrOnHook = 0,
		Carrying = 1,
		CurrentlyHooking = 2,
	};

	bool FindApplication();
	bool TryInitialize();
	bool LoadAndResizeImages();
	bool LoadAndResizeImage(int* image, const std::string& path, const BoundingBox& neededBox);
	void Run();

	void DoSurvivorStateChecks();
	SurvivorState UpdateSurvivorState(int hudIndex, SurvivorState currentState);
	void UpdateIsInKillerGame();
	void MuteScreamNow();
	inline void WaitMilliseconds(int milliseconds) { std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds)); }
	inline void WaitSeconds(float seconds) { std::this_thread::sleep_for(std::chrono::milliseconds((int)(seconds * 1000))); }


	void DrawImportantScreenPositions();
	BoundingBox GetSurvivorHudPosition(int hudIndex);
	std::pair<BoundingBox, BoundingBox> GetSurvivorEffectParticlePositions(int hudIndex);
	BoundingBox GetKillerUIHookPosition();


private:

	std::string mAppName; // The name of the app as it shows in taskbar
	std::string mExecutableName; // The process name found in task manager when clicking go to details on the process
	void* mWindowHandle = nullptr;
	uint32_t mChildProcessID = 0;
	bool mFoundApplication = false;

	int mCarryImage = -1;
	int mHookImage = -1;
	int mBloodEffectImageLeft = -1;
	int mBloodEffectImageRight = -1;

	bool mInKillerGame = false;
	int mCarryIndex = -1;
	SurvivorState survivorStates[4] = {};

	int mLastCheckedHudIndex = 0;

	FocusController mFocusController;
	DisplayManager mDisplayManager;
	ImageHandler mImageHandler;

	AppSettings mSettings;
};
