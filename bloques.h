#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>      /* Modos de apertura y función open()*/
#include <stdlib.h>     /* Funciones write() y close() */
#include <string.h>     /* Funcion atoi() */
#include "semaforo.h"

#define TAM_BLOQUE 1024 //Tamaño del bloque = 1024 bytes

/*Etapa 1*/
int bmount(const char *camino);
int bumount();
int bwrite(unsigned int bloque, const void *buf);
int bread(unsigned int bloque, void *buf);
