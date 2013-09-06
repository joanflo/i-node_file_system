#script
#!/bin/bash
# $1 --> Disco virtual
# $2 --> Directorio de la simulacion (/simul_aaaammddhhmmss/).
proceso="proceso_4189/"
# Creamos un nuevo directorio dentro de la carpeta de simulacion
echo
echo --- MI_LS: LISTAMOS LA RAIZ DEL DISTEMA DE FICHEROS
echo
./mi_ls $1 /
# Creamos un nuevo directorio dentro de la carpeta de simulacion
echo
echo --- MI_MKDIR: NUEVA CARPETA DENTRO DEL DIRECTORIO DE SIMULACION LLAMADA dir_prueba
echo
./mi_mkdir $1 $2dir_prueba/ 7
# Mostramos las carpetas generadas por los procesos
echo
echo --- MI_LS: LISTAMOS DEL DIRECTORIO DE SIMULACION
echo
./mi_ls $1 $2
# Creamos otro directorio dentro del directorio de prueba
echo
echo --- MI_MKDIR: NUEVA CARPETA DENTRO DEL DIRECTORIO DE PRUEBA
echo
./mi_mkdir $1 $2dir_prueba/subdir_prueba/ 7
# Mostramos el directorio de prueba
echo
echo --- MI_LS: MOSTRAMOS EL DIRECTORIO DE PRUEBA
echo
./mi_ls $1 $2dir_prueba/
# Listamos el directorio de prueba
echo
echo --- MI_LN: NUEVO LINK AL DIRECTORIO DE PRUEBA
echo
./mi_ln $1 $2dir_prueba/ $2dir_prueba/link
# Comprobamos que el link enlaza al fichero de prueba
echo
echo --- MI_CAT DEL LINK
echo
./mi_cat $1 $2dir_prueba/link
# Borramos el subdirectorio de prueba
echo
echo --- MI_RM: BORRAMOS EL SUBDIRECTORIO DE PRUEBA
echo
./mi_rm $1 $2dir_prueba/subdir_prueba/
# Mostramos el directorio de prueba
echo
echo --- MI_LS: MOSTRAMOS EL DIRECTORIO DE PRUEBA
echo
./mi_ls $1 $2dir_prueba/
# Mostramos la metainformacion pruebas.dat del proceso declarado arriba
echo
echo --- MI_STAT: MOSTRAMOS EL ESTADO FICHERO PRUEBAS.DAT DEL PROCESO
echo
./mi_stat $1 $2proceso_4189
