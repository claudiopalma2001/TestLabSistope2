#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "broker.c"

// Entradas: Un puntero a imagen que es la imagen que se desea saturar y un factor de punto flotante que indica la cantidad de saturación que se desea aplicar a cada pixel. 
// Salidas: Un puntero a imagen con la imagen ya saturada según el factor entregado.
// Descripción: La función se encarga de guardar memoria para la nueva imagen binarizada saturada, posteriormente guarda memoria para un pixel de la imagen para ir alterandolos 
// posteriormente de forma que cada pixel es separado segun rojo, azul y verde. 
// Luego la imagen es recorrida y cada pixel RGB es multiplicado por el factor ingresado. 
// Una vez hecho esto se le asigna a la imagen nueva los pixeles nuevos y se retorna.


BMPImage* saturate_bmp(BMPImage* image, float factor) {
    BMPImage* new_image = (BMPImage*)malloc(sizeof(BMPImage));
    new_image->width = image->width;
    new_image->height = image->height;
    new_image->data = (RGBPixel*)malloc(sizeof(RGBPixel) * image->width * image->height);

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            RGBPixel pixel = image->data[y * image->width + x];
            pixel.r = (unsigned char)(pixel.r * factor);
            if(pixel.r>255){
                pixel.r=(unsigned char) 255;
            }
            pixel.g = (unsigned char)(pixel.g * factor);
            if(pixel.g > 255){
                pixel.g = 255;
            }
            pixel.b = (unsigned char)(pixel.b * factor);
            if(pixel.b > 255){
                pixel.b = 255;
            }
            new_image->data[y * image->width + x] = pixel;
        }
    }

    return new_image;
}

// Entradas: Un puntero a imagen que es la imagen a la que se desea aplicar escala de grises.
// Salidas: Un puntero a imagen con la imagen en escala de grises.
// Descripción: Caso similar a la función de saturar, se guarda espacio para la nueva imagen binarizada que se va a crear así como también para un pixel que represente el RGB de la imagen,
// solo que en este caso se crea una nueva variable donde cada pixel de la imagen, que fue recorrida mediante dos ciclos, es multiplicado por valores fijos que producen
// escala de grises, en este caso siendo pixel.red * 0.3, pixel.blue * 0.11 y pixel.green * 0.59.
// Una vez realizado todo este procedimiento se asignan los valores grises a cada pixel y luego se guardan en la nueva imagen binarizada y es retornada al usuario.

BMPImage* greyscale_bmp(BMPImage* image) {
    BMPImage* new_image = (BMPImage*)malloc(sizeof(BMPImage));
    new_image->width = image->width;
    new_image->height = image->height;
    new_image->data = (RGBPixel*)malloc(sizeof(RGBPixel) * image->width * image->height);

    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            RGBPixel pixel = image->data[y * image->width + x];
            unsigned char gray = (unsigned char)(0.3 * pixel.r + 0.59 * pixel.g + 0.11 * pixel.b);
            pixel.r = pixel.g = pixel.b = gray;
            new_image->data[y * image->width + x] = pixel;
        }
    }
    return new_image;
}

// Entradas: Un puntero a imagen que es la imagen que se desea saturar y un factor de punto flotante que indica el umbral al que se compara cada pixel, debe ser entre 0 y 1. 
// Salidas: Un puntero a imagen con la imagen ya binarizada.
// Descripción: Esta función binariza una imagen en escala de grises basada en un umbral especificado. 
// Primero, convierte la imagen original a escala de grises utilizando la función grayscale_bmp para simplificar el proceso de binarización
// y mejorar la consistencia en los resultados. 
// Luego, crea una nueva estructura BMPImage para almacenar la imagen binarizada y reserva memoria para los datos de la imagen. 
// Para binarizar la imagen se itera sobre cada píxel de la imagen en escala de grises y se compara su valor de gris con el umbral. 
// Si el valor de gris del píxel es mayor que el umbral, se asigna blanco al píxel en la imagen binarizada, en caso contrario (menor o igual al umbral), 
// se asigna negro al píxel en la imagen binarizada. 
// Una vez que se ha binarizado toda la imagen, se libera la memoria de la imagen en escala de grises y se retorna la imagen binarizada.

BMPImage* binarize_bmp(BMPImage* grayscale_image, float threshold) {
    //BMPImage* grayscale_image = greyscale_bmp(image); Convertir la imagen a escala de grises

    BMPImage* binarized_image = (BMPImage*)malloc(sizeof(BMPImage));
    binarized_image->width = grayscale_image->width;
    binarized_image->height = grayscale_image->height;
    binarized_image->data = (RGBPixel*)malloc(sizeof(RGBPixel) * grayscale_image->width * grayscale_image->height);

    for (int y = 0; y < grayscale_image->height; y++) {
        for (int x = 0; x < grayscale_image->width; x++) {
            RGBPixel pixel = grayscale_image->data[y * grayscale_image->width + x];
            // Binarizar el pixel basado en el umbral
            if (pixel.r > threshold * 255) {
                pixel.r = pixel.g = pixel.b = 255; // Blanco
            } else {
                pixel.r = pixel.g = pixel.b = 0; // Negro
            }
            binarized_image->data[y * grayscale_image->width + x] = pixel;
        }
    }
    

    return binarized_image;
}


