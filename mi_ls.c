#include "directorios.h"

int main(int argc, char **argv) {
    	if (argc != 3) {
        	printf("mi_ls: el numero de argumentos no es el correcto.\n");
        	return -1;
    	}
	if(argv[2][strlen(argv[2])-1] != '/'){
		printf("mi_ls: %s corresponde al path de un fichero\n",argv[2]);
		return -1;
	} 
    	char *directorio = argv[2];    
    	bmount(argv[1]);    
    	int elementosAMostrar;
    	struct STAT stat;
    	char rutaConFichero[1000];
    	char nombreFichero[60];
    	char *finFichero, *inicioFichero;            
    	if (mi_stat(directorio, &stat) > -1) {
        	char buffer[(60+1)*(stat.tamEnBytesLog/sizeof(struct entrada))];
        	if ((elementosAMostrar = mi_dir(directorio, buffer)) > -1) {
            	inicioFichero = buffer;
			printf("Hay %i elementos a mostrar.\n",elementosAMostrar);
            		while (elementosAMostrar > 0) {
                		finFichero = strchr(inicioFichero, '|');
                		strncpy(nombreFichero, inicioFichero, finFichero - inicioFichero);
                		rutaConFichero[0] = '\0';
                		nombreFichero[finFichero - inicioFichero] = '\0';
                		inicioFichero = finFichero + 1;
                		strcpy(rutaConFichero, directorio);
                		strcat(rutaConFichero, nombreFichero);
                		mi_stat(rutaConFichero, &stat);
                		if (stat.tipo == 'd') {
                    			printf("%c[%d;%dm",27,1,34);
                		} else {
                    			printf("%c[%d;%dm",27,1,32);
                		}
                		printf("%s\n", nombreFichero);
                		elementosAMostrar--;
            		}
            		printf("%c[%dm",27,0);
        	} else {
			printf("El directorio esta vacio.\n");
		}
    	}    
    	bumount();    
    	return 0;    
}
