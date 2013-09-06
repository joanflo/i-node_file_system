#include <stdio.h>
#include "directorios.h"

int main (int argc, char **argv) {
	if(argc != 4){
		printf("mi_ln: numero de argumentos incorrecto\n");
		return -1;
	}
	bmount (argv[1]);
	if (mi_link(argv[2], argv[3]) == -1) {
		printf("mi_ln: hubo un error durante la creacion del enlace físico\n");
	} else {
		printf("mi_ln: el enlace físico ha sido realizado con exito\n");
	}
	bumount();
}


