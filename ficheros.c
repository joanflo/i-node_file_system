#include "ficheros.h"

/*
* Escribe el contenido de un buffer de memoria en un fichero/directorio y devuelve la cantidad de bytes escritos
* inodo: fichero/directorio a escribir
* offset: posición de escritura inicial respecto el inodo (byte lógico)
* nbytes: número de bytes a escribir
*/
int mi_write_f(unsigned int inodo, const void *buf_original, unsigned int offset, unsigned int nbytes) {
	struct inodo inodoAux;
	inodoAux = leer_inodo(inodo);
	//Debemos tener permisos de escritura:
	if ((inodoAux.permisos & 2) == 2) {        
		unsigned int primerBloqueLog = offset/TAM_BLOQUE;
		unsigned int ultimoBloqueLog = (offset + nbytes - 1)/TAM_BLOQUE;
        	char bufBloque [TAM_BLOQUE];         	
        	if (primerBloqueLog == ultimoBloqueLog) {//Si es escritura de un solo bloque
            		unsigned int bfisico;
            		traducir_bloque_inodo(inodo, primerBloqueLog, &bfisico, 1);
            		bread(bfisico, bufBloque);
            		unsigned int desplazamientoBytes = offset % TAM_BLOQUE;
            		memcpy (bufBloque + desplazamientoBytes, buf_original, nbytes);
            		bwrite(bfisico, bufBloque);       	 	
		} else {//Si es escritura de multiples bloques
            		//Primero
            		unsigned int bfisico;
            		traducir_bloque_inodo(inodo, primerBloqueLog, &bfisico, 1);
            		bread(bfisico, bufBloque);
            		unsigned int despBytesPrimerBloque = offset % TAM_BLOQUE;
            		memcpy (bufBloque + despBytesPrimerBloque, buf_original, TAM_BLOQUE - despBytesPrimerBloque);
            		bwrite(bfisico, bufBloque);            
            		//Intermedios
            		unsigned int bloqueActual;
            		for (bloqueActual = primerBloqueLog + 1; bloqueActual < ultimoBloqueLog; bloqueActual++) {
                		traducir_bloque_inodo(inodo, bloqueActual, &bfisico, 1);
                		bwrite(bfisico, buf_original + (TAM_BLOQUE - despBytesPrimerBloque) + (bloqueActual - primerBloqueLog - 1) * TAM_BLOQUE);
            		}            
            		//Ultimo
            		traducir_bloque_inodo(inodo, ultimoBloqueLog, &bfisico, 1);
            		bread(bfisico, bufBloque);
            		unsigned int despBytesUltimoBloque = (offset + nbytes - 1) % TAM_BLOQUE;
            		memcpy (bufBloque, /*siguiente linea*/
			buf_original + (TAM_BLOQUE - despBytesPrimerBloque) + (ultimoBloqueLog - primerBloqueLog - 1) * TAM_BLOQUE, despBytesUltimoBloque + 1);
            		bwrite(bfisico, bufBloque);            
        	}        
        	inodoAux = leer_inodo(inodo);
        	inodoAux.mtime = time(NULL);
        	if (offset + nbytes > inodoAux.tamEnBytesLog) {
           		inodoAux.tamEnBytesLog = offset + nbytes;
        	}
        	escribir_inodo(inodoAux, inodo);        
        	return nbytes;
    	} else {
        	printf("El fichero a leer no tiene permisos de escritura.\n");
        	return -1;
    	}
}


