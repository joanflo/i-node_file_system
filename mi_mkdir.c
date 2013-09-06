#include "directorios.h"

int main(int argc, char **argv) {    
    if (argc != 4) {
        printf("mi_mkdir: numero de argumentos incorrecto\n");
        return -1;
    }    
    char *camino = argv[2];
    char permisos = atoi(argv[3]);    
    // Abrimos el sistema de ficheros
    bmount(argv[1]);
    //mi_mkdir no puede crear ficheros
    if (camino[strlen(camino)-1]=='/') {
        mi_creat(camino, permisos);
	printf("mi_mkdir: directorio creado con exito\n");
    } else {
        printf("mi_mkdir: el camino no es un directorio\n");
    }    
    bumount();    
    return 0;    
}
