#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char** argv) {
    FILE* fpBootloader;
    FILE* fpSketch;
    FILE* fpOutput;
    int sketchSize;
    int bootloaderSize;
    uint8_t* buf;
    const int SKETCH_OFFSET = 0x5000;


    if (argc != 4) {
        printf("\n%s Incorrect number of args\n\n", argv[0]);
        return -1;
    }

    // Открываем загрузчик
    fpBootloader = fopen(argv[1], "rb");
    if (fpBootloader == NULL) {
        printf("Unable to open bootloader binary file %s\n", argv[1]);
        return -2;
    }
    fseek(fpBootloader, 0, SEEK_END);
    bootloaderSize = ftell(fpBootloader);
    fseek(fpBootloader, 0, SEEK_SET);
    printf("Opened bootloader file %s : size is %d\n", argv[1], bootloaderSize);

    //Открываем пользовательскую программу
    fpSketch = fopen(argv[2], "rb");
    if (fpSketch == NULL) {
        fclose(fpBootloader);
        printf("Unable to open sketch binary file %s\n", argv[2]);
        return -2;
    }
    fseek(fpSketch, 0, SEEK_END);
    sketchSize = ftell(fpSketch);
    fseek(fpSketch, 0, SEEK_SET);
    printf("Opened sketch file %s : size is %d\n", argv[2], sketchSize);

    //Создаем буфер
    buf = (uint8_t*)malloc(SKETCH_OFFSET + sketchSize);
    if (buf == NULL) {
        fclose(fpBootloader);
        fclose(fpSketch);
        printf("Unable to open allocate memory\n");
        return -3;
    }
    printf("Allocated %d bytes for buffer\n", SKETCH_OFFSET + sketchSize);

    //Читаем файлы программ в буфер
    fread(buf, bootloaderSize, 1, fpBootloader);
    fclose(fpBootloader);

    fread(buf + SKETCH_OFFSET, sketchSize, 1, fpSketch);
    fclose(fpSketch);

    //Открываем файл на выход
    fpOutput = fopen(argv[3], "wb");
    if (fpOutput == NULL) {
        fclose(fpBootloader);
        fclose(fpSketch);
        printf("Unable to open output file %s for writing \n", argv[3]);
        return -2;
    }

    //Записываем выходной файл
    fwrite(buf, SKETCH_OFFSET + sketchSize, 1, fpOutput);
    fclose(fpOutput);
    printf("Wrote combined file to %s \n", argv[3]);

    return 0;
}
