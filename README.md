
# Image Processor with Buddy System

This C++ project implements an image processor capable of performing **rotation** and **scaling** operations on images, using two different memory management methods: the standard dynamic allocation (`new/delete`) and a custom **Buddy System** allocator.

## Requirements

- Operating system compatible with C++
- `g++` compiler with C++11 or higher
- `stb_image.h` and `stb_image_write.h` libraries already included in the `src/` folder

## Project Structure


imageProcess/

├── Makefile

├── src/               # Source code (.cpp and .h)

├── bin/               # Compiled executable

├── build/             # Object files (.o)

├── assets/            # Input images

├── out/               # Output images



## Compilation

Make sure you have the `stb_image.h` and `stb_image_write.h` files inside the `src/` folder. Then run:

```bash
make
```

This will compile the source code and generate the executable at `bin/program_image`.

## Execution

The program is executed from the command line using the following format:

```bash
./bin/program_image assets/input.jpg out/output.jpg -angulo 45 -escalar 1.5 [-buddy]
```

### Parameters

- `input.jpg`: image file located in the `assets/` folder
- `output.jpg`: name of the processed image to be saved in `out/`
- `-angulo`: rotation angle in degrees
- `-escalar`: scaling factor (e.g. 0.5, 1.5, 2.0)
- `-buddy`: optional flag to enable Buddy System memory allocation

## Example

```bash
./bin/program_image assets/image.jpg out/result.jpg -angulo 30 -escalar 1.2 -buddy
```

This command will:
1. Load the input image from `assets/image.jpg`
2. Rotate the image by 30 degrees (clockwise)
3. Scale the image by a factor of 1.2
4. Use the Buddy System for memory allocation
5. Save the processed image as `out/result.jpg`

## Source Files

- `main.cpp`: Program entry point, command-line parsing and control flow
- `buddy_allocator.h/cpp`: Implementation of the Buddy memory allocator
- `image_processor.h/cpp`: Image operations (load, rotate, scale, save)
- `stb_image.h`: Header for loading image data (included in `src/`)
- `stb_image_write.h`: Header for writing image data (included in `src/`)

## Implementation Details

### Buddy System

The custom Buddy allocator:
- Initializes with a power-of-two memory pool
- Recursively splits memory into buddy pairs to fit allocations
- Merges adjacent free buddies to reduce fragmentation
- Tracks allocated and free blocks efficiently

### Image Operations

- **Rotation**: Uses bilinear interpolation around the center of the image
- **Scaling**: Maintains aspect ratio with smooth bilinear resizing

## Performance Comparison

At the end of the execution, the program prints:
- Processing time using both memory modes
- Memory usage with and without the Buddy System
- Original and final image dimensions
