#include "DisplayManager.h"
#include <iostream>
#include <vector>
#include <array>
#include "Utility.h"

DisplayManager::~DisplayManager()
{
    Uninitialize();
}

void DisplayManager::Initialize(const std::string& appName)
{
    Initialize(FindWindowA(NULL, appName.c_str()));
}
void DisplayManager::Initialize(void* windowHandle)
{
    mWindowHandle = *(HWND*)&windowHandle;
    if (mWindowHandle == NULL || !IsWindowValid())
    {
        LogErr("Provided null window handle");
        return;
    }


    HDC dc = GetDC(GetDesktopWindow());
    int dpiX = GetDpiForWindow(mWindowHandle);
    //int dpiY = GetDeviceCaps(dc, LOGPIXELSY);

    // The magic number 96 is windows industry standard for 100% scale
    mWindowScaleSetting = dpiX / 96.0f;

    ReleaseDC(GetDesktopWindow(), dc);
}

void DisplayManager::UpdateLoadedWindow()
{
	// Save the current display settings
	DEVMODE dm;
	ZeroMemory(&dm, sizeof(dm));
	dm.dmSize = sizeof(dm);
	EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm);

	// Change the display settings to force a refresh
	dm.dmBitsPerPel = dm.dmBitsPerPel == 32 ? 16 : 32;
	ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
	ChangeDisplaySettings(NULL, 0);
}

void DisplayManager::Uninitialize()
{

}

bool DisplayManager::IsWindowValid()
{
	return IsWindow(mWindowHandle);
}

void DisplayManager::LoadImageFromScreenView(BoundingBox boundingBox, Image& outImage)
{
    HDC handle = GetDC(mWindowHandle);
    LoadImageFromHandle(boundingBox, outImage, handle);
    ReleaseDC(mWindowHandle, handle);
}
void DisplayManager::LoadImageFromDesktopView(BoundingBox boundingBox, Image& outImage)
{
    HWND desktop = GetDesktopWindow();
    HDC deskDC = GetDC(desktop);
    
    ConvertWindowToScreenSpace(&boundingBox.x, &boundingBox.y);
    LoadImageFromHandle(boundingBox, outImage, deskDC);
    
    ReleaseDC(desktop, deskDC);
}


void DisplayManager::LoadImageFromHandle(BoundingBox boundingBox, Image& outImage, HDC usedImageHandle)
{
    HDC hdcMemDC = CreateCompatibleDC(usedImageHandle);

    if (!hdcMemDC) {
        LogErr("CreateCompatibleDC has failed");
        return;
    }

    HBITMAP hBitmap = CreateCompatibleBitmap(usedImageHandle, boundingBox.width, boundingBox.height);

    if (!hBitmap) {
        LogErr("CreateCompatibleBitmap has failed");
        DeleteDC(hdcMemDC);
        return;
    }

    SelectObject(hdcMemDC, hBitmap);

    if (!BitBlt(hdcMemDC, 0, 0, boundingBox.width, boundingBox.height, usedImageHandle, boundingBox.x, boundingBox.y, SRCCOPY)) {
        LogErr("BitBlt has failed");
		DeleteObject(hBitmap);
        DeleteDC(hdcMemDC);
        return;
    }

    // Convert bitmap to vector
    BITMAP bitmap;
    GetObject(hBitmap, sizeof(BITMAP), &bitmap);

    int bytesPerPixel = 3;
    int paddedRowSize = ((boundingBox.width * bytesPerPixel + 3) / 4) * 4;
    int imageByteCount = boundingBox.width * boundingBox.height * bytesPerPixel;

    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMemDC, hBitmap);

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(BITMAPINFO));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = boundingBox.width;
    bmi.bmiHeader.biHeight = -boundingBox.height;  // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 8 * bytesPerPixel;
    bmi.bmiHeader.biCompression = BI_RGB;

    std::vector<uint8_t> paddedBuffer(paddedRowSize * boundingBox.height);
    outImage.buffer.resize(imageByteCount);
    outImage.width = boundingBox.width;
    outImage.height = boundingBox.height;

	if (!GetDIBits(hdcMemDC, hBitmap, 0, boundingBox.height, paddedBuffer.data(), &bmi, DIB_RGB_COLORS)) {
        LogErr("GetDIBits has failed for hBitmap");
        SelectObject(hdcMemDC, hOldBitmap);
		DeleteObject(hBitmap);
        DeleteDC(hdcMemDC);
        return;
    }

    // Copy the image data row by row, ignoring the padding
    for (int y = 0; y < boundingBox.height; ++y) {
        memcpy(outImage.buffer.data() + y * boundingBox.width * bytesPerPixel,
               paddedBuffer.data() + y * paddedRowSize,
               boundingBox.width * bytesPerPixel);
    }

	SelectObject(hdcMemDC, hOldBitmap);
	DeleteObject(hBitmap);
    DeleteDC(hdcMemDC);
}



