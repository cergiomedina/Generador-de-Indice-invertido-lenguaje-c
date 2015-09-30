LABORATORIO NUMERO 3 - COMPUTACION PARALELA
SERGIO MEDINA MEDEL - 18.120.798-1

En este laboratorio se implemento la indexacion de documentos con sus respectivas palabras
en el lenguaje de programacion C.
Ademas se utilizó la libreria MPI para trabajar en una determinada cantidad de procesos paralelos
en el sistema. De este modo se logra una mayor rapidez en los calculos y es posible
distribuir el trabajo del software.
Sin embargo algunas cosas no pudieron ser implementadas

Dentro de los aspectos implementados estan:
- Paso de mensajes entre procesos
- Separacion de documentos entre los procesos
- Envio y recepcion de vocabularios locales entre los procesos
- Generacion de Indice de los documentos, con su respectivo titulo
- Generacion de Indice de las palabras que se encuentran en los documentos
- Eliminacion de palabras repetidas
- Determinacion de la cantidad de documentos en los que aparece cada palabra y su correspondiente Indice

Dentro de los aspectos que no pudieron ser implementados estan:
- Eliminacion de palabras no relevantes
- Calculo de frecuencias normalizadas

EJECUCION DEL CODIGO MEDIANTE CONSOLA

Como requisitos se necesita descargar e instalar la libreria MPICH2

Comandos de ejecucion:
COMPILAR: 

mpicc -I /usr/local/mpich2/include/ lab.c -o lab -L /usr/local/mpich2/lib

EJECUTAR:

mpirun -np [CANTIDAD PROCESOS=CANTIDAD DOCUMENTOS] ./lab -f [DOCUMENTO.txt] -s [STOPWORDS.txt]