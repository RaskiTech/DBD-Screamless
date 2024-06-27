#pragma once
#include <string>
#include <windows.h>
#include <mmdeviceapi.h>
#include <audiopolicy.h>

class VolumeController
{
public:
	void Initialize(const std::string& appName);
	void Uninitialize();
	bool IsInitialized();
	~VolumeController() { Uninitialize(); }

	void Mute(bool mute);

private:
	DWORD mPid = 0;

	HRESULT mHr;
	IMMDeviceEnumerator* mDeviceEnumerator = nullptr;
	IMMDevice* mDevice = nullptr;
	IAudioSessionManager2* mAudioSessionManager = nullptr;
	IAudioSessionEnumerator* mAudioSessionEnumerator = nullptr;

};
