#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "worker.c"

typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPHeader;

typedef struct {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t size_image;
    int32_t x_pixels_per_meter;
    int32_t y_pixels_per_meter;
    uint32_t colors_used;
    uint32_t colors_important;
} BMPInfoHeader;

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} RGBPixel;

typedef struct {
    int width;
    int height;
    RGBPixel *data;
} BMPImage;

void send_image_fragment(int fd, BMPImage* image, int start_col, int end_col) {
    int fragment_width = end_col - start_col;
    int fragment_size = fragment_width * image->width * sizeof(RGBPixel); // Utilizamos image->width

    // Enviar la cabecera
    BMPInfoHeader info_header = {
        .size = sizeof(BMPInfoHeader),
        .width = fragment_width, // Ancho del fragmento
        .height = image->height, // Alto de la imagen completa
        .planes = 1,
        .bit_count = 24, // Asumiendo formato BMP de 24 bits
        .compression = 0,
        .size_image = fragment_size, // Tamaño del fragmento
        .x_pixels_per_meter = 0,
        .y_pixels_per_meter = 0,
        .colors_used = 0,
        .colors_important = 0
    };

    write(fd, &info_header, sizeof(BMPInfoHeader));

    // Enviar los datos del fragmento
    for (int y = 0; y < image->height; y++) {
        write(fd, &image->data[y * image->width + start_col], fragment_width * sizeof(RGBPixel));
    }
}

BMPImage* receive_image_fragment(int fd, int width, int height) {
    BMPImage* fragment = (BMPImage*)malloc(sizeof(BMPImage));
    fragment->width = width;
    fragment->height = height;
    fragment->data = (RGBPixel*)malloc(width * height * sizeof(RGBPixel));

    BMPInfoHeader info_header;
    read(fd, &info_header, sizeof(BMPInfoHeader)); // Leemos la cabecera del fragmento

    for (int y = 0; y < height; y++) {
        read(fd, &fragment->data[y * width], width * sizeof(RGBPixel));
    }

    return fragment;
}

void assemble_image(BMPImage* image, BMPImage** fragments, int num_fragments) {
    int fragment_width = image->width / num_fragments;
    int current_col = 0;

    for (int i = 0; i < num_fragments; i++) {
        for (int y = 0; y < image->height; y++) {
            memcpy(&image->data[y * image->width + current_col], 
                   &fragments[i]->data[y * fragment_width], 
                   fragment_width * sizeof(RGBPixel));
        }
        current_col += fragment_width;
    }
}

int main(int argc, char *argv[]) {
    if (argc != 9) {
        fprintf(stderr, "Uso: %s <nombre_archivo.bmp> <f> <p> <u> <v> <W> <C> <R>\n", argv[0]);
        exit(1);
    }

    char* N = argv[1];
    int f = atoi(argv[2]);
    float p = atof(argv[3]);
    float u = atof(argv[4]);
    float v = atof(argv[5]);
    int W = atoi(argv[6]);
    char C[100];
    char R[100];
    strcpy(C, argv[7]);
    strcpy(R, argv[8]);

    BMPImage* image = read_bmp(N);
    if (!image) {
        exit(1);
    }

    int columns_per_worker = image->width / W;
    int remaining_columns = image->width % W;

    pid_t workers[W];
    int pipes[W][2];

    for (int i = 0; i < W; i++) {
        pipe(pipes[i]);
        workers[i] = fork();
        if (workers[i] == 0) {
            close(pipes[i][1]); // Cerramos el extremo de escritura en el hijo
            dup2(pipes[i][0], STDIN_FILENO); // Redirigir la entrada estándar para leer desde el pipe
            close(pipes[i][0]); // Buena práctica

            char *worker_args[] = {"./worker", NULL};
            execv(worker_args[0], worker_args);
            perror("Error en execv");
            exit(1);
        } else if (workers[i] < 0) {
            perror("fork");
            exit(1);
        }
    }

    for (int i = 0; i < W; i++) {
        close(pipes[i][0]); // Cerramos el extremo de lectura en el padre

        int start_col = i * columns_per_worker;
        int end_col = (i == W - 1) ? (i + 1) * columns_per_worker + remaining_columns : (i + 1) * columns_per_worker;
        
        send_image_fragment(pipes[i][1], image, start_col, end_col);
        write(pipes[i][1], &p, sizeof(float)); // Factor de saturación
        write(pipes[i][1], &u, sizeof(float)); // Umbral de binarización
        close(pipes[i][1]); // Buena práctica
    }

    BMPImage* fragments[W];
    for (int i = 0; i < W; i++) {
        int status = 0;
        waitpid(workers[i], &status, 0);
        if (WIFEXITED(status)) {
            fragments[i] = receive_image_fragment(pipes[i][0], columns_per_worker, image->height);
        }
    }

    assemble_image(image, fragments, W);
    write_bmp_nopointer("output_image.bmp", *image);

    free_bmp(image);
    for (int i = 0; i < W; i++) {
        free_bmp(fragments[i]);
    }

    printf("Broker terminó su trabajo\n");
    return 0;
}
