#include "ficheros_basico.h"

/*
* Calcula el tamaño en bloques del mapa de bits
*/
int tamMB (unsigned int nbloques){
	int tamMapaBits = nbloques/(TAM_BLOQUE*8);
	if(nbloques%(TAM_BLOQUE*8) != 0){
		tamMapaBits++;
	}
	return tamMapaBits;	
}

/*
* Calcula el tamaño en bloques del array de inodos
*/
int tamAI (unsigned int ninodos){
	int tamArrayInodos = ninodos / (TAM_BLOQUE / TAM_INODO);
	if(ninodos % (TAM_BLOQUE / TAM_INODO) != 0){
		tamArrayInodos++;
	}
	return tamArrayInodos;
}


/*
* Inicializa la estructura del superbloque
*/
int initSB(unsigned int nbloques, unsigned int ninodos) {
	struct superbloque SB;
	SB.posPrimerBloqueMB = POS_SB + 1;
	SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
	SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
	SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
	SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
	SB.posUltimoBloqueDatos = nbloques - 1;
	SB.posInodoRaiz = 0; //En el programa mi_mkfs se reserva el inodo para el root
	SB.posPrimerInodoLibre = 0;
	SB.cantBloquesLibres = SB.posUltimoBloqueDatos - SB.posPrimerBloqueDatos + 1; 
	SB.cantInodosLibres = ninodos;
	SB.totBloques = nbloques;
	SB.totInodos = ninodos;
	return bwrite(POS_SB, &SB);
}

/*
* Inicializa el mapa de bits
*/
int initMB(unsigned int nbloques) {
	struct superbloque SB;
	bread(POS_SB, &SB);
	char buffer[TAM_BLOQUE];
	memset(buffer, 0, TAM_BLOQUE);
	int i;
	for (i = SB.posPrimerBloqueMB; i <= SB.posUltimoBloqueMB; i++) {
		bwrite(i,buffer);
	}
	return 0;
}

/*
* Inicializa el array de inodos
*/
int initAI(unsigned int ninodos) {
	struct superbloque SB;
	bread(POS_SB, &SB);
	int inodosPorBloque = TAM_BLOQUE/TAM_INODO;
	struct inodo libres[inodosPorBloque]; //Array de 1024/128 = 8 inodos
	int i,j;
	int sigLibre = SB.posPrimerInodoLibre;
	for (i = SB.posPrimerBloqueAI; i < SB.posUltimoBloqueAI; i++) {//Para todo bloque
		for (j = 0; j < inodosPorBloque; j++) {//Para todo inodo
			sigLibre++;
			libres[j].punterosDirectos[0] = sigLibre;//Ligar inodo
        	}
		bwrite(i, libres);
		
    	}
	int otrosLibres = (ninodos - 1) % inodosPorBloque;
	for (i = 0; i < otrosLibres; i++) { 
		sigLibre++;
		libres[i].punterosDirectos[0] = sigLibre;
	}
	libres[otrosLibres].punterosDirectos[0] = NIL; // Fin de la lista de inodos libres
	return bwrite(SB.posUltimoBloqueAI, libres);
}

/*
* Escribe el valor 0 o 1 en un determinado bit del mapa de bits
*/
int escribir_bit(unsigned int nbloque, unsigned int bit) {
	int pos_byte = nbloque / 8;
	int pos_bit = nbloque % 8;
	char bufBloqueMB [TAM_BLOQUE];
	struct superbloque SB;
	bread(POS_SB, &SB);    
	int sigBloque = SB.posPrimerBloqueMB + pos_byte/TAM_BLOQUE;    
	bread(sigBloque, bufBloqueMB);
	unsigned char byte;
	if (bit == 1) {
		byte = 128;
		byte >>= pos_bit;//desplaçament
		bufBloqueMB[pos_byte % TAM_BLOQUE] |= byte;//OR lògica
    	} else {
		byte = 128;
		byte >>= pos_bit;//desplaçament
		bufBloqueMB[pos_byte % TAM_BLOQUE] &= ~byte;//AND lògica + byte negat
   	}
	bwrite(sigBloque, bufBloqueMB);
}

