#include <stdio.h>
#include "directorios.h"

int main (int argc, char **argv) {
	if(argc != 3){
		printf("mi_rm: numero de argumentos incorrecto\n");
		return -1;
	}
	if((argv[2])[0] == '/' && strlen(argv[2]) == 1){
		printf("mi_rm: no puedes remover la raiz del sistema de ficheros\n");
		return -1;
	}
	bmount (argv[1]);
	if (mi_unlink(argv[2]) == -1) {
		printf("mi_rm: hubo un error al borrar el archivo\n");
	} else {
		printf("mi_rm: archivo borrado correctamente\n");
	}
	bumount(argv[1]);
	return 0;
}
