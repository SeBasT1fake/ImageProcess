# Image Processor with Buddy System

Este programa implementa un procesador de imágenes en C++ que permite realizar operaciones de rotación y escalado, utilizando dos métodos diferentes de gestión de memoria: la asignación convencional (new/delete) y el sistema Buddy System.

## Requisitos

- Sistema operativo compatible con C++ (Linux, macOS, Windows con WSL)
- Compilador g++ con soporte para C++11
- Biblioteca stb_image y stb_image_write (se descargan automáticamente durante la compilación)
- curl (para descargar las bibliotecas durante la compilación)

## Compilación

Para compilar el programa, siga estos pasos:

1. Descomprima el archivo .zip o .tar.gz
2. Navegue al directorio del proyecto
3. Ejecute el comando `make`

```bash
cd image-processor-buddy
make
```

La compilación descargará automáticamente las bibliotecas stb_image necesarias para el procesamiento de imágenes.

## Ejecución

El programa se ejecuta desde la línea de comandos con los siguientes parámetros:

```bash
./programa_imagen entrada.jpg salida.jpg -angulo 45 -escalar 1.5 [-buddy]
```

Parámetros:
- `entrada.jpg`: archivo de imagen de entrada
- `salida.jpg`: archivo donde se guarda la imagen procesada
- `-angulo`: define el ángulo de rotación en grados
- `-escalar`: define el factor de escalado
- `-buddy`: activa el modo Buddy System (si se omite, se usará el modo convencional)

## Ejemplo de uso

```bash
./programa_imagen imagen.jpg resultado.jpg -angulo 30 -escalar 1.2 -buddy
```

Este comando:
1. Carga la imagen `imagen.jpg`
2. Rota la imagen 30 grados en sentido horario
3. Escala la imagen por un factor de 1.2
4. Utiliza el Buddy System para la gestión de memoria
5. Guarda el resultado en `resultado.jpg`

## Estructura del código

- `main.cpp`: Punto de entrada del programa, manejo de argumentos y coordinación de operaciones
- `buddy_allocator.h/cpp`: Implementación del sistema de asignación de memoria Buddy System
- `image_processor.h/cpp`: Operaciones de procesamiento de imágenes (carga, rotación, escalado, guardado)
- `stb_image.h`: Biblioteca para la carga de imágenes (descargada automáticamente)
- `stb_image_write.h`: Biblioteca para guardar imágenes (descargada automáticamente)

## Detalles de implementación

### Buddy System

El sistema Buddy implementado:
- Inicia con un bloque único de memoria de tamaño 2^N
- Divide los bloques recursivamente en "buddies" (pares) cuando se necesita un bloque de menor tamaño
- Fusiona bloques contiguos cuando se liberan para evitar la fragmentación
- Mantiene un seguimiento de los bloques asignados y disponibles

### Operaciones de imagen

- **Rotación**: Implementa rotación mediante interpolación bilineal para mantener la calidad
- **Escalado**: Utiliza interpolación bilineal para redimensionar la imagen manteniendo la proporción

## Comparación de rendimiento

El programa compara automáticamente:
- Tiempo de procesamiento con y sin Buddy System
- Consumo de memoria con ambos métodos
- Muestra los resultados al finalizar el procesamiento