/*
* Lee el valor de un determinado bit del mapa de bits
*/
unsigned char leer_bit(unsigned int nbloque) {
	int pos_byte = nbloque / 8;
	int pos_bit = nbloque % 8;
	char bufBloqueMB[TAM_BLOQUE];    
	struct superbloque SB;
	bread(POS_SB, &SB);     
	int sigBloque = SB.posPrimerBloqueMB + pos_byte/TAM_BLOQUE;    
	bread(sigBloque, bufBloqueMB);
	unsigned char mascara = 128;
	mascara >>= pos_bit; 
	mascara &= bufBloqueMB[pos_byte];//AND lògica
	mascara >>= (7-pos_bit);
	return mascara;
}

/*
* Encuentra el primer bloque libre (ver cálculos para el mapa de bits), lo ocupa (con la ayuda de la función escribir_bit) y devuelve su posición
*/
int reservar_bloque() {
	struct superbloque SB;
	bread(POS_SB, &SB);
	if (SB.cantBloquesLibres==0) {
		printf("Actualmente no quedan bloques libres.\n");
		return -1;
    	} else {
		unsigned char bloqueMB[TAM_BLOQUE]; 
		unsigned char bufferAux[TAM_BLOQUE];
		int bloqueAux;
		for (bloqueAux = SB.posPrimerBloqueMB; bloqueAux <= SB.posUltimoBloqueMB; bloqueAux++) {//Para todo bloque del MB
            		memset(bufferAux, 255, TAM_BLOQUE);
            		bread(bloqueAux, bloqueMB);
            		if (memcmp(bloqueMB, bufferAux, TAM_BLOQUE)!=0) {//Ver si hay algun 0 (habra bloque libre)
                		char encontrado = 0;
                		int posByteActual;
                		for (posByteActual = 0; posByteActual < TAM_BLOQUE; posByteActual++) {//Para todo byte del bloque
                    			unsigned char mascara = 128;
                    			int i = 0;
                    			if (bloqueMB[posByteActual] < 255) {//Si hay algun 0 (habra algun bit libre)
                        			encontrado = 1;
                        			while (bloqueMB[posByteActual] & mascara) {//Desplazamos la mascara hasta conseguir el bit
                            				bloqueMB[posByteActual] <<= 1;
                            				i++;
                        			}
                        			int numBloque = ((bloqueAux - SB.posPrimerBloqueMB) * TAM_BLOQUE + posByteActual)*8 + i;
                        			escribir_bit(numBloque, 1);
                        			SB.cantBloquesLibres--;
                       	 			if (bwrite(POS_SB, &SB)==-1) {
                            				return -1;
                        			}
                        			return numBloque+SB.posPrimerBloqueDatos;//Devolvemos el bloque reservado con el offset de la metainformacion
                    			}
                		}
            		}
        	}
	}
}

/*
* Libera un bloque determinado (con la ayuda de la función escribir_bit)
*/
int liberar_bloque(unsigned int nbloque) {//Premisa: ésta funcion se llama solo si el bloque esta ocupado
	struct superbloque SB;
	bread(POS_SB, &SB);
	escribir_bit(nbloque - SB.posPrimerBloqueDatos, 0);
	SB.cantBloquesLibres++;
	bwrite(POS_SB, &SB);
	return nbloque;
}

/*
* Escribe el contenido de una variable de tipo INODO en un determinado inodo del array de inodos
*/
int escribir_inodo(struct inodo inodo, unsigned int ninodo) {
	struct superbloque SB;
	bread(POS_SB, &SB);
	int inodosPorBloque = TAM_BLOQUE/TAM_INODO; //1024 bytes / 128 bytes = 8 inodos por bloque
	int numBloqueInodo = SB.posPrimerBloqueAI + ninodo/inodosPorBloque;
	struct inodo inodosBloque[inodosPorBloque];
	bread(numBloqueInodo, inodosBloque);
	inodosBloque[ninodo%inodosPorBloque] = inodo;//Colocamos en la posicion dentro del bloque, el inodo en cuestion y reescribimos el bloque 
	bwrite(numBloqueInodo, inodosBloque);
	return 0;
}

