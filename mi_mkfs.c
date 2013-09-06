#include "bloques.h"

int main(int argc, char **argv){
	int num_bloques = atoi(argv[2]);
	bmount(argv[1]);
	char buffer[TAM_BLOQUE];
	memset(buffer, 0, TAM_BLOQUE);
	int i;
	for(i=0;i<num_bloques;i++){
		bwrite(i,buffer);
	}
	initSB(num_bloques, num_bloques/4);
	initMB(num_bloques);
	initAI(num_bloques/4);

	//Reservamos el inodo del root:
	reservar_inodo('d', '7');

	bumount();
	return 0;
}
