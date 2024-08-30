#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char** argv) {
    FILE* fpBootloader;
    FILE* fpSketch;
    FILE* fpOutput;
    FILE* fpUARTLoader;
    int sketchSize;
    int bootloaderSize;
    int uartloaderSize;
    uint8_t* buf;
    const int SKETCH_OFFSET = 0x2000;
    const int MY_OFFSET = 0x1B000;


    if (argc != 5) {
        printf("\n%s Usage. mergebin bootloader.bin sketch.bin output.bin - Incorrect number of args\n\n", argv[0]);
        return -1;
    }

    // Открываем bootloader
    fpBootloader = fopen(argv[1], "rb");
    if (fpBootloader == NULL) {
        printf("Unable to open bootloader binary file %s\n", argv[1]);
        return -2;
    }
    fseek(fpBootloader, 0, SEEK_END); // seek to send of bootloader to determine its length
    bootloaderSize = ftell(fpBootloader);
    fseek(fpBootloader, 0, SEEK_SET); // go back to start, so we can read from the beginning
    printf("Opened bootloader file %s : size is %d\n", argv[1], bootloaderSize);

    //Открываем пользовательскую программу
    fpSketch = fopen(argv[2], "rb");
    if (fpSketch == NULL) {
        fclose(fpBootloader);
        printf("Unable to open sketch binary file %s\n", argv[2]);
        return -2;
    }
    fseek(fpSketch, 0, SEEK_END); // seek to send of sketch to determine its length
    sketchSize = ftell(fpSketch);
    fseek(fpSketch, 0, SEEK_SET); // go back to start, so we can read from the beginning
    printf("Opened sketch file %s : size is %d\n", argv[2], sketchSize);

    //Открываем UART loader
    fpUARTLoader = fopen(argv[3], "rb");
    if (fpUARTLoader == NULL) {
        fclose(fpBootloader);
        fclose(fpSketch);
        printf("Unable to open UART loader binary file %s\n", argv[3]);
        return -2;
    }
    fseek(fpUARTLoader, 0, SEEK_END); // seek to send of sketch to determine its length
    uartloaderSize = ftell(fpUARTLoader);
    fseek(fpUARTLoader, 0, SEEK_SET); // go back to start, so we can read from the beginning
    printf("Opened UART loader file %s : size is %d\n", argv[3], uartloaderSize);

    //Создаем буфер
    buf = (uint8_t*)malloc(MY_OFFSET + uartloaderSize);
    if (buf == NULL) {
        fclose(fpBootloader);
        fclose(fpSketch);
        fclose(fpUARTLoader);
        printf("Unable to open allocate memory\n");
        return -3;
    }
    printf("Allocated %d bytes for buffer\n", MY_OFFSET + uartloaderSize);

    //Читаем файлы программ в буфер
    fread(buf, bootloaderSize, 1, fpBootloader);
    fclose(fpBootloader);

    fread(buf + SKETCH_OFFSET, sketchSize, 1, fpSketch);
    fclose(fpSketch);

    fread(buf + MY_OFFSET, uartloaderSize, 1, fpUARTLoader);
    fclose(fpUARTLoader);

    //Открываем файл на выход
    fpOutput = fopen(argv[4], "wb");
    if (fpOutput == NULL) {
        fclose(fpBootloader);
        fclose(fpSketch);
        fclose(fpUARTLoader);
        printf("Unable to open output file %s for writing \n", argv[4]);
        return -2;
    }

    //Записываем выходной файл
    fwrite(buf, MY_OFFSET + uartloaderSize, 1, fpOutput);
    fclose(fpOutput);
    printf("Wrote combined file to %s \n", argv[4]);

    return 0;
}