#include "ficheros_basico.h"
#include <time.h>

struct STAT {
	unsigned char tipo;
	unsigned char permisos;
	time_t atime;
	time_t mtime;
	time_t ctime;
	unsigned int nlinks;
	unsigned int tamEnBytesLog;
	unsigned int numBloquesOcupados;
};

int mi_write_f(unsigned int inodo, const void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_read_f(unsigned int inodo, void *buf_original, unsigned int offset, unsigned int nbytes);
int mi_chmod_f(unsigned int inodo, unsigned char modo);
int mi_truncar_f(unsigned int inodo, unsigned int nbytes);
int mi_stat_f(unsigned int inodo, struct STAT *p_stat);
