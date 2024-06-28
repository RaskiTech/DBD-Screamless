#include "Utility.h"
#include "VolumeController.h"
#include "WindowsUtility.h"
#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <audiopolicy.h>


void VolumeController::Initialize(const std::string& appName)
{
    mPid = FindPIDByName(appName);

    mHr = CoInitialize(nullptr);
    if (FAILED(mHr)) {
        LogErr("Failed to initialize COM library");
        Uninitialize();
        return;
    }

    mHr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID*)&mDeviceEnumerator);
    if (FAILED(mHr)) {
        LogErr("Failed to create device enumerator");
		Uninitialize();
        return;
    }

    mHr = mDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &mDevice);
    if (FAILED(mHr)) {
		LogErr("Failed to get default audio endpoint");
        Uninitialize();
        return;
    }

    mHr = mDevice->Activate(__uuidof(IAudioSessionManager2), CLSCTX_INPROC_SERVER, nullptr, (LPVOID*)&mAudioSessionManager);
    if (FAILED(mHr)) {
        LogErr("Failed to get audio session manager interface");
        Uninitialize();
        return;
    }

    mHr = mAudioSessionManager->GetSessionEnumerator(&mAudioSessionEnumerator);
    if (FAILED(mHr)) {
		LogErr("Failed to get session enumerator");
        Uninitialize();
        return;
    }
}

void VolumeController::Uninitialize()
{
    if (mAudioSessionEnumerator != nullptr)
        mAudioSessionEnumerator->Release();
    if (mAudioSessionManager != nullptr)
        mAudioSessionManager->Release();
	if (mDevice != nullptr)
		mDevice->Release();
    if (mDeviceEnumerator != nullptr)
        mDeviceEnumerator->Release();

	CoUninitialize();

    mAudioSessionEnumerator = nullptr;
    mAudioSessionManager = nullptr;
	mDeviceEnumerator = nullptr;
	mDevice = nullptr;
    mPid = 0;
}

bool VolumeController::IsInitialized()
{
	return mDeviceEnumerator != nullptr && mDevice != nullptr && mAudioSessionEnumerator != nullptr && mAudioSessionManager != nullptr;
}

void VolumeController::Mute(bool mute)
{
    int sessionCount = 0;
    mHr = mAudioSessionEnumerator->GetCount(&sessionCount);
    if (FAILED(mHr)) {
        LogErr("Failed to get session count");
        Uninitialize();
        return;
    }

    for (int i = 0; i < sessionCount; ++i) {
        IAudioSessionControl* pSessionControl = nullptr;
        mHr= mAudioSessionEnumerator->GetSession(i, &pSessionControl);
        if (FAILED(mHr)) {
            LogErr("Failed to get session control");
            continue;
        }

        IAudioSessionControl2* pSessionControl2 = nullptr;
        mHr = pSessionControl->QueryInterface(__uuidof(IAudioSessionControl2), (void**)&pSessionControl2);
        if (FAILED(mHr)) {
            pSessionControl->Release();
            continue;
        }

        DWORD processId = 0;
        mHr = pSessionControl2->GetProcessId(&processId);
        if (SUCCEEDED(mHr) && processId == mPid) {
            ISimpleAudioVolume* pVolumeControl = nullptr;
            mHr = pSessionControl2->QueryInterface(__uuidof(ISimpleAudioVolume), (void**)&pVolumeControl);
            if (SUCCEEDED(mHr)) {
                mHr = pVolumeControl->SetMute(mute, nullptr);
                if (SUCCEEDED(mHr)) {
                    // Mute succeeded
                } else {
                    LogErr("Failed to mute application");
                }
                pVolumeControl->Release();
            }
        }

        pSessionControl2->Release();
        pSessionControl->Release();
    }
}
