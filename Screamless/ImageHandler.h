#pragma once
#include "Utility.h"
#include <string>
#include <array>

class ImageHandler
{
public:
	ImageHandler() {};

	Image& GetImage(int handle) { return mImages[handle]; }
	int GetFreeImageHandle();
	Image& GetFreeImage() { return mImages[GetFreeImageHandle()]; }

	Image& LoadImageFromDisk(const std::string& path, int insertToHandle = -1);
	void WriteImageToDisk(const std::string& path, Image& image);
	void DeleteImage(int index);
	void DeleteImage(Image& image);
	bool ResizeImage(Image& img, int newWidth, int newHeight);
	bool ResizeImage(int img, int newWidth, int newHeight) { return ResizeImage(mImages[img], newWidth, newHeight); }
	void PrintAverageColor(const Image& img);

	static const int CompareBlueChannel  = 1 << 0;
	static const int CompareGreenChannel = 1 << 1;
	static const int CompareRedChannel   = 1 << 2;
	float CalculateImageDifference(const Image& baseImage, const Image& comparingImage, int comparingChannels);
	float CalculateRedPrecense(const Image& weightTable, const Image& comparingImage);

private:
	std::array<Image, 10> mImages = std::array<Image, 10>();
};