/*Lee un determinado inodo del array de inodos para volcarlo en una variable de tipo INODO*/
struct inodo leer_inodo(unsigned int ninodo) {
	struct superbloque SB;
	bread(POS_SB, &SB);
	int inodosPorBloque = TAM_BLOQUE/TAM_INODO;
	int numBloqueInodo = SB.posPrimerBloqueAI + ninodo/inodosPorBloque;
	struct inodo inodosBloque[inodosPorBloque];
	bread(numBloqueInodo, inodosBloque);
	return inodosBloque[ninodo%inodosPorBloque];//Devolvemos la posicion dentro del bloque, es decir, el inodo
}

/*
* Encuentra el primer inodo libre (información almacenada en el superbloque), lo reserva (con la ayuda de la función escribir_inodo) y devuelve su número
*/
int reservar_inodo(unsigned char tipo, unsigned char permisos) {
	struct superbloque SB;
	bread(POS_SB, &SB);
	if (SB.cantInodosLibres==0) {
		printf("Actualmente no quedan inodos libres.\n");
		return -1;
    	} else {
		int posAux = SB.posPrimerInodoLibre;
        	struct inodo inodoAux = leer_inodo (posAux);
        	SB.posPrimerInodoLibre = inodoAux.punterosDirectos[0];//Swap con el primero de la lista de libres
        	SB.cantInodosLibres--;//Decrementamos el reservado
        	bwrite(POS_SB, &SB);
		//Inicializacion del inodo
		inodoAux.tipo = tipo;
		inodoAux.permisos = permisos;
		inodoAux.nlinks = 1; 
		inodoAux.tamEnBytesLog = 0;
		time(&(inodoAux.atime));
		time(&(inodoAux.mtime));
		time(&(inodoAux.ctime));
		inodoAux.numBloquesOcupados = 0;
        	int i;
		for (i = 0; i < 12; i++) {
            		inodoAux.punterosDirectos[i] = 0;
        	}
        	for (i = 0; i < 3; i++) {
            		inodoAux.punterosIndirectos[i] = 0;
        	}
        	escribir_inodo(inodoAux, posAux);//Escritura del inodo
        	return posAux;
	} 
}

/*
* Libera todos los bloques de un inodo determinado, lo marca como libre y lo incluye en la lista enlazada de inodos libres
*/
int liberar_inodo(unsigned int ninodo) {
	struct superbloque SB;
	bread(POS_SB, &SB);
	struct inodo inodoAux;
	inodoAux = leer_inodo(ninodo);
	liberar_bloques_inodo(ninodo,0);//Liberamos TODOS=>blogico = 0 los bloques
	inodoAux.punterosDirectos[0] = SB.posPrimerInodoLibre;
	SB.posPrimerInodoLibre = ninodo; 
	SB.cantInodosLibres++;
	escribir_inodo(inodoAux,ninodo);
	bwrite (POS_SB, &SB);
    	return ninodo;
}



