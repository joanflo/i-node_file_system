#include "directorios.h"

// Cambia los permisos de un directorio/fichero
int main(int argc, char **argv) {    
	if (argc != 4) {
        	printf("mi_chmod: numero de argumentos incorrecto.\n");
        	return -1;
    	}    
    	char *fod = argv[2];//fod = fichero o directorio a cambiar los permisos
    	unsigned char permisos = atoi(argv[3]);//permisos en octal
	if(permisos > 7){
		printf("mi_chmod: el rango de permisos es 0..7\n");
	}
	bmount(argv[1]);    
	mi_chmod(fod, permisos);    
	bumount();    
	return 0;    
}
