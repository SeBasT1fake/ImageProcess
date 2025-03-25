#include "image_processor.h"
#include <iostream>
#include <cmath>
#include <cstring>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const double PI = 3.14159265358979323846;

ImageProcessor::ImageProcessor(bool useBuddySystem, BuddyAllocator* allocator)
    : imageData(nullptr), width(0), height(0), channels(0),
      useBuddySystem(useBuddySystem), allocator(allocator) {
}

ImageProcessor::~ImageProcessor() {
    deallocateImage();
}

void ImageProcessor::allocateImage(int w, int h, int c) {
    deallocateImage();
    width = w;
    height = h;
    channels = c;

    size_t size = width * height * channels * sizeof(unsigned char);
    if (useBuddySystem && allocator) {
        imageData = static_cast<unsigned char*>(allocator->allocate(size));
    } else {
        imageData = new unsigned char[size];
    }
}

void ImageProcessor::deallocateImage() {
    if (imageData) {
        if (useBuddySystem && allocator) {
            allocator->deallocate(imageData);
        } else {
            delete[] imageData;
        }
        imageData = nullptr;
    }
}

bool ImageProcessor::loadImage(const std::string& filename) {
    deallocateImage();

    int w, h, c;
    unsigned char* loadedData = stbi_load(filename.c_str(), &w, &h, &c, 0);
    if (!loadedData) {
        std::cerr << "Failed to load image: " << filename << std::endl;
        return false;
    }

    allocateImage(w, h, c);
    std::memcpy(imageData, loadedData, w * h * c * sizeof(unsigned char));
    stbi_image_free(loadedData);
    return true;
}

bool ImageProcessor::saveImage(const std::string& filename) {
    if (!imageData) {
        std::cerr << "No image data to save" << std::endl;
        return false;
    }

    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) {
        std::cerr << "Unknown file extension" << std::endl;
        return false;
    }

    std::string ext = filename.substr(dotPos + 1);
    bool success = false;

    if (ext == "jpg" || ext == "jpeg") {
        success = stbi_write_jpg(filename.c_str(), width, height, channels, imageData, 95) != 0;
    } else if (ext == "png") {
        success = stbi_write_png(filename.c_str(), width, height, channels, imageData, width * channels) != 0;
    } else if (ext == "bmp") {
        success = stbi_write_bmp(filename.c_str(), width, height, channels, imageData) != 0;
    } else {
        std::cerr << "Unsupported file format: " << ext << std::endl;
        return false;
    }

    return success;
}

void ImageProcessor::getImageInfo(int& w, int& h, int& c) {
    w = width;
    h = height;
    c = channels;
}

unsigned char* ImageProcessor::getPixel(unsigned char* data, int x, int y, int c, int w, int h) {
    if (x < 0) x = 0;
    if (x >= w) x = w - 1;
    if (y < 0) y = 0;
    if (y >= h) y = h - 1;

    return &data[(y * w + x) * channels + c];
}

void ImageProcessor::setPixel(unsigned char* data, int x, int y, int c, unsigned char value, int w, int h) {
    if (x >= 0 && x < w && y >= 0 && y < h) {
        data[(y * w + x) * channels + c] = value;
    }
}

unsigned char ImageProcessor::bilinearInterpolation(unsigned char* data, double x, double y, int c, int w, int h) {
    int x1 = static_cast<int>(x);
    int y1 = static_cast<int>(y);
    int x2 = x1 + 1;
    int y2 = y1 + 1;

    double xFrac = x - x1;
    double yFrac = y - y1;

    unsigned char p1 = *getPixel(data, x1, y1, c, w, h);
    unsigned char p2 = *getPixel(data, x2, y1, c, w, h);
    unsigned char p3 = *getPixel(data, x1, y2, c, w, h);
    unsigned char p4 = *getPixel(data, x2, y2, c, w, h);

    double top = p1 * (1 - xFrac) + p2 * xFrac;
    double bottom = p3 * (1 - xFrac) + p4 * xFrac;
    double result = top * (1 - yFrac) + bottom * yFrac;

    return static_cast<unsigned char>(result);
}

void ImageProcessor::rotateImage(double angle) {
    if (!imageData) return;

    double radians = angle * PI / 180.0;
    double cosA = std::cos(radians);
    double sinA = std::sin(radians);

    double absAngleCos = std::abs(cosA);
    double absAngleSin = std::abs(sinA);
    int newWidth = static_cast<int>(width * absAngleCos + height * absAngleSin);
    int newHeight = static_cast<int>(width * absAngleSin + height * absAngleCos);

    size_t newSize = newWidth * newHeight * channels * sizeof(unsigned char);
    unsigned char* rotatedData;

    if (useBuddySystem && allocator) {
        rotatedData = static_cast<unsigned char*>(allocator->allocate(newSize));
    } else {
        rotatedData = new unsigned char[newSize];
    }

    std::memset(rotatedData, 0, newSize);

    double oldCenterX = width / 2.0;
    double oldCenterY = height / 2.0;
    double newCenterX = newWidth / 2.0;
    double newCenterY = newHeight / 2.0;

    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            double xRel = x - newCenterX;
            double yRel = y - newCenterY;

            double xOld = xRel * cosA + yRel * sinA + oldCenterX;
            double yOld = -xRel * sinA + yRel * cosA + oldCenterY;

            if (xOld >= 0 && xOld <= width - 1 && yOld >= 0 && yOld <= height - 1) {
                for (int c = 0; c < channels; c++) {
                    unsigned char value = bilinearInterpolation(imageData, xOld, yOld, c, width, height);
                    setPixel(rotatedData, x, y, c, value, newWidth, newHeight);
                }
            }
        }
    }

    deallocateImage();
    imageData = rotatedData;
    width = newWidth;
    height = newHeight;
}

void ImageProcessor::scaleImage(double factor) {
    if (!imageData || factor <= 0) return;

    int newWidth = static_cast<int>(std::round(width * factor));
    int newHeight = static_cast<int>(std::round(height * factor));

    size_t newSize = newWidth * newHeight * channels * sizeof(unsigned char);
    unsigned char* scaledData;

    if (useBuddySystem && allocator) {
        scaledData = static_cast<unsigned char*>(allocator->allocate(newSize));
    } else {
        scaledData = new unsigned char[newSize];
    }

    std::memset(scaledData, 0, newSize);

    double xRatio = width / static_cast<double>(newWidth);
    double yRatio = height / static_cast<double>(newHeight);

    for (int y = 0; y < newHeight; y++) {
        for (int x = 0; x < newWidth; x++) {
            double xOld = x * xRatio;
            double yOld = y * yRatio;

            for (int c = 0; c < channels; c++) {
                unsigned char value = bilinearInterpolation(imageData, xOld, yOld, c, width, height);
                setPixel(scaledData, x, y, c, value, newWidth, newHeight);
            }
        }
    }

    deallocateImage();
    imageData = scaledData;
    width = newWidth;
    height = newHeight;
}
