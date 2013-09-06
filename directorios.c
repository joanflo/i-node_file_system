#include "directorios.h"
#include <string.h>


/* Dada una cadena de caracteres camino (que comience por '/'), separa su
 * contenido en dos: inicial y final.
 * > INICIAL. Guarda la porción de camino comprendida entre los dos primeros '/'
 * (entonces inicial se trata del nombre de un directorio). Si no hay segundo '/'
 * copia camino en inicial sin el primer '/' (entonces inicial se trata del nombre
 * de un fichero).
 * La función debe devolver un valor que indique si en inicial hay un nombre de
 * directorio o un nombre de fichero (devolveremos un char: d=directorio, f=fichero).
 * > FINAL: Guarda el resto de camino a partir del segundo '/' inclusive (en caso
 * de directorio, porque en caso de fichero no guarda nada).
 */
int extraer_camino(const char *camino, char *inicial, char *final){
	char *dir;
	if(camino[0] != '/'){
		printf("Camino no valido.");
		return -1;
	}else{
		camino++;
		dir = strchr(camino,'/');
		if(dir != NULL){
			strncpy(inicial,camino,(dir-camino)+1);
			inicial[dir-camino]='\0';
			camino += dir-camino; 
			strcpy(final,camino);
			return 0;
		}else{
			strcpy(inicial,camino);
			strcpy(final, "");
			return 1;
		}
	}
}

/*Dada una cadena de caracteres (camino_parcial) y el inodo del directorio sobre el que se apoya esta cadena (p_inodo_dir), calcula:
* El número de inodo de su directorio más cercano (p_inodo_dir).
* Su número de inodo (p_inodo).
El número de su entrada dentro del último directorio que lo contiene (p_entrada).
*/
int buscar_entrada(const char *camino_parcial, unsigned int *p_inodo_dir, unsigned int *p_inodo, unsigned int *p_entrada, char reservar, unsigned char modo) {
	if (strcmp(camino_parcial,"/")==0) { // El directorio es el raíz
		*p_inodo = 0;
		*p_entrada = 0;
		return 0;
	}
	char inicial[60];
	char final[500];
	char tipo;
	char trobada;
	if ((tipo = extraer_camino(camino_parcial,inicial,final)) == -1) {//Si la extraccion de camino falla, abortamos la busqueda 
		return -1;
	}
    
	struct inodo inodo;
	inodo = leer_inodo(*p_inodo_dir);//Leemos el directorio
    
	struct entrada entrada;
	entrada.nombre[0] == '\0';    
	unsigned int numentradas = inodo.tamEnBytesLog/sizeof(struct entrada);
	unsigned int nentrada = 0;
	if (numentradas > 0) {//Si el directorio tiene entradas, abortamos
		if (mi_read_f(*p_inodo_dir,&entrada,0,sizeof(struct entrada)) == -1) {
			return -2;
		}
		nentrada++;
		while ((nentrada < numentradas) && (strcmp (inicial,entrada.nombre)!=0)) {//Para todas las entradas del directorio...
			mi_read_f(*p_inodo_dir,&entrada,nentrada*sizeof(struct entrada),sizeof(struct entrada));//...leer la entrada
			nentrada++;
		}
		if (strcmp (inicial,entrada.nombre)==0) {
			trobada=1;
		} else {
			trobada=0;
		}
	} else {
		trobada=0;   
	}
    	//Si despues de recorrer la tabla, no la hemos encontrado debemos comprobar el parametro reservar (para crearla en el caso necesario)
	if (trobada == 0) { 
		switch (reservar) {
			case 0: // Modo consulta. 
				return -4;
				break;
			case 1: // Modo escritura. 
				strcpy(entrada.nombre, inicial);
				if (tipo == 0) { // Directorio
					if (strcmp(final,"/")==0) {
						entrada.inodo = reservar_inodo('d',modo);
					} else {
						entrada.inodo = reservar_inodo('d',7);
					}
				} else { // Fichero
					entrada.inodo = reservar_inodo('f',modo);
				}
                
				if (mi_write_f(*p_inodo_dir,&entrada,nentrada*sizeof(struct entrada),sizeof(struct entrada)) == -1) { // Error de escritura
					if (entrada.inodo != -1) {
						liberar_inodo(entrada.inodo);
					}
					return -3;
				}
				nentrada++;
				break;
		}
	}
	//Iteracion final
	if (strcmp(final,"/") == 0 || strcmp(final,"") == 0) { 
		*p_inodo = entrada.inodo;
		if ((trobada==1) && (reservar==1)) { 
			*p_entrada = nentrada;
			return -5;
		}
		*p_entrada = nentrada-1;
		return 0;
	} else {//Sino, llamada recursiva con el resto del camino
		*p_inodo_dir = entrada.inodo;
		return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, modo);
	}
}