/*
* Calcula el número de bloque físico ('bfisico') al que hace referencia el número de bloque lógico ('blogico') de un determinado
* inodo ('ninodo') A su vez, 'reservar' nos indica si queremos consultar o bien consultar y reservar un bloque libre si ningún bloque
* físico es apuntado por el número de bloque lógico
*/
int traducir_bloque_inodo(unsigned int inodo, unsigned int blogico, unsigned int *bfisico, char reservar) {
	    const int cantPuntDirectos = 12;
	    const int cantPuntIndirectos = 3;
	    int pos = blogico-cantPuntDirectos;
	    int numPunteros = TAM_BLOQUE/sizeof(int);//256 por defecto
	    int numPunteros2 = numPunteros*numPunteros;
	    int numPunteros3 = numPunteros2*numPunteros;
	    struct inodo inodoLeido = leer_inodo(inodo);//leemos inodo

	    //Si punteros directos...
	    if (blogico<cantPuntDirectos) {
		switch (reservar) {
		    case 0:
		        if (inodoLeido.punterosDirectos[blogico] == 0) {
			    //hemos intentado traducir un bloque inexistente
		            return -1;
		        } else {
			    //devolvemos bloque físico
		            *bfisico = inodoLeido.punterosDirectos[blogico];
		        }
		        break;
		    case 1:
		        if (inodoLeido.punterosDirectos[blogico] == 0) {
			    //hemos intentado traducir un bloque inexistente, así que lo reservamos
		            inodoLeido.punterosDirectos[blogico] = reservar_bloque();
		            inodoLeido.numBloquesOcupados++;
		            *bfisico = inodoLeido.punterosDirectos[blogico];
		            escribir_inodo(inodoLeido, inodo);
		        } else {
			    //devolvemos bloque físico
		            *bfisico = inodoLeido.punterosDirectos[blogico];
		        }
		        break;
		}
	    //Si punteros indirectos simples... (nivel 1)
	    } else if (blogico < cantPuntDirectos+numPunteros) {
		switch (reservar) {
		    case 0: 
		        if (inodoLeido.punterosIndirectos[0] == 0) {
			    //hemos intentado traducir un bloque inexistente
		            return -1;
		        } else {
		            int buffer[numPunteros];
		            bread(inodoLeido.punterosIndirectos[0], buffer);
		            if (buffer[pos] == 0) {
			        //hemos intentado traducir un bloque inexistente
		                return -1;
		            } else {
			        //devolvemos bloque físico
		                *bfisico = buffer[pos];
		            }
		        }
		        break;
		    case 1:
		        if (inodoLeido.punterosIndirectos[0] == 0) {
			    //hemos intentado traducir un bloque inexistente, así que lo reservamos
		            inodoLeido.punterosIndirectos[0] = reservar_bloque();
		            int buffer[numPunteros];
		            memset(buffer, 0, TAM_BLOQUE);
		            buffer[pos] = reservar_bloque();
		            inodoLeido.numBloquesOcupados++;
		            bwrite(inodoLeido.punterosIndirectos[0], buffer);
		            *bfisico = buffer[pos];
		        } else {
		            int buffer[numPunteros];
		            bread(inodoLeido.punterosIndirectos[0], buffer);
		            if (buffer[pos] == 0) {
			        //hemos intentado traducir un bloque inexistente, así que lo reservamos
		                buffer[pos] = reservar_bloque();
		                inodoLeido.numBloquesOcupados++;
		                bwrite(inodoLeido.punterosIndirectos[0], buffer);
		                *bfisico = buffer[pos];
		            } else {
			        //devolvemos bloque físico
		                *bfisico = buffer[pos];
		            }
		        }
		        escribir_inodo(inodoLeido, inodo);
		        break;
		}
	    //Si punteros indirectos dobles... (nivel 2)
	    } else if (blogico < cantPuntDirectos+numPunteros+numPunteros2) {
		int punt0,punt1;
		punt0 = (blogico-(cantPuntDirectos+numPunteros))%numPunteros;
		punt1 = (blogico-(cantPuntDirectos+numPunteros))/numPunteros;
		switch (reservar) {
		    case 0:
		        if (inodoLeido.punterosIndirectos[1] == 0) {
			    //hemos intentado traducir un bloque inexistente
		            return -1;
		        } else {
		            int bufferNivel1[numPunteros];
		            bread(inodoLeido.punterosIndirectos[1], bufferNivel1);
		            if (bufferNivel1[punt1]==0) {
			        //hemos intentado traducir un bloque inexistente
		                return -1;
		            } else {
		                int bufferNivel0[numPunteros];
		                bread(bufferNivel1[punt1], bufferNivel0);
		                if (bufferNivel0[punt0] == 0) {
			    	    //hemos intentado traducir un bloque inexistente
		                    return -1;
		                } else {
				    //devolvemos bloque físico
		                    *bfisico = bufferNivel0[punt0];
		                }
		            } 
		        }
		        break;
		    case 1: 
		        if (inodoLeido.punterosIndirectos[1] == 0) {
			    //hemos intentado traducir un bloque inexistente, así que lo reservamos
		            inodoLeido.punterosIndirectos[1] = reservar_bloque();
		            int bufferNivel1[numPunteros];
		            memset(bufferNivel1, 0, TAM_BLOQUE);
		            bufferNivel1[punt1] = reservar_bloque();
		            int bufferNivel0[numPunteros];
		            memset(bufferNivel0, 0, TAM_BLOQUE);
		            bufferNivel0[punt0] = reservar_bloque();
		            inodoLeido.numBloquesOcupados++;
		            bwrite(inodoLeido.punterosIndirectos[1],bufferNivel1);
		            bwrite(bufferNivel1[punt1],bufferNivel0);
		            *bfisico = bufferNivel0[punt0];
		        } else {
		            int bufferNivel1[numPunteros]; 
		            bread(inodoLeido.punterosIndirectos[1], bufferNivel1);
		            if (bufferNivel1[punt1] == 0) {
			        //hemos intentado traducir un bloque inexistente, así que lo reservamos
		                bufferNivel1[punt1] = reservar_bloque();
		                int bufferNivel0[numPunteros];
		                memset(bufferNivel0, 0, TAM_BLOQUE);
		                bufferNivel0[punt0] = reservar_bloque();
		                inodoLeido.numBloquesOcupados++;
		                bwrite(inodoLeido.punterosIndirectos[1],bufferNivel1);
		                bwrite(bufferNivel1[punt1],bufferNivel0);
		                *bfisico = bufferNivel0[punt0];
		            } else {
		                int bufferNivel0[numPunteros]; 
		                bread(bufferNivel1[punt1], bufferNivel0);
		                if (bufferNivel0[punt0] == 0) {
			    	    //hemos intentado traducir un bloque inexistente, así que lo reservamos
		                    bufferNivel0[punt0] = reservar_bloque();
		                    inodoLeido.numBloquesOcupados++;
		                    bwrite(bufferNivel1[punt1],bufferNivel0);
		                    *bfisico = bufferNivel0[punt0];
		                } else {
				    //devolvemos bloque físico
		                    *bfisico = bufferNivel0[punt0];
		                }
		            }
		        }
		        escribir_inodo(inodoLeido, inodo);
		        break;
		}
	    //Si punteros indirectos triples... (nivel 3)
	    } else if (blogico < cantPuntDirectos + numPunteros + numPunteros2 + numPunteros3) {
		int punt0,punt1,punt2;
		punt2 = (blogico - (cantPuntDirectos + numPunteros + numPunteros2))/numPunteros2;
		punt1 = ((blogico - (cantPuntDirectos + numPunteros + numPunteros2))%numPunteros2)/numPunteros;
		punt0 = ((blogico - (cantPuntDirectos + numPunteros + numPunteros2))%numPunteros2)%numPunteros;
		switch (reservar) {
		    case 0:
		        if (inodoLeido.punterosIndirectos[2] == 0) {
			    //hemos intentado traducir un bloque inexistente
		            return -1;
		        } else {
		            int bufferNivel2[numPunteros];
		            bread(inodoLeido.punterosIndirectos[2], bufferNivel2);
		            if (bufferNivel2[punt2] == 0) {
			        //hemos intentado traducir un bloque inexistente
		                return -1;
		            } else {
		                int bufferNivel1[numPunteros];
		                bread(bufferNivel2[punt2], bufferNivel1);
		                if (bufferNivel1[punt1] == 0) {
			   	    //hemos intentado traducir un bloque inexistente
		                    return -1;
		                } else {
		                    int bufferNivel0[numPunteros];
		                    bread(bufferNivel1[punt1], bufferNivel0);
		                    if (bufferNivel0[punt0] == 0) {
			    		//hemos intentado traducir un bloque inexistente
		                        return -1;
		                    } else {
				        //devolvemos bloque físico
		                        *bfisico = bufferNivel0[punt0];
		                    }
		                }
		            }
		        }
		        break;
		    case 1:
		        if (inodoLeido.punterosIndirectos[2] == 0) {
			    //hemos intentado traducir un bloque inexistente, así que lo reservamos
		            inodoLeido.punterosIndirectos[2] = reservar_bloque();
		            int bufferNivel2[numPunteros];
		            memset(bufferNivel2, 0, TAM_BLOQUE);
		            bufferNivel2[punt2] = reservar_bloque();
		            int bufferNivel1[numPunteros];
		            memset(bufferNivel1, 0, TAM_BLOQUE);
		            bufferNivel1[punt1] = reservar_bloque();
		            int bufferNivel0[numPunteros];
		            memset(bufferNivel0, 0, TAM_BLOQUE);
		            bufferNivel0[punt0] = reservar_bloque();
		            inodoLeido.numBloquesOcupados++;
		            bwrite(inodoLeido.punterosIndirectos[2],bufferNivel2);
		            bwrite(bufferNivel2[punt2],bufferNivel1);
		            bwrite(bufferNivel1[punt1],bufferNivel0);
		            *bfisico = bufferNivel0[punt0];
		        } else {
		            int bufferNivel2[numPunteros];
		            bread(inodoLeido.punterosIndirectos[2], bufferNivel2);
		            if (bufferNivel2[punt2] == 0) {
			    	//hemos intentado traducir un bloque inexistente, así que lo reservamos
		                bufferNivel2[punt2] = reservar_bloque();
		                int bufferNivel1[numPunteros];
		                memset(bufferNivel1, 0, TAM_BLOQUE);
		                bufferNivel1[punt1] = reservar_bloque();
		                int bufferNivel0[numPunteros];
		                memset(bufferNivel0, 0, TAM_BLOQUE);
		                bufferNivel0[punt0] = reservar_bloque();
		                inodoLeido.numBloquesOcupados++;
		                bwrite(inodoLeido.punterosIndirectos[2],bufferNivel2);
		                bwrite(bufferNivel2[punt2],bufferNivel1);
		                bwrite(bufferNivel1[punt1],bufferNivel0);
		                *bfisico = bufferNivel0[punt0];
		            } else {
		                int bufferNivel1[numPunteros];
		                bread(bufferNivel2[punt2], bufferNivel1);
		                if (bufferNivel1[punt1] == 0) {
			   	    //hemos intentado traducir un bloque inexistente, así que lo reservamos
		                    bufferNivel1[punt1] = reservar_bloque();
		                    int bufferNivel0[numPunteros];
		                    memset(bufferNivel0, 0, TAM_BLOQUE);
		                    bufferNivel0[punt0] = reservar_bloque();
		                    inodoLeido.numBloquesOcupados++;
		                    bwrite(bufferNivel2[punt2],bufferNivel1);
		                    bwrite(bufferNivel1[punt1],bufferNivel0);
		                    *bfisico = bufferNivel0[punt0];
		                } else {
		                    int bufferNivel0[numPunteros];
		                    bread(bufferNivel1[punt1], bufferNivel0);
		                    if (bufferNivel0[punt0] == 0) {
			    		//hemos intentado traducir un bloque inexistente, así que lo reservamos
		                        bufferNivel0[punt0] = reservar_bloque();
		                        inodoLeido.numBloquesOcupados++;
		                        bwrite(bufferNivel1[punt1],bufferNivel0);
		                        *bfisico = bufferNivel0[punt0];
		                    } else {
				        //devolvemos bloque físico
		                        *bfisico = bufferNivel0[punt0];
		                    }
		                }
		            }
		        }
			//escribimos el inodo con los datos actualizados
		        escribir_inodo(inodoLeido, inodo);
		        break;
		}
	    }
	    return 0;
}

