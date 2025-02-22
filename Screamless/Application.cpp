#include "Application.h"
#include "WindowsUtility.h"
#include "ImageHandler.h"
#include "stb_image.h"
#include <iostream>
#include <thread>

bool Application::TryInitialize()
{
    mWindowHandle = GetWindowHandle(mExecutableName, mAppName);

    if (!IsWindow(*(HWND*)&mWindowHandle))
    {
        return false;
    }

    mFocusController = FocusController();
    mFocusController.Initialize(mWindowHandle);

    mDisplayManager = DisplayManager();
    mDisplayManager.Initialize(mWindowHandle);

    if (mDisplayManager.GetWindowSize()[0] == 0)
    {
        // Window not yet open
        mFocusController.Uninitialize();
        mDisplayManager.Uninitialize();
        return false;
    }

    bool success = LoadAndResizeImages();
	if (!success)
	{
		mImageHandler.DeleteImage(mCarryImage);
		mImageHandler.DeleteImage(mBloodEffectImageLeft);
		mImageHandler.DeleteImage(mBloodEffectImageRight);
		mFocusController.Uninitialize();
		mDisplayManager.Uninitialize();
		return false;
	}

    mFoundApplication = true;

    if (mAppName != "")
		Log("Started Screamless on application ", mAppName,        mSettings.VisualizeLookingPositions ? " with visualising looking positions." : ".", " Hud scale: ", mSettings.GameHUDScale);
    else 
		Log("Started Screamless on application ", mExecutableName, mSettings.VisualizeLookingPositions ? " with visualising looking positions." : ".", " Hud scale: ", mSettings.GameHUDScale);

    return true;
}

bool Application::LoadAndResizeImages()
{
	bool success = true;

    success &= LoadAndResizeImage(&mCarryImage, "Textures/CarryDark.png", GetSurvivorHudPosition(0));
    success &= LoadAndResizeImage(&mHookImage, "Textures/Hook.png", GetKillerUIHookPosition());
    success &= LoadAndResizeImage(&mProgressBarImage, "Textures/ProgressBar.png", GetProgressBarPosition());
    auto [left, right] = GetSurvivorEffectParticlePositions(0);
    success &= LoadAndResizeImage(&mBloodEffectImageLeft, "Textures/ParticleEffectLeft.png", left);
    success &= LoadAndResizeImage(&mBloodEffectImageRight, "Textures/ParticleEffectRight.png", right);

    return success;
}

bool Application::LoadAndResizeImage(int* image, const std::string& path, const BoundingBox& box)
{
    if (*image == -1 || mImageHandler.GetImage(*image).width != box.width || mImageHandler.GetImage(*image).height != box.height)
    {
		if (*image == -1)
			*image = mImageHandler.GetFreeImageHandle();
		else
			mImageHandler.DeleteImage(*image);

		mImageHandler.LoadImageFromDisk(path,  *image);
        return mImageHandler.ResizeImage(*image, box.width, box.height);
    }
    return true;
}

