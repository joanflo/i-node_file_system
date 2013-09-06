#include "bloques.h"

/*
* Las variables son estaticas para que no puedan ser modificadas en ficheros externos (directorios.c)
*/
static unsigned int fichero;//Descriptor de fichero
static int semaforo;

/*
* Hace el open del fichero que se usara como (sistema de archivos)
*/
int bmount (const char *camino) {
	fichero = open (camino, O_RDWR|O_CREAT, 0666);
	if (fichero == -1) {
		printf("Error al montar el sistema de ficheros.\n");
		return fichero;
	}
	iniciarSem(&semaforo); //Inicializamos el semaforo
	return fichero;
}

/*
* Hace el close del fichero (sistema de archivos)
*/
int bumount() {
	if(close (fichero) == -1){
		printf("Error al desmontar el sistema de ficheros.\n");
		return -1;
	}else{		
		eliminarSem(semaforo); //Eliminamos el semaforo
		return 0;
	}
}

/*
* Escribe un bloque en el dispositivo
*/
int bwrite (unsigned int bloque, const void *buf) {
	if (lseek(fichero, bloque*TAM_BLOQUE, SEEK_SET) == -1) {
		printf("Error al posicionar el puntero del fichero.\n");
		return -1;
	} else {
		int escritura = write(fichero, buf, TAM_BLOQUE);
		if (escritura == -1) {
			printf("Error al escribir en el fichero.\n");
        	}
        	return escritura;
	}
}

/*
* Lee un bloque del dispositivo
*/
int bread (unsigned int bloque, void *buf) {
	if (lseek(fichero, bloque*TAM_BLOQUE, SEEK_SET) == -1) {
		printf("Error al posicionar el puntero del fichero.\n");
		return -1;
	} else {
		int lectura = read(fichero, buf, TAM_BLOQUE);
		if (lectura == -1) {
			printf("Error al leer el fichero.\n");
		}
		return lectura;
	}
}

/*
* Hace el wait del semaforo encargado de la exclusion mutua
*/
void waitSemaforo() {
    waitSem(semaforo);
}

/*
* Hace el signal del semaforo encargado de la exclusion mutua
*/
void signalSemaforo() {
    signalSem(semaforo);
}