/*
* Libera todos los bloques (con la ayuda de la función liberar_bloque) a partir del bloque lógico indicado por el argumento blogico (inclusive)
*/
int liberar_bloques_inodo(unsigned int ninodo, unsigned int blogico) {

	unsigned int numPunterosDirectos = 12;
	unsigned int numPunteros1 = TAM_BLOQUE/sizeof(unsigned int);//256 por defecto
	unsigned int numPunteros2 = numPunteros1*numPunteros1;
	unsigned int numPunteros3 = numPunteros2*numPunteros1;

	//leemos el inodo:
	struct inodo Inodo = leer_inodo(ninodo);

	//Inicializar a 0 un buffer auxiliar de punteros:
	unsigned int bufferAux[TAM_BLOQUE/sizeof(unsigned int)];//Máscara
	memset(bufferAux, 0, TAM_BLOQUE);

	//Encontramos el último bloque con contenido:
	unsigned int ultimoBloque = Inodo.tamEnBytesLog/TAM_BLOQUE;

	unsigned int blog;
	for (blog=blogico; blog<=ultimoBloque; blog++) {
		if (blog < numPunterosDirectos) {//blogico se encuentra entre 0-11
			if (Inodo.punterosDirectos[blog] > 0) {//Existe el bloque físico
				//Liberar el bloque:
				liberar_bloque(Inodo.punterosDirectos[blog]);
				//Poner a 0 el puntero del inodo a ese bloque:
				Inodo.punterosDirectos[blog] = 0;
				//Decrementar el número de bloques ocupados:
				Inodo.numBloquesOcupados--;
			}
		} else if (blog < numPunterosDirectos + numPunteros1) {//blogico se encuentra entre 12-267
			if (Inodo.punterosIndirectos[0] > 0) {//Existe el bloque de punteros simples
				//Leer el bloque de punteros simples:
				unsigned int bufferNivel0[TAM_BLOQUE/sizeof(unsigned int)];
				bread(Inodo.punterosIndirectos[0], bufferNivel0);
				if (bufferNivel0[blog-numPunterosDirectos] > 0) {//Existe el bloque físico
					//Liberar el bloque:
					liberar_bloque(bufferNivel0[blog-numPunterosDirectos]);
					//Poner a 0 el puntero del buffer del bloque:
					bufferNivel0[blog-numPunterosDirectos] = 0;
					if (memcmp(bufferAux, bufferNivel0, TAM_BLOQUE) == 0) {//No quedan punteros ocupados en el bloque de punteros simples
						//Liberar el bloque de punteros simples:
						liberar_bloque(Inodo.punterosIndirectos[0]);
						//Decrementar el número de bloques ocupados:
						Inodo.numBloquesOcupados--;
						//Poner a 0 el puntero del inodo al bloque
						Inodo.punterosIndirectos[0] = 0;
					} else {
						//Escribimos el bloque de punteros simples actualizado:
						bwrite(Inodo.punterosIndirectos[0], bufferNivel0);
					}
				}
			}
		} else if (blog < numPunterosDirectos + numPunteros1 + numPunteros2) {//blogico se encuentra entre 268-65803
			unsigned int punteroSimple = (blog - (numPunterosDirectos+numPunteros1)) % numPunteros1;
			unsigned int punteroDoble =  (blog - (numPunterosDirectos+numPunteros1)) / numPunteros1;
			if (Inodo.punterosIndirectos[1] > 0) {//Existe el bloque de punteros dobles
				//Leer el bloque de punteros dobles:
				unsigned int bufferNivel1[TAM_BLOQUE/sizeof(unsigned int)];
				bread(Inodo.punterosIndirectos[1], bufferNivel1);
				if (bufferNivel1[punteroDoble] > 0) {//Existe el bloque de punteros simples
					//Leer el bloque de punteros simples:
					unsigned int bufferNivel0[TAM_BLOQUE/sizeof(unsigned int)];
					bread(bufferNivel1[punteroDoble], bufferNivel0);
					if (bufferNivel0[punteroSimple] > 0) {//Existe el bloque físico
						//Liberar el bloque:
						liberar_bloque(bufferNivel0[punteroSimple]);
						//Poner a 0 el puntero del buffer del bloque:
						bufferNivel0[punteroSimple] = 0;
						//Decrementar el número de bloques ocupados:
						Inodo.numBloquesOcupados--;
						if (memcmp(bufferAux, bufferNivel0, TAM_BLOQUE) == 0) {//No quedan punteros ocupados en el bloque de punteros simples
							//Liberar el bloque de punteros simples:
							liberar_bloque(bufferNivel1[punteroDoble]);
							//Poner a 0 el puntero del buffer de punteros simples:
							bufferNivel1[punteroDoble] = 0;
							//Decrementar el número de bloques ocupados:
							Inodo.numBloquesOcupados--;
						} else {
							//Escribimos el bloque de punteros simples actualizado:
							bwrite(bufferNivel1[punteroDoble], bufferNivel0);
						}

						if (memcmp(bufferAux, bufferNivel1, TAM_BLOQUE) == 0) {//No quedan punteros ocupados en el bloque de punteros dobles
							//Liberar el bloque de punteros dobles:
							liberar_bloque(Inodo.punterosIndirectos[1]);
							//Poner a 0 el puntero del inodo al bloque:
							Inodo.punterosIndirectos[1] = 0;
							//Decrementar el número de bloques ocupados:
							Inodo.numBloquesOcupados--;
						} else {
							//Escribimos el bloque de punteros dobles actualizado:
							bwrite(Inodo.punterosIndirectos[1], bufferNivel1);
						}
					}
				}
			}
		} else if (blog < numPunterosDirectos + numPunteros1 + numPunteros2 + numPunteros3) {//blogico se encuentra entre 65804-16843019
			unsigned int punteroSimple = ((blog - (numPunterosDirectos+numPunteros1+numPunteros2)) % numPunteros2) % numPunteros1;
			unsigned int punteroDoble =  ((blog - (numPunterosDirectos+numPunteros1+numPunteros2)) % numPunteros2) / numPunteros1;
			unsigned int punteroTriple = (blog  - (numPunterosDirectos+numPunteros1+numPunteros2)) / numPunteros2;
			if (Inodo.punterosIndirectos[2] > 0) {//Existe el bloque de punteros triples
				//Leer el bloque de punteros triples:
				unsigned int bufferNivel2[TAM_BLOQUE/sizeof(unsigned int)];
				bread(Inodo.punterosIndirectos[2], bufferNivel2);
				if (bufferNivel2[punteroTriple] > 0) {//Existe el bloque de punteros dobles
					//Leer el bloque de punteros dobles:
					unsigned int bufferNivel1[TAM_BLOQUE/sizeof(unsigned int)];
					bread(bufferNivel2[punteroTriple], bufferNivel1);
					if (bufferNivel1[punteroDoble] > 0) {//Existe el bloque de punteros simples
						//Leer el bloque de punteros simples:
						unsigned int bufferNivel0[TAM_BLOQUE/sizeof(unsigned int)];
						bread(bufferNivel1[punteroDoble], bufferNivel0);
						if (bufferNivel0[punteroSimple] > 0) {//Existe el bloque físico
							//Liberar el bloque:
							liberar_bloque(bufferNivel0[punteroSimple]);
							//Poner a 0 el puntero del buffer del bloque:
							bufferNivel0[punteroSimple] = 0;
							if (memcmp(bufferAux, bufferNivel0, TAM_BLOQUE) == 0) {//No punteros ocupados en el bloque de punteros simples
								//Liberar el bloque de punteros simples:
								liberar_bloque(bufferNivel1[punteroDoble]);
								//Decrementar el número de bloques ocupados:
								Inodo.numBloquesOcupados--;
								//Poner a 0 el puntero del buffer de punteros simples:
								bufferNivel1[punteroDoble] = 0;
							} else {
								//Escribimos el bloque de punteros simples actualizado:
								bwrite(bufferNivel1[punteroDoble], bufferNivel0);
							}
							if (memcmp(bufferAux, bufferNivel1, TAM_BLOQUE) == 0) {//No punteros ocupados en el bloque de punteros dobles
								//Liberar el bloque de punteros dobles:
								liberar_bloque(bufferNivel2[punteroTriple]);
								//Poner a 0 el puntero del buffer de punteros dobles:
								bufferNivel2[punteroTriple] = 0;
							} else {
								//Escribimos el bloque de punteros dobles actualizado:
								bwrite(bufferNivel2[punteroTriple], bufferNivel1);
							}
							if (memcmp(bufferAux, bufferNivel2, TAM_BLOQUE) == 0) {//No punteros ocupados en el bloque de punteros triples
								//Liberar el bloque de punteros dobles:
								liberar_bloque(Inodo.punterosIndirectos[2]);
								//Poner a 0 el puntero del inodo al bloque:
								Inodo.punterosIndirectos[2] = 0;
							} else {
								//Escribimos el bloque de punteros dobles actualizado:
								bwrite(Inodo.punterosIndirectos[2], bufferNivel2);
							}
						}
					}
				}
			}
		}
	}
	//Escribir el inodo con la información actualizada:
	escribir_inodo(Inodo, ninodo);
}
