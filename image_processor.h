#ifndef IMAGE_PROCESSOR_H
#define IMAGE_PROCESSOR_H

#include <string>
#include "buddy_allocator.h"

class ImageProcessor {
public:
    ImageProcessor(bool useBuddySystem, BuddyAllocator* allocator);
    ~ImageProcessor();
    
    // Load an image from file
    bool loadImage(const std::string& filename);
    
    // Save the image to file
    bool saveImage(const std::string& filename);
    
    // Rotate the image by the specified angle (in degrees)
    void rotateImage(double angle);
    
    // Scale the image by the specified factor
    void scaleImage(double factor);
    
    // Get image information
    void getImageInfo(int& width, int& height, int& channels);

private:
    // Image data
    unsigned char* imageData;
    int width;
    int height;
    int channels;
    
    // Memory allocation mode
    bool useBuddySystem;
    BuddyAllocator* allocator;
    
    // Helper methods
    void allocateImage(int w, int h, int c);
    void deallocateImage();
    unsigned char* getPixel(unsigned char* data, int x, int y, int c);
    void setPixel(unsigned char* data, int x, int y, int c, unsigned char value);
    unsigned char bilinearInterpolation(unsigned char* data, double x, double y, int c);
};

#endif // IMAGE_PROCESSOR_H