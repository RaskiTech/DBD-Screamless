#pragma once
#include "Utility.h"
#include <string>
#include <windows.h>
#include <array>
#include <vector>


class DisplayManager
{
public:
	DisplayManager() {};
	~DisplayManager();

	void Initialize(const std::string& appName);
	void Initialize(void* windowHandle);
	void UpdateLoadedWindow(); // This function should be only called in testing builds, as it is slow and clunky.
	void Uninitialize();
	bool IsWindowValid();

	// Due to limitaions on some software, this function uses the desktop window the capture
	void LoadImageFromScreenView(BoundingBox boundingBox, Image& outImage);
	void LoadImageFromDesktopView(BoundingBox boundingBox, Image& outImage);

	void DrawRect(BoundingBox box);
	void DrawImage(int x, int y, Image& image);
	std::array<int, 2> GetWindowSize();

private:
	void LoadImageFromHandle(BoundingBox DesktopBoundingBox, Image& outImage, HDC usedWindowDC);

	void ConvertScreenToWindowSpace(int* x, int* y);
	void ConvertScreenToWindowSpace(LONG* x, LONG* y);
	void ConvertWindowToScreenSpace(int* x, int* y);
	void ConvertWindowToScreenSpace(LONG* x, LONG* y);

	HWND mWindowHandle = NULL;
	float mWindowScaleSetting = 0;
};
