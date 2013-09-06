#include <time.h>
#include <string.h>
#include "bloques.h"
#include "semaforo.h"

#define TAM_INODO 128 // Tamaño inodo = 128 bytes
#define POS_SB 0 // Posición del superbloque
#define NIL 999999999 //Posicion que sale del rango de inodos

struct superbloque {
    unsigned int posPrimerBloqueMB; // Posición del primer bloque del mapa de bits
    unsigned int posUltimoBloqueMB; // Posición del último bloque del mapa de bits
    unsigned int posPrimerBloqueAI; // Posición del primer bloque del array de inodos
    unsigned int posUltimoBloqueAI; // Posición del último bloque del array de inodos
    unsigned int posPrimerBloqueDatos; // Posición del primer bloque de datos
    unsigned int posUltimoBloqueDatos; // Posición del último bloque de datos
    unsigned int posInodoRaiz; // Posición del inodo del directorio raíz
    unsigned int posPrimerInodoLibre; // Posición del primer inodo libre
    unsigned int cantBloquesLibres; // Cantidad de bloques libres
    unsigned int cantInodosLibres; // Cantidad de inodos libres
    unsigned int totBloques; // Cantidad total de bloques
    unsigned int totInodos; // Cantidad total de inodos
    char padding[TAM_BLOQUE-12*sizeof(unsigned int)]; // Relleno
};

struct inodo {
    unsigned char tipo; // Tipo (libre, directorio o fichero)
    unsigned char permisos; // Permisos (lectura y/o escritura y/o ejecución)
    unsigned char reservado_alineacion[2]; 
    time_t atime; // Fecha y hora del último acceso a datos: atime
    time_t mtime; // Fecha y hora de la última modificación de datos: mtime
    time_t ctime; // Fecha y hora de la última modificación del inodo: ctime
    unsigned int nlinks; // Cantidad de enlaces de entradas en directorio
    unsigned int tamEnBytesLog; // Tamaño en bytes lógicos
    unsigned int numBloquesOcupados; // Cantidad de bloques ocupados en la zona de datos
    unsigned int punterosDirectos[12]; // 12 punteros a bloques directos
    unsigned int punterosIndirectos[3]; // 3 punteros a bloques indirectos: 1 simple, 1 doble, 1 triple
    char padding[TAM_INODO-(4*sizeof(char))-(18*sizeof(int))-(3*sizeof(time_t))];
};

int tamMB (unsigned int nbloques);
int tamAI (unsigned int ninodos);
int initSB (unsigned int nbloques, unsigned int ninodos);
int initMB (unsigned int nbloques); 
int initAI (unsigned int ninodos); 
int escribir_bit(unsigned int nbloque, unsigned int bit); 
unsigned char leer_bit(unsigned int nbloque); 
int reservar_bloque(); 
int liberar_bloque(unsigned int nbloque); 
int escribir_inodo(struct inodo inodo, unsigned int ninodo); 
struct inodo leer_inodo(unsigned int ninodo); 
int reservar_inodo(unsigned char tipo, unsigned char permisos); 
int liberar_inodo(unsigned int inodo); 
int traducir_bloque_inodo(unsigned int ninodo, unsigned int blogico, unsigned int *bfisico, char reservar); 
int liberar_bloques_inodo (unsigned int inodo, unsigned int blogico); 