/*
* Lee información de un fichero/directorio y la almacena en un buffer de memoria y devuelve la cantidad de bytes escritos
* inodo: fichero/directorio a leer
* offset: posición de lectura inicial respecto el inodo (byte lógico)
* nbytes: número de bytes a leer
*/
int mi_read_f(unsigned int inodo, void *buf_original, unsigned int offset, unsigned int nbytes) {
	struct inodo inodoAux;
	inodoAux = leer_inodo (inodo);
	//Debemos tener permisos de lectura
	if ((inodoAux.permisos & 4) == 4) {        
		unsigned int primerBloqueLog;
		unsigned int ultimoBloqueLog;        
		if (offset > inodoAux.tamEnBytesLog) {
			return 0;
		} else if (offset + nbytes - 1 > inodoAux.tamEnBytesLog) {
			ultimoBloqueLog = (inodoAux.tamEnBytesLog - 1)/TAM_BLOQUE;
			nbytes = inodoAux.tamEnBytesLog - offset;
        	} else {
			ultimoBloqueLog = (offset + nbytes - 1)/TAM_BLOQUE;
        	}        
        	primerBloqueLog = offset/TAM_BLOQUE;
        	char bufBloque [TAM_BLOQUE];
		if (primerBloqueLog == ultimoBloqueLog) {//Si es lectura de un solo bloque
            		unsigned int bloqueLectura;
            		if (traducir_bloque_inodo(inodo, primerBloqueLog, &bloqueLectura, 0)==0) {
                		bread(bloqueLectura, bufBloque);
                		unsigned int offset2 = offset % TAM_BLOQUE;
                		memcpy (buf_original, bufBloque + offset2, nbytes);
            		}
        	} else {//Si es lectura de multiples bloques
			//Primero
			unsigned int bloqueLectura;
			unsigned int despBytesPrimerBloque = offset % TAM_BLOQUE;
			if (traducir_bloque_inodo(inodo, primerBloqueLog, &bloqueLectura, 0) == 0) {
				bread(bloqueLectura, bufBloque);                
				memcpy (buf_original, bufBloque + despBytesPrimerBloque, TAM_BLOQUE - despBytesPrimerBloque);
            		}        
			//Intermedios
			unsigned int bloqueActual;
			for (bloqueActual = primerBloqueLog + 1; bloqueActual < ultimoBloqueLog; bloqueActual++) {
				if (traducir_bloque_inodo(inodo, bloqueActual, &bloqueLectura, 0)==0) {
					bread(bloqueLectura, buf_original + (TAM_BLOQUE - despBytesPrimerBloque) + (bloqueActual - primerBloqueLog - 1) * TAM_BLOQUE);
				}
			}            
			//Ultimo
			if(traducir_bloque_inodo(inodo, ultimoBloqueLog, &bloqueLectura, 0)==0) {
				bread(bloqueLectura, bufBloque);
				unsigned int offset3 = (offset + nbytes - 1) % TAM_BLOQUE;
				memcpy (buf_original + (TAM_BLOQUE - despBytesPrimerBloque) + /*siguiente linea*/
				(ultimoBloqueLog - primerBloqueLog - 1) * TAM_BLOQUE, bufBloque, offset3 + 1);
 			}
		}
		inodoAux.atime = time(NULL);
        	escribir_inodo(inodoAux, inodo);
        	return nbytes;
	} else {
        	printf("No tiene permiso para leer este fichero.\n");
        	return -1;
    	}
}

/*
* Cambia los permisos de un fichero/directorio (que se corresponde con ninodo) según indique el argumento modo
*/
int mi_chmod_f(unsigned int ninodo, unsigned char modo) {
	struct inodo Inodo = leer_inodo(ninodo);
	Inodo.permisos = modo;
	Inodo.ctime = time(NULL);
	return escribir_inodo(Inodo, ninodo);
}


/*
* Trunca un fichero/directorio (que se corresponde con ninodo) a los bytes indicados, liberando los bloques que no hagan falta
* (si se trunca a 0 bytes hay que liberar todos los bloques)
*/
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes) {
	unsigned int blogico = -1;
	//Debemos tener permiso de escritura:
	struct inodo Inodo = leer_inodo(ninodo);
	if ((Inodo.permisos & 2) == 2) {
		blogico = nbytes/TAM_BLOQUE;
		//Si el byte esta a mitad de bloque, hay que preservar el mismo bloque
		if (nbytes%TAM_BLOQUE != 0) {
			blogico++;
		}
		liberar_bloques_inodo(ninodo, blogico);
	}
	unsigned int t = time(NULL);
	Inodo.ctime = t;
	Inodo.mtime = t;
	Inodo.tamEnBytesLog = nbytes;
	escribir_inodo(Inodo,ninodo);
	return blogico;
}


/*
* Devuelve la metainformación de un fichero/directorio (que se corresponde con ninodo)
*/
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat) {
	struct inodo Inodo = leer_inodo(ninodo);
	p_stat->tipo=Inodo.tipo;
	p_stat->permisos=Inodo.permisos;
	p_stat->atime=Inodo.atime;
	p_stat->mtime=Inodo.mtime;
	p_stat->ctime=Inodo.ctime;
	p_stat->nlinks=Inodo.nlinks;
	p_stat->tamEnBytesLog=Inodo.tamEnBytesLog;
	p_stat->numBloquesOcupados=Inodo.numBloquesOcupados;
	return 0;
}
