#include <stdio.h>
#include <time.h>
#include "directorios.h"

int main (int argc, char **argv) {
	struct STAT metainf;
	int mascpermisos = 2;
	char permisos[3];
	struct tm *t;		//Puntero para transformar el tiempo en otro formato
	char time[128];

	bmount (argv[1]);
	if (mi_stat(argv[2], &metainf) == -1) {
		printf("mi_stat: error al leer la información del fichero\n");
	} else {
		printf("********************************************************\n");
		printf("%s\n",argv[2]);
		printf("********************************************************\n");
		if (metainf.tipo == 'd') {
			printf("Tipo: directorio\n");
		} else {
			printf("Tipo: fichero\n");
		}
		//De 4 a 7 el bit mas significativo es un 1
		if (metainf.permisos > 3) {
			permisos[0] = 'r';
		} else {
			permisos[0] = ' ';
		}
		//Comprobamos el ultimo bit (si modulo 2 != 0 estara a 1)
		if ((metainf.permisos % 2) != 0) {
			permisos[2] = 'x';
		} else {
			permisos[2] = ' ';
		}
		mascpermisos &= metainf.permisos;
		if (mascpermisos == 2) {
			permisos[1] = 'w';
		} else {
			permisos[1] = ' ';
		}
		printf("Permisos: %c %c %c\n", permisos[0], permisos[1], permisos[2]);
		t = localtime(&metainf.atime);
		printf("Último acceso a datos: %s", asctime(t));
		t = localtime(&metainf.mtime);
		printf("Última modificación de datos: %s", asctime(t));
		t = localtime(&metainf.ctime);
		printf("Última modificación del inodo: %s", asctime(t));
		printf("Numero de enlaces de la entrada en el directorio: %d\n", metainf.nlinks);
		printf("Tamaño en bytes: %d\n", metainf.tamEnBytesLog);
		printf("Numero de bloques ocupados: %d\n", metainf.numBloquesOcupados);
		printf("********************************************************\n");
	}
	bumount();
}
