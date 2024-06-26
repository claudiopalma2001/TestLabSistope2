#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

/*
LABORATORIO DESARROLLADO POR 

CLAUDIO MATIAS PALMA PEREZ SECCION ACOSTA 20.680.606-0
DAVID ENRIQUE MARTINEZ ESCARES 20.882.683-2

CONSIDERACIONES:

Cuando se genera la carpeta con las imagenes no se genera de inmediato las imagenes
debe entrar y salir varias veces de la carpeta que se genera para que se muestren las que Ud. generó

Laboratorio basado en el aporte de codigo que mandó el ayudante Ian Rickmers

Laboratorio testeado en Ubuntu 22.04.4 LTS
*/

int main(int argc, char *argv[]) {
    // Se crean las variables para el getopt, la clasificacion de las imagenes y la creacion de estas
    char N[100] = "";
    char R[100] = "";
    int f = 3;
    float p = 1.3;
    float u = 0.5;
    float v = 0.5;
    char C[100] = "";
    pid_t pid;
    int W = 1;

    int option;
    int mandatoryN = 0;
    int mandatoryC = 0;
    int mandatoryR = 0;

    int underscorePosition;
    char exampleFilename[10];
    char *underscorePtr;

    char bufferF[10];
    char bufferP[10]; 
    char bufferU[10]; 
    char bufferV[10];
    char bufferW[10];   

    while ((option = getopt(argc, argv, "N:f:p:u:v:C:R:W:")) != -1) {
        switch (option) {
            case 'N':
                strcpy(N, optarg);
                mandatoryN = 1;
                break;
            case 'f':
                f = atoi(optarg);
                break;
            case 'p':
                p = atof(optarg);
                break;
            case 'u':
                u = atof(optarg);
                break;
            case 'v':
                v = atof(optarg);
                break;
            case 'C':
                strcpy(C, optarg);
                mandatoryC = 1;
                break;
            case 'R':
                strcpy(R, optarg);
                mandatoryR = 1;
                break;
            case 'W':
                W = atoi(optarg);
                break;
            default:
                fprintf(stderr, "Uso: %s -N <imagen> -f <cantidad_filtros> -p <factor_saturacion> -u <umbral_binarizacion> -v <umbral_clasificacion> -C <carpeta_resultante> -R <archivo_csv> -W <cantidad_workers>\n", argv[0]);
                return 1;
        }
    }

    if (!mandatoryN || !mandatoryC || !mandatoryR) {
        fprintf(stderr, "Los parámetros -N, -C y -R son obligatorios.\n");
        return 1;
    }

    if (f <= 0 || f > 3) {
        printf("La flag -f solo puede tomar los valores 1, 2, o 3\n");
        return 1;
    }

    if (p <= 0) {
        printf("La flag -p no puede tomar el valor 0 o menor que este\n");
        return 1;
    }

    if (u < 0 || u > 1) {
        printf("La flag -u puede tomar valores entre 0 y 1 (incluyéndolos)\n");
        return 1;
    }

    if (v < 0 || v > 1) {
        printf("La flag -v puede tomar valores entre 0 y 1 (incluyéndolos)\n");
        return 1;
    }

    pid = fork();
    if (pid == 0) {
        sprintf(bufferF, "%d", f);
        sprintf(bufferP, "%f", p);
        sprintf(bufferU, "%f", u);
        sprintf(bufferV, "%f", v);
        sprintf(bufferW, "%i", W);

        char *broker_args[] = {"./broker", N, bufferF, bufferP, bufferU, bufferV, bufferW, C, R, NULL};
        execv(broker_args[0], broker_args);
        perror("Error en execv");
        exit(1);
    } else if (pid > 0) {
        printf("Soy el proceso padre (MAIN)\n");
        waitpid(pid, NULL, 0);
    } else {
        perror("fork");
        exit(1);
    }

    printf("Terminó el proceso principal (MAIN)\n");
    return 0;
}