/*
* Crea un fichero/directorio y su entrada de directorio.
* Se basa, principalmente, en la función buscar_entrada().
*/
int mi_creat(const char *camino, unsigned char modo) {
	unsigned int p_inodo_dir = 0, p_inodo;
	unsigned int p_entrada;    
	waitSemaforo();
	switch (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 1, modo)) {
		case -1:
			printf("El path no es válido.\n");
			signalSemaforo();
			return -1;
			break;
		case -2:
			printf("Algun elemento del camino no tiene permisos de lectura.\n");
			signalSemaforo();
			return -1;
			break;
		case -3:
			printf("Algun elemento camino no tiene permisos de escritura.\n");
			signalSemaforo();
			return -1;
			break;
		case -5:
			printf("La entrada ya existia.\n");
			signalSemaforo();
			return -1;
			break;
		default:
			signalSemaforo();
			return 0;
			break;
	}       
}

/*
* Crea el enlace de una entrada de directorio camino2 al inodo especificado por otra entrada de directorio camino1
*/
int mi_link(const char *camino1, const char *camino2) {
	unsigned int p_inodo_dir = 0, p_inodo1, p_inodo2, p_entrada;   
	waitSemaforo();
	switch (buscar_entrada(camino1, &p_inodo_dir, &p_inodo1, &p_entrada, 0, 7)) {
		case -1:
			printf("El path de origen no es válido.\n");
			signalSemaforo();
			return -1;
			break;
		case -2:
			printf("Algun elemento del origen no tiene permisos de lectura.\n");
			signalSemaforo();
			return -1;
			break;
		case -4:
			printf("El path origen no existe.\n");
			signalSemaforo();
			return -1;
			break;
		default: 
			switch (buscar_entrada(camino2, &p_inodo_dir, &p_inodo2, &p_entrada, 1, 7)) {
				case -1:
					printf("El destino no es es un path correcto.\n");
					signalSemaforo();
					return -1;
					break;
				case -2:
					printf("Algun elemento del destino no tiene permisos de lectura.\n");
					signalSemaforo();
					return -1;
					break;
				case -3:
					printf("Algun element del destino no tiene permisos de escritura.\n");
					signalSemaforo();
					return -1;
				case -5:
					printf("Ya existe el directorio/fichero.\n");
					signalSemaforo();
					return -1;
					break;
				default:
					liberar_inodo(p_inodo2);
					struct entrada entrada;
					mi_read_f(p_inodo_dir,&entrada,p_entrada*sizeof(struct entrada),sizeof(struct entrada));
					entrada.inodo = p_inodo1;
					mi_write_f(p_inodo_dir,&entrada,p_entrada*sizeof(struct entrada),sizeof(struct entrada));
					struct inodo inodo1 = leer_inodo(p_inodo1);
					inodo1.nlinks++;
					inodo1.ctime = time(NULL);
					escribir_inodo(inodo1, p_inodo1);
					signalSemaforo();
					return 0;
					break;
			}
			break;
	}
}

/*
* Borra la entrada de directorio especificada (no hay que olvidar actualizar la cantidad de enlaces de entradas en directorio del inodo) y, 
* en caso de que sea el último enlace existente, borrar el propio fichero/directorio.
*/
int mi_unlink(const char *camino) {
    unsigned int p_inodo_dir = 0, p_inodo, p_entrada;
    struct inodo inodo_padre;
    int numentradas;    
    waitSemaforo();    
    switch (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada,0, 7)) {
        case -1:
            printf("El path no es correcto.\n");
            signalSemaforo();
            return -1;
            break;
        case -2:
            printf("Algun elemento del path no tiene permisos de lectura.\n");
            signalSemaforo();
            return -1;
            break;
        case -4:
            printf("El fichero no existe.\n");
            signalSemaforo();
            return -1;
            break;
        default:
            inodo_padre = leer_inodo(p_inodo_dir);
            int numentradas = inodo_padre.tamEnBytesLog/sizeof(struct entrada);
            if (p_entrada == numentradas - 1) {
                mi_truncar_f(p_inodo_dir, inodo_padre.tamEnBytesLog - sizeof(struct entrada));
            } else {
                struct entrada entrada;
                mi_read_f(p_inodo_dir,&entrada,p_entrada*sizeof(struct entrada),sizeof(struct entrada));
                mi_read_f(p_inodo_dir,&entrada,inodo_padre.tamEnBytesLog - sizeof(struct entrada),sizeof(struct entrada));
                mi_write_f(p_inodo_dir,&entrada,p_entrada*sizeof(struct entrada),sizeof(struct entrada));
                mi_truncar_f(p_inodo_dir, inodo_padre.tamEnBytesLog - sizeof(struct entrada));
                struct inodo inodo_hijo = leer_inodo(p_inodo);
                inodo_hijo.nlinks--;
                if (inodo_hijo.nlinks == 0) {
                    liberar_inodo(p_inodo);
                } else {
                    inodo_hijo.ctime = time(NULL);
                    escribir_inodo(inodo_hijo, p_inodo);
                }
            }
            signalSemaforo();
            return 0;
            break;
    }
}