void Application::DrawImportantScreenPositions()
{
	// Middle screen
	BoundingBox loadLocation = GetKillerUIHookPosition();
	int correctImage = mHookImage;
	const char* pathToBigImg = "Textures/Hook.png";

    if (!(GetKeyState(VK_LSHIFT) & 0x8000))
    {
        Image& img = mImageHandler.GetFreeImage();
        mDisplayManager.LoadImageFromDesktopView(loadLocation, img);
        mImageHandler.ResizeImage(img, 500, 500 * img.height / img.width);
        mDisplayManager.DrawImage((mDisplayManager.GetWindowSize()[0] - img.width) / 2, (mDisplayManager.GetWindowSize()[1] - img.height) / 2, img);
        mImageHandler.DeleteImage(img);

        WaitMilliseconds(200);
        mDisplayManager.UpdateLoadedWindow();
        return;
    }
    else
    {
		Image& img = mImageHandler.GetFreeImage();
		mDisplayManager.LoadImageFromDesktopView(loadLocation, img);
		std::cout << "Middle screen graphic difference: " << mImageHandler.CalculateImageDifference(img, mImageHandler.GetImage(correctImage), ImageHandler::CompareBlueChannel | ImageHandler::CompareGreenChannel) << std::endl;
		mImageHandler.DeleteImage(img);

		auto [x, y] = mDisplayManager.GetWindowSize();
		Image& carryBig = mImageHandler.LoadImageFromDisk(pathToBigImg);
		mImageHandler.ResizeImage(carryBig, 500, 500 * img.height / img.width);
		mDisplayManager.DrawImage((x - carryBig.width) / 2, (y - carryBig.height) / 2, carryBig);
		mImageHandler.DeleteImage(carryBig);
    }

    // Survivor positiions
    int surv = 0;
    int draws[] = { 0, 3 };
    for (int i : draws)
    {
		BoundingBox box = GetSurvivorHudPosition(i);
		auto [left, right] = GetSurvivorEffectParticlePositions(i);
		mDisplayManager.DrawImage(box.x, box.y, mImageHandler.GetImage(mCarryImage));
		mDisplayManager.DrawImage(left.x, left.y, mImageHandler.GetImage(mBloodEffectImageLeft));
		mDisplayManager.DrawImage(right.x, right.y, mImageHandler.GetImage(mBloodEffectImageRight));
    }

	auto [leftFrom, rightFrom] = GetSurvivorEffectParticlePositions(1);
	auto [leftTo, rightTo] = GetSurvivorEffectParticlePositions(2);
    Image& img = mImageHandler.GetFreeImage();
    mDisplayManager.LoadImageFromDesktopView(leftFrom, img);
    mDisplayManager.DrawImage(leftTo.x, leftTo.y, img);
    mImageHandler.DeleteImage(img);

    Image& img2 = mImageHandler.GetFreeImage();
    mDisplayManager.LoadImageFromDesktopView(rightFrom, img2);
    mDisplayManager.DrawImage(rightTo.x, rightTo.y, img2);
    mImageHandler.DeleteImage(img2);

    // Hook
    BoundingBox box = GetKillerUIHookPosition();
    mDisplayManager.DrawImage(box.x, box.y, mImageHandler.GetImage(mHookImage));
}


Application::~Application()
{
    mFocusController.Uninitialize();
    mDisplayManager.Uninitialize();
}

bool Application::FindApplication()
{
    int tries = 0;
    while (tries < 30)
    {
        const char* name = mAppName == "" ? mExecutableName.c_str() : mAppName.c_str();
        Log("Finding application ", name);

        mFoundApplication = TryInitialize();
        if (mFoundApplication)
            break;

        WaitSeconds(mSettings.ApplicationSearchWaitTime);
        tries++;
    }
    return mFoundApplication;
}

void Application::Run()
{
    while (mFoundApplication)
    {
        if (mFocusController.IsWindowFocused())
        {
            if (GetKeyState(mSettings.ManualMuteKeybindKeycode) & 0x8000)
            {
                MuteScreamNow();
            }

            if (mSettings.VisualizeLookingPositions)
            {
				DrawImportantScreenPositions();
				WaitSeconds(0.1f);

                LoadAndResizeImages();
                continue;
            }

            UpdateIsInKillerGame();
            if (!mInKillerGame)
            {
                WaitSeconds(mSettings.ApplicationSearchWaitTime);
                continue;
            }

			DoSurvivorStateChecks();

        }
        else
        {
            if (!IsWindow(*(HWND*)&mWindowHandle))
            {
                // The program was closed, also close our program
                mFoundApplication = false;
                continue;
            }

            WaitSeconds(mSettings.ApplicationSearchWaitTime);
        }
    }
}

