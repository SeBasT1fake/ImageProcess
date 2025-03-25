#include <iostream>
#include <chrono>
#include <string>
#include <vector>
#include <cmath>
#include <cstring>
#include <malloc.h>
#include "buddy_allocator.h"
#include "image_processor.h"

#define VERSION "1.0.0"

// Define command line options
struct ProgramOptions {
    std::string inputFile;
    std::string outputFile;
    double rotationAngle = 0.0;
    double scaleFactor = 1.0;
    bool useBuddySystem = false;
    bool showHelp = false;
    bool showVersion = false;
};

void printUsage() {
    std::cout << "=== AYUDA: USO DEL PROGRAMA ===" << std::endl;
    std::cout << "Uso:\n  ./program_image entrada.jpg salida.jpg -angulo ANGULO -escalar ESCALA [-buddy]\n" << std::endl;
    std::cout << "Parámetros:" << std::endl;
    std::cout << "  entrada.jpg        Archivo de imagen de entrada" << std::endl;
    std::cout << "  salida.jpg         Archivo donde se guarda la imagen procesada" << std::endl;
    std::cout << "  -angulo ANGULO     Ángulo de rotación (en grados, puede ser decimal)" << std::endl;
    std::cout << "  -escalar ESCALA    Factor de escalado (por ejemplo 0.5, 1.5, 2.0, etc.)" << std::endl;
    std::cout << "  -buddy             (Opcional) Usa el sistema de asignación de memoria Buddy System" << std::endl;
    std::cout << "  -h, --help         Muestra esta ayuda" << std::endl;
    std::cout << "  -v, --version      Muestra la versión del programa" << std::endl;
}

void printVersion() {
    std::cout << "program_image v" << VERSION << std::endl;
    std::cout << "Desarrollado para procesamiento de imágenes y comparación de sistemas de memoria." << std::endl;
}

ProgramOptions parseCommandLine(int argc, char* argv[]) {
    ProgramOptions options;

    if (argc == 1) {
        std::cout << "Programa de procesamiento de imágenes." << std::endl;
        std::cout << "Usa './program_image -h' para ver la ayuda." << std::endl;
        exit(0);
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            options.showHelp = true;
            printUsage();
            exit(0);
        } else if (arg == "-v" || arg == "--version") {
            options.showVersion = true;
            printVersion();
            exit(0);
        } else if (arg == "-angulo" && i + 1 < argc) {
            options.rotationAngle = std::stod(argv[++i]);
        } else if (arg == "-escalar" && i + 1 < argc) {
            options.scaleFactor = std::stod(argv[++i]);
        } else if (arg == "-buddy") {
            options.useBuddySystem = true;
        } else if (options.inputFile.empty()) {
            options.inputFile = arg;
        } else if (options.outputFile.empty()) {
            options.outputFile = arg;
        } else {
            std::cout << "Parámetro desconocido: " << arg << std::endl;
            printUsage();
            exit(1);
        }
    }

    if (options.inputFile.empty() || options.outputFile.empty()) {
        std::cout << "Faltan archivos de entrada/salida." << std::endl;
        printUsage();
        exit(1);
    }

    return options;
}

int main(int argc, char* argv[]) {
    ProgramOptions options = parseCommandLine(argc, argv);

    std::cout << "=== PROCESAMIENTO DE IMAGEN ===" << std::endl;
    std::cout << "Archivo de entrada: " << options.inputFile << std::endl;
    std::cout << "Archivo de salida: " << options.outputFile << std::endl;
    std::cout << "Modo de asignación de memoria: " << (options.useBuddySystem ? "Buddy System" : "Convencional") << std::endl;
    std::cout << "------------------------" << std::endl;

    // Inicializar el Buddy Allocator
    BuddyAllocator buddyAllocator(24); // 16MB

    // Proceso convencional
    auto startConventional = std::chrono::high_resolution_clock::now();
    ImageProcessor conventionalProcessor(false, nullptr);

    if (!conventionalProcessor.loadImage(options.inputFile)) {
        std::cerr << "Error cargando la imagen: " << options.inputFile << std::endl;
        return 1;
    }

    int origWidth, origHeight, origChannels;
    conventionalProcessor.getImageInfo(origWidth, origHeight, origChannels);
    std::cout << "Dimensiones originales: " << origWidth << " x " << origHeight << std::endl;
    std::cout << "Canales: " << origChannels << (origChannels == 3 ? " (RGB)" : " (RGBA)") << std::endl;
    std::cout << "Ángulo de rotación: " << options.rotationAngle << " grados" << std::endl;
    std::cout << "Factor de escalado: " << options.scaleFactor << std::endl;
    std::cout << "------------------------" << std::endl;

    conventionalProcessor.rotateImage(options.rotationAngle);
    std::cout << "[INFO] Imagen rotada correctamente." << std::endl;

    conventionalProcessor.scaleImage(options.scaleFactor);
    std::cout << "[INFO] Imagen escalada correctamente." << std::endl;

    conventionalProcessor.saveImage("temp_conventional.jpg");

    auto endConventional = std::chrono::high_resolution_clock::now();
    auto durationConventional = std::chrono::duration_cast<std::chrono::milliseconds>(endConventional - startConventional);

    struct mallinfo2 mallocInfo = mallinfo2();
    size_t conventionalMemory = mallocInfo.uordblks;

    // Proceso con Buddy System
    auto startBuddy = std::chrono::high_resolution_clock::now();
    ImageProcessor buddyProcessor(true, &buddyAllocator);

    buddyProcessor.loadImage(options.inputFile);
    buddyProcessor.rotateImage(options.rotationAngle);
    buddyProcessor.scaleImage(options.scaleFactor);
    buddyProcessor.saveImage(options.outputFile);

    auto endBuddy = std::chrono::high_resolution_clock::now();
    auto durationBuddy = std::chrono::duration_cast<std::chrono::milliseconds>(endBuddy - startBuddy);
    size_t buddyMemory = buddyAllocator.getTotalAllocated();

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