/*
* Pone el contenido del directorio en un buffer de memoria (el nombre de cada entrada puede venir separado por '|') y devuelve el número de entradas.
*/
int mi_dir(const char *camino, char *buffer) {
	unsigned int p_inodo_dir = 0, p_inodo, p_entrada;
	struct inodo inodo;    
	if (camino[strlen(camino)-1] != '/') {
		printf("El camino no se corresponde con un directorio.\n");
		return -1;
	}    
	switch (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada,0, 7)) {
		case -1:
			printf("El camino no es válido.\n");
			return -1;
			break;
		case -2:
			printf("Algun elemento del path no tiene permisos de lectura.\n");
			return -1;
			break;
		case -4:
			printf("Directorio inexistente.\n");
			return -1;
			break;
		default:
			inodo = leer_inodo(p_inodo);
			if (inodo.tipo == 'd') {
				if ((inodo.permisos & 4) == 4) {
					int numentradas = inodo.tamEnBytesLog/sizeof(struct entrada);
					int i;
					struct entrada entrada;
					buffer[0] = '\0';
					for (i = 0; i < numentradas; i++) {
						mi_read_f(p_inodo,&entrada,i*sizeof(struct entrada),sizeof(struct entrada));
						strcat(buffer,entrada.nombre);
						strcat(buffer,"|");
					}
					return numentradas;
				} else {
					printf("El directorio no tiene permisos de lectura.\n");
					return -1;
				}
			} else {
				printf("El path no hace es ningun directorio.\n");
				return -1;
			}
			break;
	}
}

/*
* Buscar la entrada camino para obtener el p_inodo. Si la entrada existe llamamos a la función 
* correspodiente de ficheros.c pasándole el p_inodo: mi_chmod_f(p_inodo, modo)
*/
int mi_chmod(const char *camino, unsigned char modo) {
	unsigned int p_inodo_dir = 0, p_inodo, p_entrada;    
	switch (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, modo)) {
		case -1:
			printf("El camino introducido no es válido.\n");
			return -1;
			break;
		case -2:
			printf("Existe un elemento del camino que no tiene permisos de lectura\n");
			return -1;
			break;
		case -4:
			printf("Fichero inexistente\n");
			return -1;
			break;
		default:
			mi_chmod_f(p_inodo, modo);
			return 0;
			break;
	}       
}

/*
* Buscar la entrada camino para obtener el p_inodo. Si la entrada existe llamamos a la función
* correspodiente de ficheros.c pasándole el p_inodo: mi_stat_f(p_inodo, pstat)
*/
int mi_stat(const char *camino, struct STAT *p_stat) {
	unsigned int p_inodo_dir = 0, p_inodo, p_entrada;
	switch (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7)) {
		case -1:
			printf("Path no válido.\n");
			return -1;
			break;
		case -2:
			printf("Un elemento del camino no tiene permisos de lectura.\n");
			return -1;
			break;
		case -4:
			printf("Fichero inexistente.\n");
			return -1;
			break;
		default:
			mi_stat_f(p_inodo, p_stat);
			return 0;
			break;
	}
}

/*
* Buscar la entrada camino para obtener el p_inodo. Si la entrada existe llamamos a la función
* correspodiente de ficheros.c pasándole el p_inodo: mi_read_f(p_inodo, buf, offset, nbytes)
*/
int mi_read(const char *camino, void *buf, unsigned int offset, unsigned int nbytes) {
	unsigned int p_inodo_dir = 0, p_inodo, p_entrada;
	switch (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7)) {
		case -1:
			printf("El camino es invalido.\n");
			return -1;
			break;
		case -2:
			printf("Hay directorios que no tienen permisos de lectura.\n");
			return -1;
			break;
		case -4:
			printf("El fichero no existe.\n");
			return -1;
			break;
		default:
			return mi_read_f(p_inodo, buf, offset, nbytes);
			break;
	}
}

/*
* Buscar la entrada camino para obtener el p_inodo. Si la entrada existe llamamos a la función
* correspodiente de ficheros.c pasándole el p_inodo: mi_write_f(p_inodo, buf, offset, nbytes)
*/
int mi_write(const char *camino, const void *buf, unsigned int offset, unsigned int nbytes) {
	unsigned int p_inodo_dir = 0, p_inodo, p_entrada;
	struct inodo inodo;
	waitSemaforo();
	switch (buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 7)) {
		case -1:
			printf("El camino introducido no es valido.\n");
			signalSemaforo();    
			return -1;
			break;
		case -2:
			printf("Un directorio del path no tiene permisos de lectura.\n");
			signalSemaforo();
			return -1;
			break;
		case -4:
			printf("El fichero no existe.\n");
			signalSemaforo();
			return -1;
			break;
		default:
			inodo = leer_inodo(p_inodo);
			if (inodo.tipo=='d') {
				printf("El camino no es ningun fichero.\n");
				signalSemaforo();
				return -1;
			}
			mi_write_f(p_inodo, buf, offset, nbytes);
			signalSemaforo();
			return 0;
			break;
	}
}