BoundingBox Application::GetSurvivorHudPosition(int hudIndex)
{
    auto [width, height] = mDisplayManager.GetWindowSize();
    auto [xOffset, yOffset] = mDisplayManager.GetWindowSmallOffset();

    // All of these values are hard coded by looking at the screen

    return BoundingBox(
    /* x */         (int)(width * 0.05635f * mSettings.GameHUDScale) + xOffset,
    /* y */	        (int)(height * (1 - (1 - (0.648f - hudIndex * 0.0814f)) * mSettings.GameHUDScale)) + yOffset,
    /* Width */	    (int)(width * 0.0225f * mSettings.GameHUDScale),
    /* Height */	(int)(width * 0.0225f * mSettings.GameHUDScale)
    );
}

std::pair<BoundingBox, BoundingBox> Application::GetSurvivorEffectParticlePositions(int hudIndex)
{
    BoundingBox survBox = GetSurvivorHudPosition(hudIndex);

    return std::make_pair<BoundingBox, BoundingBox>(

        // Left one
        BoundingBox(
        /* x */        (int)(survBox.x - survBox.width * 0.75f),
        /* y */	       (int)(survBox.y + survBox.height * 0.6f),
        /* Width */	   (int)(survBox.width * 0.86f),
        /* Height */   (int)(survBox.width * 0.86f)
        ),

        // Right one
        BoundingBox(
        /* x */        (int)(survBox.x + survBox.width * 0.83f),
        /* y */	       (int)(survBox.y - survBox.height * 0.7f),
        /* Width */	   (int)(survBox.width * 1.45f),
        /* Height */   (int)(survBox.width * 1.45f)
        )

    );
}

BoundingBox Application::GetProgressBarPosition()
{
    auto [width, height] = mDisplayManager.GetWindowSize();
    auto [xOffset, yOffset] = mDisplayManager.GetWindowSmallOffset();

    const int imageDimensionX = 174;
    const int imageDimensionY = 16;
    const float scale = 0.0005f;

    return BoundingBox(
    /* x */         (int)(width  * (0.5f - 0.064f * mSettings.GameHUDScale)) + xOffset,
    /* y */	        (int)(height * (1 - 0.2f * mSettings.GameHUDScale)) + yOffset,
    /* Width */	    (int)(width * scale * imageDimensionX * mSettings.GameHUDScale),
    /* Height */	(int)(width * scale * imageDimensionY * mSettings.GameHUDScale)
    );
}

BoundingBox Application::GetKillerUIHookPosition()
{
    auto [width, height] = mDisplayManager.GetWindowSize();
    auto [xOffset, yOffset] = mDisplayManager.GetWindowSmallOffset();

    return BoundingBox(
    /* x */         (int)(width * 0.0559f * mSettings.GameHUDScale) + xOffset,
    /* y */	        (int)(height * (1 - (1 - 0.748f) * mSettings.GameHUDScale)) + yOffset,
    /* Width */	    (int)(width * 0.0213f * mSettings.GameHUDScale),
    /* Height */	(int)(width * 0.0213f * mSettings.GameHUDScale)
    );
}


void Application::DoSurvivorStateChecks()
{
	if (mCarryIndex != -1)
	{
		// We ignore everyone else and focus on this one being carried, because we want to catch the hooking as early as possible.
		// This needs to be reconsidered if 2 killers are needed

		SurvivorState newState = UpdateSurvivorState(mCarryIndex, SurvivorState::Carrying);

		switch (newState)
		{
		case Application::FreeOrOnHook:
			survivorStates[mCarryImage] = SurvivorState::FreeOrOnHook;
			mCarryIndex = -1;
			break;
		case Application::CurrentlyHooking:
			survivorStates[mCarryIndex] = SurvivorState::FreeOrOnHook;
			mCarryIndex = -1;
			MuteScreamNow();
            WaitSeconds(mSettings.HookingWaitTime);
			break;
		case Application::Carrying:
			break;
		default:
			break;
		}

		WaitSeconds(mSettings.SearchForSurvivorHookingWaitTime);
	}
	else
	{
		// If no one is being carried, we repeatedly go through the survivors one by one and check if someone is being picked up

		mLastCheckedHudIndex = (mLastCheckedHudIndex + 1) % 4;
		survivorStates[mLastCheckedHudIndex] = UpdateSurvivorState(mLastCheckedHudIndex, SurvivorState::FreeOrOnHook);
		if (survivorStates[mLastCheckedHudIndex] == SurvivorState::Carrying)
		{
			mCarryIndex = mLastCheckedHudIndex;
			Log("Started carrying index ", mLastCheckedHudIndex);
			WaitSeconds(mSettings.SearchForSurvivorHookingWaitTime);
		}
		else
		{
			WaitSeconds(mSettings.SearchAllSurvivorsWaitTime);
		}
	}
}


