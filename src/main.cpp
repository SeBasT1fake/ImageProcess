#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <cmath>
#include <cstring>
#include <malloc.h>
#include "buddy_allocator.h"
#include "image_processor.h"

// Define command line options
struct ProgramOptions {
    std::string inputFile;
    std::string outputFile;
    double rotationAngle = 0.0;
    double scaleFactor = 1.0;
    bool useBuddySystem = false;
};

void printUsage() {
    std::cout << "Usage: ./program_image input.jpg output.jpg -angulo ANGLE -escalar SCALE [-buddy]" << std::endl;
    std::cout << "Parameters:" << std::endl;
    std::cout << "  input.jpg: input image file" << std::endl;
    std::cout << "  output.jpg: output image file" << std::endl;
    std::cout << "  -angulo: rotation angle in degrees" << std::endl;
    std::cout << "  -escalar: scaling factor (> 0)" << std::endl;
    std::cout << "  -buddy: activates Buddy System allocation mode (optional)" << std::endl;
}

ProgramOptions parseCommandLine(int argc, char* argv[]) {
    ProgramOptions options;
    
    if (argc < 5) {
        printUsage();
        exit(1);
    }
    
    options.inputFile = argv[1];
    options.outputFile = argv[2];
    
    for (int i = 3; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-angulo" && i + 1 < argc) {
            options.rotationAngle = std::stod(argv[++i]);
        } else if (arg == "-escalar" && i + 1 < argc) {
            options.scaleFactor = std::stod(argv[++i]);
        } else if (arg == "-buddy") {
            options.useBuddySystem = true;
        } else {
            std::cout << "Unknown parameter: " << arg << std::endl;
            printUsage();
            exit(1);
        }
    }
    
    return options;
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    ProgramOptions options = parseCommandLine(argc, argv);
    
    std::cout << "=== PROCESAMIENTO DE IMAGEN ===" << std::endl;
    std::cout << "Archivo de entrada: " << options.inputFile << std::endl;
    std::cout << "Archivo de salida: " << options.outputFile << std::endl;
    std::cout << "Modo de asignación de memoria: " << (options.useBuddySystem ? "Buddy System" : "Convencional") << std::endl;
    std::cout << "------------------------" << std::endl;
    
    // Initialize our buddy allocator
    BuddyAllocator buddyAllocator(24); // 16MB (2^24 bytes)
    
    // Process with conventional allocation
    auto startConventional = std::chrono::high_resolution_clock::now();
    ImageProcessor conventionalProcessor(false, nullptr);
    
    // Load image
    if (!conventionalProcessor.loadImage(options.inputFile)) {
        std::cerr << "Error loading image: " << options.inputFile << std::endl;
        return 1;
    }
    
    // Display original dimensions
    int origWidth, origHeight, origChannels;
    conventionalProcessor.getImageInfo(origWidth, origHeight, origChannels);
    std::cout << "Dimensiones originales: " << origWidth << " x " << origHeight << std::endl;
    std::cout << "Canales: " << origChannels << (origChannels == 3 ? " (RGB)" : " (RGBA)") << std::endl;
    std::cout << "Ángulo de rotación: " << options.rotationAngle << " grados" << std::endl;
    std::cout << "Factor de escalado: " << options.scaleFactor << std::endl;
    std::cout << "------------------------" << std::endl;
    
    // Process image with conventional allocation
    conventionalProcessor.rotateImage(options.rotationAngle);
    std::cout << "[INFO] Imagen rotada correctamente." << std::endl;
    
    conventionalProcessor.scaleImage(options.scaleFactor);
    std::cout << "[INFO] Imagen escalada correctamente." << std::endl;
    
    // Save processed image (temporary)
    conventionalProcessor.saveImage("temp_conventional.jpg");
    
    auto endConventional = std::chrono::high_resolution_clock::now();
    auto durationConventional = std::chrono::duration_cast<std::chrono::milliseconds>(endConventional - startConventional);
    
    // Get memory usage for conventional allocation
    struct mallinfo2 mallocInfo = mallinfo2();
    size_t conventionalMemory = mallocInfo.uordblks; // Bytes in use
    
    // Process with Buddy System allocation
    auto startBuddy = std::chrono::high_resolution_clock::now();
    ImageProcessor buddyProcessor(true, &buddyAllocator);
    
    // Load image
    buddyProcessor.loadImage(options.inputFile);
    
    // Process image with Buddy System allocation
    buddyProcessor.rotateImage(options.rotationAngle);
    buddyProcessor.scaleImage(options.scaleFactor);
    
    // Save the final processed image
    buddyProcessor.saveImage(options.outputFile);
    
    auto endBuddy = std::chrono::high_resolution_clock::now();
    auto durationBuddy = std::chrono::duration_cast<std::chrono::milliseconds>(endBuddy - startBuddy);
    
    // Get memory usage for Buddy System allocation
    size_t buddyMemory = buddyAllocator.getTotalAllocated();
    
    // Get final dimensions
    int finalWidth, finalHeight, finalChannels;
    buddyProcessor.getImageInfo(finalWidth, finalHeight, finalChannels);
    
    std::cout << "------------------------" << std::endl;
    std::cout << "Dimensiones finales: " << finalWidth << " x " << finalHeight << std::endl;
    std::cout << "------------------------" << std::endl;
    std::cout << "TIEMPO DE PROCESAMIENTO:" << std::endl;
    std::cout << " - Sin Buddy System: " << durationConventional.count() << " ms" << std::endl;
    std::cout << " - Con Buddy System: " << durationBuddy.count() << " ms" << std::endl;
    std::cout << std::endl;
    std::cout << "MEMORIA UTILIZADA:" << std::endl;
    std::cout << " - Sin Buddy System: " << conventionalMemory / (1024.0 * 1024.0) << " MB" << std::endl;
    std::cout << " - Con Buddy System: " << buddyMemory / (1024.0 * 1024.0) << " MB" << std::endl;
    std::cout << "------------------------" << std::endl;
    std::cout << "[INFO] Imagen guardada correctamente en " << options.outputFile << std::endl;
    
    return 0;
}