all: mi_mkfs mi_chmod mi_ln mi_ls mi_ln mi_stat mi_cat mi_rm mi_mkdir simulacion 

mi_mkfs: mi_mkfs.o bloques.o ficheros_basico.o semaforo.o
	gcc -Wall -g mi_mkfs.o bloques.o ficheros_basico.o semaforo.o -o mi_mkfs

mi_cat: mi_cat.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o
	gcc -Wall -g mi_cat.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o -o mi_cat

mi_cat2: mi_cat2.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o
	gcc -Wall -g mi_cat2.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o -o mi_cat2

mi_chmod: mi_chmod.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o
	gcc -Wall -g mi_chmod.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o -o mi_chmod

mi_ln: mi_ln.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o
	gcc -Wall -g mi_ln.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o -o mi_ln

mi_ls: mi_ls.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o
	gcc -Wall -g mi_ls.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o -o mi_ls

mi_rm: mi_rm.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o
	gcc -Wall -g mi_rm.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o -o mi_rm

mi_stat: mi_stat.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o
	gcc -Wall -g mi_stat.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o -o mi_stat

mi_mkdir: mi_mkdir.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o
	gcc -Wall -g mi_mkdir.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o -o mi_mkdir

simulacion: simulacion.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o
	gcc -Wall -g simulacion.o bloques.o ficheros_basico.o ficheros.o directorios.o semaforo.o -o simulacion

clean:
	rm -f *.o mi_mkfs mi_chmod mi_ln mi_ls mi_ln mi_stat mi_cat mi_rm mi_mkdir simulacion 

zip:
	zip -r PracticaASO *.c *.h Makefile script.sh
