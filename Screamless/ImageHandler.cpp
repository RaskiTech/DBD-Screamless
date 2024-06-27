#include "ImageHandler.h"
#include "stb_image.h"
#include "stb_image_write.h"
#include "base_resample.h"
#include <iostream>
#include <string>

Image& ImageHandler::LoadImageFromDisk(const std::string& path, int insertToHandle)
{
    if (insertToHandle == -1)
        insertToHandle = GetFreeImageHandle();

    int channels;
    int wantedChannels = 3;
    stbi_uc* data = stbi_load(path.c_str(), &mImages[insertToHandle].width, &mImages[insertToHandle].height, &channels, wantedChannels);
    if (stbi_failure_reason())
        std::cout << stbi_failure_reason() << std::endl;

    int dataSize = mImages[insertToHandle].width * mImages[insertToHandle].height * wantedChannels;
    mImages[insertToHandle].buffer = std::vector<uint8_t>(data, data + dataSize);
    stbi_image_free(data);

    // Convert RGBA to BGRA
    for (int i = 0; i < mImages[insertToHandle].buffer.size(); i += wantedChannels)
    {
        int temp = mImages[insertToHandle].buffer[i];
        mImages[insertToHandle].buffer[i] = mImages[insertToHandle].buffer[i + 2];
        mImages[insertToHandle].buffer[i + 2] = temp;
    }

    return mImages[insertToHandle];
}

void ImageHandler::WriteImageToDisk(const std::string& path, Image& image)
{
    int channels = 3;

    // Convert BGR to RGB
    for (int i = 0; i < image.buffer.size(); i += channels)
        std::swap(image.buffer[i], image.buffer[i + 2]);

    stbi_write_png(path.c_str(), image.width, image.height, channels, image.buffer.data(), 0);

    for (int i = 0; i < image.buffer.size(); i += channels)
        std::swap(image.buffer[i], image.buffer[i + 2]);
}

int ImageHandler::GetFreeImageHandle()
{
    int index = 0;
    while (index < mImages.size() && mImages[index].ContainsImage())
        index++;
    if (index == mImages.size())
    {
        Log("Could not load image, image buffer full. Do you have an image leak? :D");
        return -1;
    }
    return index;
}

void ImageHandler::DeleteImage(int handle)
{
    if (handle >= mImages.size() || !mImages[handle].ContainsImage())
    {
        Log("DeleteImage was given an invalid handle");
        return;
    }
    mImages[handle] = Image();
}

void ImageHandler::DeleteImage(Image& image)
{
    DeleteImage(&image - &mImages[0]);
}

bool ImageHandler::ResizeImage(Image& img, int newWidth, int newHeight)
{
    std::vector<uint8_t> newData = std::vector<uint8_t>(newWidth * newHeight * 3);
    if (!base::ResampleImage24(img.buffer.data(), img.width, img.height, newData.data(), newWidth, newHeight, base::KernelTypeNearest))
    {
        Log("Couldn't resize image");
        return false;
    }
    img.buffer = newData;
    img.width = newWidth;
    img.height = newHeight;
    return true;
}

void ImageHandler::PrintAverageColor(const Image& img)
{
    int pixelCount = img.height * img.width;
    int channelsInPixel = img.buffer.size() / pixelCount;

    std::array<float, 3> totalColor;
	totalColor[0] = totalColor[1] = totalColor[2] = 0;
    for (int i = 0; i < img.buffer.size(); i += channelsInPixel)
    {
		totalColor[0] += (float)img.buffer[i+0] / (float)255;
        totalColor[1] += (float)img.buffer[i+1] / (float)255;
        totalColor[2] += (float)img.buffer[i+2] / (float)255;
    }

    Log("Average color for Image: ", (float)totalColor[0] / pixelCount, " ", (float)totalColor[1] / pixelCount, " ", (float)totalColor[2] / pixelCount);
}

