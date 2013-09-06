/* Fichero semaforo.c */

#include <sys/sem.h> 
#include "semaforo.h"

void obtenerSem(int *sem){
        int key = ftok("/bin/ls", 1);
        *sem = semget(key, 1, 0600 | IPC_CREAT); 
        //semget crea o toma el control de un semáforo.
        //El 1 indica la cantidad de semáforos
}


void iniciarSem(int *sem){
        obtenerSem(sem); //sem identifica el grupo de semáforos
        semctl(*sem,0,SETVAL,1); 
        //El 0 indica el nº de semáforo 
        //y SETVAL que se modifica el valor del semáforo
}

void eliminarSem(int sem){
        semctl(sem, 0, IPC_RMID, 0);
        //IPC_RMID indica al Kernel que debe borrar el conj. de semáforos
        //No tiene efecto mientras exista algún proceso que esté usando los sem.
}

void signalSem(int sem){
        struct sembuf s;

        s.sem_num = 0;
        //Nª de semáforo dentro del conjunto

        s.sem_op = 1;
        //clase de operación: <0 decrementa (Wait), >0 incrementa (Signal)
        // =0 no se altera

        s.sem_flg = 0; // modificadores de operación

        semop(sem, &s, 1); 
        /* semop realiza operaciones de incremento o decremento con bloqueo
        *  &s es un puntero al array de operaciones
        * El 1 indica el nº de elementos que tiene el array de operaciones */
        
}

void waitSem(int sem){
        struct sembuf w;        
        
        w.sem_num = 0;
        w.sem_op = -1;
        w.sem_flg = 0;
        semop(sem, &w, 1);
}