void Application::MuteScreamNow()
{
	mFocusController.LoseFocus();
    WaitSeconds(mSettings.OnHookingTabOutTime);
	mFocusController.TakeFocus();
}

Application::SurvivorState Application::UpdateSurvivorState(int hudIndex, SurvivorState currentState)
{
    switch (currentState)
    {
    case Application::FreeOrOnHook:
    {
        // Check for carry

        Image& screenImage = mImageHandler.GetFreeImage();
        BoundingBox box = GetSurvivorHudPosition(hudIndex);
        mDisplayManager.LoadImageFromDesktopView(box, screenImage);
        float difference = mImageHandler.CalculateImageDifference(
            mImageHandler.GetImage(mCarryImage), screenImage, ImageHandler::CompareGreenChannel | ImageHandler::CompareBlueChannel
        );
        mImageHandler.DeleteImage(screenImage);

        if (difference == -1)
            LoadAndResizeImages();

		static float maxCarry = 0;

        if (difference < mSettings.CarryImageTreshold)
        {
            if (difference == -1)
            {
                mImageHandler.ResizeImage(mCarryImage, box.width, box.height);
                return SurvivorState::FreeOrOnHook;
            }

            Log("Started carrying with certainty (lower, more certain) ", difference);
            maxCarry = 0;
            return SurvivorState::Carrying;
        }
        else
        {
            Log("Survivor ", hudIndex, " was alive with certainty ", difference);

            return SurvivorState::FreeOrOnHook;
        }

        break;
    }
    case Application::Carrying:
    {
        // Check for hook or a save
        // The hook image is not used, instead we check for the carry image

        Image& screenImage = mImageHandler.GetFreeImage();
        BoundingBox box = GetSurvivorHudPosition(hudIndex);
        mDisplayManager.LoadImageFromDesktopView(box, screenImage);
        float carryDifference = mImageHandler.CalculateImageDifference(
            mImageHandler.GetImage(mCarryImage), screenImage, ImageHandler::CompareBlueChannel | ImageHandler::CompareGreenChannel
        );
        mImageHandler.DeleteImage(screenImage);

        // In order for them to get the save we check multiple times and only register if all of them succeed
        // This is to avoid false positives in the most critical moments
        static int gotSaveResultsInRow = 0;
        if (carryDifference >= mSettings.CarryImageTreshold)
        {
            if (gotSaveResultsInRow > 1)
            {
                Log("Got the save with certainty ", carryDifference);
                return SurvivorState::FreeOrOnHook;
            }
            else
            {
                gotSaveResultsInRow++;
            }
        }
        else
        {
            gotSaveResultsInRow = 0;
        }

        BoundingBox progressBox = GetProgressBarPosition();
		if (carryDifference == -1)
		{
			mImageHandler.ResizeImage(mCarryImage, box.width, box.height);
            mImageHandler.ResizeImage(mProgressBarImage, progressBox.width, progressBox.height);
			return SurvivorState::Carrying;
		}

        Image& freeImg = mImageHandler.GetFreeImage();
        mDisplayManager.LoadImageFromDesktopView(progressBox, freeImg);
        float progressDifference = mImageHandler.CalculateImageDifference(
            mImageHandler.GetImage(mProgressBarImage), freeImg, ImageHandler::CompareBlueChannel | ImageHandler::CompareGreenChannel | ImageHandler::CompareRedChannel
        );
        mImageHandler.DeleteImage(freeImg);

        static float minDiff = 1.0f;
        if (progressDifference < minDiff)
        {
            minDiff = progressDifference;
            Log("ProgressBarDifference: ", minDiff);
        }

        if (progressDifference < mSettings.ProgressBarTreshold)
        {
            Log("Survivor hooked with certanty (lower, more certain) ", progressDifference);
            minDiff = 1.0f;
            return SurvivorState::CurrentlyHooking;
        }
        else
            return SurvivorState::Carrying;

        break;
    }
    case Application::CurrentlyHooking:
    {
        return Application::FreeOrOnHook;
    }
    default:
        Log("UpdateSurvivorState switch default hit.");
        break;
    }

    return SurvivorState::FreeOrOnHook;
}