// Draw a rectangle to the desktop display
void DisplayManager::DrawRect(BoundingBox box)
{
    RECT rect = { box.x, box.y, 0, 0 };
    ConvertWindowToScreenSpace(&rect.left, &rect.top);
    rect.right = rect.left + box.width;
    rect.bottom = rect.top + box.height;

    HBRUSH blueBrush = CreateSolidBrush(RGB(80, 100, 50));

    HWND desktop = GetDesktopWindow();
    HDC deskDC = GetDC(desktop);

    bool success = FillRect(deskDC, &rect, blueBrush);

    ReleaseDC(desktop, deskDC);
}

void DisplayManager::DrawImage(int x, int y, Image& image)
{
    ConvertWindowToScreenSpace(&x, &y);

    HWND desktop = GetDesktopWindow();
    HDC deskDC = GetDC(desktop);
    
    // Create a memory device context (DC) compatible with the desktop DC
    HDC memDC = CreateCompatibleDC(deskDC);

	// Create a bitmap compatible with the desktop DC

	int bytesPerPixel = image.buffer.size() / (image.height * image.width);
	int stride = ((image.width * bytesPerPixel + 3) / 4) * 4; // Floor everything inside brackets

	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = image.width;
	bmi.bmiHeader.biHeight = -image.height;  // Use negative height for top-down bitmap
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 24;     // Assuming the buffer is in 24-bit RGB format
	bmi.bmiHeader.biCompression = BI_RGB;

	// Create the DIB section
	void* pBits;
	HBITMAP hBitmap = CreateDIBSection(deskDC, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
	if (hBitmap == NULL)
	{
		LogErr("CreateDIBSection has failed. ", GetLastError());
		ReleaseDC(desktop, deskDC);
		return;
	}

	// Copy the buffer data to the DIB section
    // Do this row by row to account for 4-bit alignment requirement iint DIB
	for (int y = 0; y < image.height; ++y)
	{
		memcpy((BYTE*)pBits + y * stride, image.buffer.data() + y * image.width * bytesPerPixel, image.width * bytesPerPixel);
	}

    // Select the bitmap into the memory DC
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, hBitmap);

    // Copy the bitmap to the screen
    BitBlt(deskDC, x, y, image.width, image.height, memDC, 0, 0, SRCCOPY);

    // Clean up
    SelectObject(memDC, oldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(memDC);
    ReleaseDC(desktop, deskDC);
}

std::array<int, 2> DisplayManager::GetWindowSize()
{
    RECT rect;
    GetClientRect(mWindowHandle, &rect);
    return std::array<int, 2>{(int)((rect.right - rect.left) * mWindowScaleSetting), (int)((rect.bottom - rect.top) * mWindowScaleSetting)};
}

std::array<int, 2> DisplayManager::GetWindowOffset()
{
    RECT rect;
    GetClientRect(mWindowHandle, &rect);
    return std::array<int, 2>{(int)(rect.left * mWindowScaleSetting), (int)(rect.top * mWindowScaleSetting)};
}

std::array<int, 2> DisplayManager::GetWindowSmallOffset()
{
    return { (int)((mWindowScaleSetting - 1.0f) * 8), (int)((mWindowScaleSetting - 1.0f) * 8) };
}

void DisplayManager::ConvertScreenToWindowSpace(int* x, int* y)
{
	POINT pos;
    pos.x = *x / mWindowScaleSetting;
	pos.y = *y / mWindowScaleSetting;
	ClientToScreen(mWindowHandle, &pos);
    *x = pos.x * mWindowScaleSetting;
    *y = pos.y * mWindowScaleSetting;
}
void DisplayManager::ConvertScreenToWindowSpace(LONG* x, LONG* y)
{
	POINT pos;
	pos.x = *x / mWindowScaleSetting;
	pos.y = *y / mWindowScaleSetting;
	ClientToScreen(mWindowHandle, &pos);
    *x = pos.x * mWindowScaleSetting;
    *y = pos.y * mWindowScaleSetting;
}

void DisplayManager::ConvertWindowToScreenSpace(int* x, int* y)
{
	POINT pos;
	pos.x = *x / mWindowScaleSetting;
	pos.y = *y / mWindowScaleSetting;
    ClientToScreen(mWindowHandle, &pos);
    *x =  pos.x * mWindowScaleSetting;
    *y = pos.y * mWindowScaleSetting;
}
void DisplayManager::ConvertWindowToScreenSpace(LONG* x, LONG* y)
{
	POINT pos;
	pos.x = *x / mWindowScaleSetting;
	pos.y = *y / mWindowScaleSetting;
	ScreenToClient(mWindowHandle, &pos);
    *x = pos.x * mWindowScaleSetting;
    *y = pos.y * mWindowScaleSetting;
}