int is_nearly_black(BMPImage* image, float threshold){
    int total_pixels = image->width * image->height;
    int black_pixels = 0;

    // Contar el número de píxeles negros en la imagen
    for (int y = 0; y < image->height; y++) {
        for (int x = 0; x < image->width; x++) {
            RGBPixel pixel = image->data[y * image->width + x];
            if (pixel.r <= 10 && pixel.g <= 10 && pixel.b <= 10){
                black_pixels++;
            }
        }
    }

    // Calcular el porcentaje de píxeles negros en la imagen
    float percentage_black = (float)black_pixels / total_pixels;

    // Comprobar si el porcentaje de píxeles negros supera el umbral
    if (percentage_black >= threshold) {
        return 1; // La imagen es casi negra
    } else {
        return 0; // La imagen no es casi negra
    }
}

// Entradas: Un arreglo de punteros a cadenas que contienen los nombres de las imágenes (char** image_names), 
// un arreglo de enteros que contiene las clasificaciones de las imágenes (int* classifications), 
// y el número total de imágenes (int num_images).
// Salidas: Ninguna (void).
// Descripción: Esta función crea un archivo CSV con los resultados de clasificación de las imágenes.
// Toma como entrada un arreglo dinámico de nombres de imágenes y otro de clasificaciones,
// junto con el número total de imágenes. Luego, crea un archivo CSV con dos columnas: una para los nombres de las imágenes
// y otra para sus respectivas clasificaciones. Cada fila del archivo CSV representa una imagen y su clasificación correspondiente.
// El archivo CSV se crea con el nombre especificado en el primer parámetro. Los datos se escriben en el archivo en formato CSV.
// Después de crear el archivo CSV, la función lo cierra correctamente.

// Deja el "filename" como el parametro "-R" que se pide
// El "image_names" es un arreglo de strings que tienes que crear de acuerdo a las imagenes que se pidieron, de ahi saca los nombres
// El "classifications" es un arreglo donde le calculas el is_nearly_black a las imagenes y en ese arreglo guardas los resultados
// EL "num_images" va directamente relacionado con la cantidad de filtros que pida en el "-f", por lo que lo sacas de ahí
void create_csv(const char* filename, char** image_names, int* classifications, int num_images) {
    FILE* file = fopen(filename, "w");
    if (!file) {
        fprintf(stderr, "Error: No se pudo abrir el archivo CSV.\n");
        return;
    }

    // Escribir encabezado del archivo CSV
    fprintf(file, "Nombre de la imagen,Clasificación\n");

    // Escribir datos de las imágenes y sus clasificaciones en el archivo CSV
    for (int i = 0; i < num_images; i++) {
        fprintf(file, "%s,%d\n", image_names[i], classifications[i]);
    }

    fclose(file);
}

void free_bmp(BMPImage* image) {
    if (image) {
        free(image->data);
        free(image);
    }
}

BMPImage* receive_image_from_pipe(int fd) {
    BMPImage* image = (BMPImage*)malloc(sizeof(BMPImage));
    read(fd, &image->width, sizeof(int)); // Leer el ancho de la imagen
    read(fd, &image->height, sizeof(int)); // Leer el alto de la imagen
    
    // Calcular el tamaño de los datos de píxeles
    int data_size = image->width * image->height * sizeof(RGBPixel);
    
    // Asignar memoria para los datos de píxeles
    image->data = (RGBPixel*)malloc(data_size);
    
    // Leer los datos de píxeles desde el pipe
    read(fd, image->data, data_size);
    
    return image;
}

void send_image_to_pipe(int fd, BMPImage* image) {
    // Escribir el ancho y alto de la imagen
    write(fd, &image->width, sizeof(int));
    write(fd, &image->height, sizeof(int));
    
    // Escribir los datos de píxeles
    int data_size = image->width * image->height * sizeof(RGBPixel);
    write(fd, image->data, data_size);
}



int main() {
    BMPImage* image = receive_image_from_pipe(STDIN_FILENO);
    float p, u;
    read(STDIN_FILENO, &p, sizeof(float));
    read(STDIN_FILENO, &u, sizeof(float));

    BMPImage* saturated_image = saturate_bmp(image, p);
    BMPImage* greyscale_image = greyscale_bmp(saturated_image);
    BMPImage* binarized_image = binarize_bmp(greyscale_image, u);

    send_image_to_pipe(STDOUT_FILENO, binarized_image);

    free_bmp(image);
    free_bmp(saturated_image);
    free_bmp(greyscale_image);
    free_bmp(binarized_image);

    return 0;
}
