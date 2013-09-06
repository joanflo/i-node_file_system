#include <signal.h>
#include <sys/wait.h>
#include <limits.h>

#include "directorios.h"

#define NUM_PROCESOS 100
#define NUM_ESCRITURAS 50
#define BUFFER 1048576


int procesos_acabados;

struct registro {
	time_t fecha;
	pid_t pid_proceso;
	unsigned int num_escritura;
	unsigned int num_registro;
};

/* 
* Elimina los procesos hijo en estado zombie (despues de enviar SIGCHLD)
*/
void enterrador () {
    while (wait3(NULL, WNOHANG, NULL)>0) {
        procesos_acabados++;
    }
}

int main(int argc, char **argv) {    
	if (argc != 2) {
		printf("simulacion: numero de argumentos incorrecto\n");
        	return -1;
    	}    
    	int fichero;
    	//Se envia la señal y se llama al enterrador
    	signal(SIGCHLD, enterrador);
    
    	bmount(argv[1]);    
    	procesos_acabados = 0;
    
    	//'simul_aaaammddhhmmss'
    	char nombre_dir[22];
    	struct tm *tm;
   	 time_t hora = time(NULL);
    	tm = localtime(&hora);
    	sprintf(nombre_dir,"/simul_%d%02d%02d%02d%02d%02d/",tm->tm_year+1900, tm->tm_mon+1,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
    	mi_creat(nombre_dir, 7);
    	unsigned int maximo_registros = UINT_MAX/sizeof(struct registro);
    
	printf("simulacion: empieza la simulacion de escrituras.\n");
	int i;
    	for (i = 0; i < NUM_PROCESOS; i++) {
        	if (fork() == 0) {
            		int pid_proceso = getpid();            
            		char nombre_fich[60+80];
            		sprintf(nombre_fich,"%sproceso_%i/pruebas.dat",nombre_dir,pid_proceso);
            		mi_creat(nombre_fich, 7);
            		int j;
            		struct registro reg;
            		srand(time(NULL)*pid_proceso);            
            		for (j=1; j<=NUM_ESCRITURAS; j++) { //Para toda escritura
                		reg.fecha = time(NULL);
                		reg.pid_proceso = pid_proceso;
                		reg.num_escritura = j;
                		reg.num_registro = rand() % maximo_registros; //calculo de la posicion aleatoria
                		if (mi_write(nombre_fich, &reg, reg.num_registro*sizeof(struct registro), sizeof(struct registro))==-1) {
                    			printf("simulacion: la escritura del registro %i del proceso %i ha provocado un error\n",j,pid_proceso);
                		}
                		usleep(50000);
           		}
           		printf("El proceso %i ha acabado de escribir.\nQuedan %i procesos para acabar.\n",pid_proceso,NUM_PROCESOS-i-1);
           		exit(0);
        	}
        	usleep(200000);//Duerme durante 0.2 segundos
    	}
    
    	while (procesos_acabados < NUM_PROCESOS) {//Hasta que no hayan acabado las escrituras
        	pause();
    	}    
    	printf("simulacion: la simulacion de escrituras ha acabado\nsimulacion: empieza la verificacion de escrituras\n");
     
    	char path[60+80];
    	char subdirectorios[60*100];
    
    	sprintf(path,"%s",nombre_dir);
    	if (mi_dir(path, subdirectorios)==-1) {
        	printf("simulacion: no se han podido obtener los directorios de los procesos\n");
        	return -1;
    	}    
    	char *posFinFichero;
    	char *posInicioFichero = subdirectorios;
    	struct registro registro;
    	unsigned int posRegistro;
    	time_t primeraEscritura , ultimaEscritura;
    	int pid;
    	char cpid[4];
    	for (i=0; i<NUM_PROCESOS; i++) {
        	posFinFichero = strchr(subdirectorios, '|');
        	*posFinFichero = '\0'; 
        	memcpy(path + 22, posInicioFichero, posFinFichero - posInicioFichero + 1);
        	sprintf(path + 21, "/%s/pruebas.dat", posInicioFichero);
        	*posFinFichero = '_'; 
        	posInicioFichero = posFinFichero + 1;		
        	struct registro registroActual[BUFFER/sizeof(struct registro)];
        	memset(registroActual, 0, BUFFER);        
        	memcpy(cpid, path+30, 4);
        	pid = atoi(cpid);
		printf("*Verficando las escrituras del proceso %i...\n",pid);
        	struct registro primeraEscritura, ultimaEscritura;
        	primeraEscritura.num_escritura = 51;
        	ultimaEscritura.num_escritura = 0;        
        	posRegistro = 0;
        	struct registro primerRegistro, ultimoRegistro;
        	primerRegistro.num_registro = maximo_registros;
        	ultimoRegistro.num_registro = 0;
        	int registrosValidados = 0;
        	int j;        
        	// Leemos el fichero
        	while (mi_read(path, &registroActual, posRegistro*sizeof(struct registro), BUFFER)>0 && posRegistro<maximo_registros){
            		for (j = 0; j<BUFFER/sizeof(struct registro); j++) {
                		if (registroActual[j].pid_proceso == pid) {
                    			registrosValidados++;
                    			if (primeraEscritura.num_escritura > registroActual[j].num_escritura) {
                        			primeraEscritura.fecha = registroActual[j].fecha;
                        			primeraEscritura.num_escritura = registroActual[j].num_escritura;
                        			primeraEscritura.num_registro = registroActual[j].num_registro;
                    			} else if (ultimaEscritura.num_escritura < registroActual[j].num_escritura) {
                        			ultimaEscritura.fecha = registroActual[j].fecha;
                        			ultimaEscritura.num_escritura = registroActual[j].num_escritura;
                        			ultimaEscritura.num_registro = registroActual[j].num_registro;
                    			}
                    			if (primerRegistro.num_registro > registroActual[j].num_registro) {
                        			primerRegistro.fecha = registroActual[j].fecha;
                        			primerRegistro.num_escritura = registroActual[j].num_escritura;
                        			primerRegistro.num_registro = registroActual[j].num_registro;
                    			} else if (ultimoRegistro.num_registro < registroActual[j].num_registro) {
                        			ultimoRegistro.fecha = registroActual[j].fecha;
                        			ultimoRegistro.num_escritura = registroActual[j].num_escritura;
                        			ultimoRegistro.num_registro = registroActual[j].num_registro;
                    			}
                		}
                		posRegistro++;
            		}
            		memset(registroActual, 0, BUFFER);
        	}  
        	// Mostramos la información de los registros del proceso
		printf("**********************************************\n");
		printf("*** 		PROCESO %i               ***\n", pid);
		printf("**********************************************\n");
		printf("> Número de escrituras verficadas: %i\n", registrosValidados);
		printf("> Primera escritura: \n");
		printf("  Fecha: %s", asctime(localtime(&primeraEscritura.fecha)));
		printf("  Num. escritura: %i\n", primeraEscritura.num_escritura);
		printf("  Num. registro: %i\n", primeraEscritura.num_registro);
		printf("> Última escritura: \n");
		printf("  Fecha: %s", asctime(localtime(&ultimaEscritura.fecha)));
		printf("  Num. escritura: %i\n", ultimaEscritura.num_escritura);
		printf("  Num. registro: %i\n", ultimaEscritura.num_registro);
		printf("> Menor posición: \n");
		printf("  Fecha: %s", asctime(localtime(&primerRegistro.fecha)));
		printf("  Num. escritura: %i\n", primerRegistro.num_escritura);
		printf("  Num. registro: %i\n", primerRegistro.num_registro);
		printf("> Mayor posición: \n");
		printf("  Fecha: %s", asctime(localtime(&ultimoRegistro.fecha)));
		printf("  Num. escritura: %i\n", ultimoRegistro.num_escritura);
		printf("  Num. registro: %i\n", ultimoRegistro.num_registro);
		printf("**********************************************\n");
    	}    
    	bumount();
    	return 0;    
}
