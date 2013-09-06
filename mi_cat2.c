#include "directorios.h"

#define TAM_BUF 524288//0.5MB 

int main(int argc, char **argv) {
	if (argc != 4) { 
		printf("mi_cat: numero de argumentos incorrecto\n");
		return -1;
    	}
	if(argv[2][strlen(argv[2])-1] == '/'){
		printf("mi_cat: %s corresponde al path de un directorio\n",argv[2]);
		return -1;
	}    
    	bmount(argv[1]);    
    	unsigned int i = 0;//variable incremental auxiliar para calcular el offset
    	char buffer [TAM_BUF];//buffer de lectura para no cargar la memoria principal con lecturas pesadas
    	int lecturaB;
	int totalLeidos = 0; //lecturaB(ytes): variable para controlar el resultado de las lecturas e imprimirlas sin llamadas innecesarias a mi_read
	char *archivo = argv[2];
    	while ((lecturaB = mi_read(archivo, buffer, i*TAM_BUF, TAM_BUF))>0 && totalLeidos < atoi(argv[3]) ) {
        	fwrite(buffer,sizeof(char),lecturaB,stdout); //volcado del buffer en la salida estandar
        	i++;
		totalLeidos+=lecturaB;
    	}
    	bumount();    
   	return 0;    
}