float ImageHandler::CalculateImageDifference(const Image& baseImg, const Image& comparingImg, int comparingChannels)
{
    bool blue  = comparingChannels & CompareBlueChannel;
    bool green = comparingChannels & CompareGreenChannel;
    bool red   = comparingChannels & CompareRedChannel;

    // Difference squared
    auto costDifferenceSquared = [](uint8_t baseByte, uint8_t otherByte) {
        float diff = std::abs(baseByte - otherByte) / 255.0f;
        return diff * diff;
    };

    int pixelCount = baseImg.width * baseImg.height;
    if (comparingImg.width * comparingImg.height != pixelCount)
    {
        Log("Compering images that have different dimensions. They were (", 
            baseImg.width, ", ", baseImg.height, ") and (", comparingImg.width, ", ", comparingImg.height, ")");
        return -1;
    }

    int bytesPerPixel1 = baseImg.buffer.size() / pixelCount;
    int bytesPerPixel2 = comparingImg.buffer.size() / pixelCount;
    if (bytesPerPixel1 < 3 || bytesPerPixel2 < 3)
        return -1;

    std::array<float, 3> difference;
    difference[0] = difference[1] = difference[2] = 0;

    const uint8_t treshold = 255 * 0.5f;
    for (int i = 0; i < pixelCount; i += 1)
    {
        if (i == pixelCount / 2)
            int temp = 2;

        //float brightness = blue * (baseImg.buffer[i * bytesPerPixel1 + 0] + green * baseImg.buffer[i * bytesPerPixel1 + 1] + red * baseImg.buffer[i * bytesPerPixel1 + 2]) / ((blue + green + red) * 255.0f);
        //brightness = brightness * 0.5f + 0.5f;
        
        float brightness = 1;
        int val;

        val = comparingImg.buffer[i * bytesPerPixel1 + 0];
        if (val < treshold)
            val = 0;
            
		difference[0] += blue  * brightness * costDifferenceSquared(baseImg.buffer[i * bytesPerPixel2 + 0], val);

        val = comparingImg.buffer[i * bytesPerPixel1 + 0];
        if (val < treshold)
            val = 0;

		difference[1] += green * brightness * costDifferenceSquared(baseImg.buffer[i * bytesPerPixel2 + 1], val);

        val = comparingImg.buffer[i * bytesPerPixel1 + 0];
        if (val < treshold)
            val = 0;

		difference[2] += red   * brightness * costDifferenceSquared(baseImg.buffer[i * bytesPerPixel2 + 2], val);
    }
    
    float overallAverage = (difference[0] + difference[1] + difference[2]) / ((blue + green + red) * pixelCount);
    return overallAverage;
}

float ImageHandler::CalculateRedPrecense(const Image& weightTable, const Image& comparingImage)
{
    int pixelCount = weightTable.width * weightTable.height;
    if (comparingImage.width * comparingImage.height != pixelCount)
    {
        Log("Compering images that have different dimensions. They were (",
            weightTable.width, ", ", weightTable.height, ") and (", comparingImage.width, ", ", comparingImage.height, ")");
        return -1;
    }

    int bytesPerPixel1 = weightTable.buffer.size() / pixelCount;
    int bytesPerPixel2 = comparingImage.buffer.size() / pixelCount;
    if (bytesPerPixel1 < 3 || bytesPerPixel2 < 3)
        return -1;

    // Calculate the total redness and weigh it against the weight table
    float cost = 0;
    for (int i = 0; i < pixelCount; i++)
    {
        float blueGreenAverage = (comparingImage.buffer[i * bytesPerPixel2 + 0] + comparingImage.buffer[i * bytesPerPixel2 + 1]) / 2.0f;
        float redness = comparingImage.buffer[i * bytesPerPixel2 + 2] - blueGreenAverage;
        //float difference = std::abs(weightTable.buffer[i * bytesPerPixel1 + 2] - redness) / 255.0f;
        float difference = max(0, redness / 255.0f);
        difference *= weightTable.buffer[i * bytesPerPixel1 + 2] / 255.0f;
        cost += difference * difference;
    }

    return cost / (pixelCount);
}