bool Application::CheckForBloodParticleEffects(int hudIndex)
{
	auto [left, right] = GetSurvivorEffectParticlePositions(hudIndex);
    if (left.width != mImageHandler.GetImage(mBloodEffectImageLeft).width)
    {
        LoadAndResizeImages();
    }

    Image& screenImage = mImageHandler.GetFreeImage();
	mDisplayManager.LoadImageFromDesktopView(left, screenImage);
	float leftDifference = mImageHandler.CalculateRedPrecense(mImageHandler.GetImage(mBloodEffectImageLeft), screenImage);
	mImageHandler.DeleteImage(screenImage);
	mDisplayManager.LoadImageFromDesktopView(right, screenImage);
	float rightDifference = mImageHandler.CalculateRedPrecense(mImageHandler.GetImage(mBloodEffectImageRight), screenImage);
	mImageHandler.DeleteImage(screenImage);
	
	// If either one of them got under the treshold, we'll go through with it
	// since something was probably just interfering with the other one
	return leftDifference > mSettings.ParticleEffectLeftTreshold || rightDifference > mSettings.ParticleEffectRightTreshold;
}

void Application::UpdateIsInKillerGame()
{
    // We don't need to check this every single time this function is called, but once in a while.
    // If it seems like we have switched, check again after a short period of time to confirm.

    static std::chrono::seconds secondsUntilNextCheck = std::chrono::seconds(0);
    static auto prevCheckTime = std::chrono::steady_clock::now() - std::chrono::seconds(1); // Initialize to lesser value so the function runs the first time

    auto now = std::chrono::steady_clock::now();
    if (std::chrono::duration_cast<std::chrono::seconds>(now - prevCheckTime) < std::chrono::seconds(secondsUntilNextCheck))
        return;

    prevCheckTime = std::chrono::steady_clock::now();

    static bool lastResult = mInKillerGame;

	Image& img = mImageHandler.GetFreeImage();
	mDisplayManager.LoadImageFromDesktopView(GetKillerUIHookPosition(), img);
	float diff = mImageHandler.CalculateImageDifference(mImageHandler.GetImage(mHookImage), 
		img, ImageHandler::CompareBlueChannel | ImageHandler::CompareGreenChannel);
    if (diff == -1)
    {
        LoadAndResizeImages();
		diff = mImageHandler.CalculateImageDifference(mImageHandler.GetImage(mHookImage), 
			img, ImageHandler::CompareBlueChannel | ImageHandler::CompareGreenChannel);
    }

	mImageHandler.DeleteImage(img);
	bool isInKillerGame = diff < mSettings.KillerUIHookTreshold;

	Log("Currently in killer game: ", mInKillerGame, ". Now got the result ", isInKillerGame, " with certanty ", diff);
    if (isInKillerGame == mInKillerGame)
    {
        secondsUntilNextCheck = std::chrono::seconds(15);
    }
    else
    {
        if (lastResult == isInKillerGame)
            mInKillerGame = isInKillerGame;
        secondsUntilNextCheck = std::chrono::seconds(5);
    }

	lastResult = isInKillerGame;
}



