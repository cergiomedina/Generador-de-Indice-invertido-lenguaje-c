#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <mpi.h>
#include "string.h"
#include <unistd.h>


const char delimitador[]="<end-document>"; // 14 caracteres
const char espacio[] = " ";
const char index_documentos[]="documentos.txt";
const int MAX_LINEA= 200;
const int MAX_PALABRA= 20;
char **vectorStop;
int tamanoVectorStop;
FILE* stopwords;
int repeticion(char *palabra, char ** palabras_documento, int cantidad_palabras);

int main (int argc, char *argv[]){
   int rank, size, get, cantidad_lineas=0,diferencia,cantidad_documentos=0,i,k,j;
   char *linea,**total_lineas,**Documentos,*palabra;
   char *documento_stopwords=(char*)malloc(sizeof(char)*MAX_LINEA);
   FILE *archivo;
   MPI_Init (&argc, &argv);	                /* Inicia MPI */
   MPI_Comm_rank (MPI_COMM_WORLD, &rank);	/* obtiene el id del proceso actual */
   MPI_Comm_size (MPI_COMM_WORLD, &size);	/* obtiene el total de procesos */
// SS1 ------------------------------------------------------------------------------
// Leo todas las lineas del documento y las envio a los procesos equitativamente para que creen el vocabulario
   if(rank==0){
      while((get=getopt(argc,argv,"f:s:"))!=-1)
      switch(get){
         case 'f':
            archivo = fopen(optarg,"r");
            if(archivo==NULL){
               printf("No se puede abrir el fichero %s\n",optarg);
               return 0;         
            }        
            break;
         case 's':
            stopwords = fopen(optarg,"r");
            if(archivo==NULL){
               printf("No se puede abrir el fichero de stopwords %s\n",optarg);
               return 0;         
            }
            strcpy(documento_stopwords,optarg);
            break;
      }
      linea = (char *) malloc(sizeof(char)*MAX_LINEA);
      while(!feof(archivo)){
         if(fgets(linea,MAX_LINEA,archivo)==NULL){
            break;
         }
         cantidad_lineas ++;
         diferencia = strncmp(delimitador,linea,14); // verifico si la linea leida es el final del documento 
         if(diferencia==0){
            cantidad_documentos++;
         }
      }
// Creo el vector de las stopwords
      
      tamanoVectorStop=0;
      char *palabra=(char*)malloc(sizeof(char)*MAX_PALABRA);
      while(!feof(stopwords)){
         if(fgets(palabra,MAX_PALABRA,stopwords)==NULL){
               break;
         }
         palabra[strlen(palabra)-1]='\0';
           tamanoVectorStop ++;
      }
      //printf("TAMANO VECTOR: %i\n",tamanoVectorStop );
      vectorStop=(char**)malloc(sizeof(char*)*tamanoVectorStop);
      for(i=0;i<tamanoVectorStop;i++){
         vectorStop[i]=(char*)malloc(sizeof(char)*MAX_PALABRA);
      }
      rewind(stopwords);
      j=0;
      while(!feof(stopwords)){
         if(fgets(vectorStop[j],MAX_PALABRA,stopwords)==NULL){
               break;
         }
           j++;
      }
      fclose(stopwords);
      
// ---------------------------------------------

      cantidad_lineas -=cantidad_documentos;
      free(linea);
      total_lineas = (char **) malloc(sizeof(char *)*cantidad_lineas);
      for(i=0;i<cantidad_lineas;i++){
         total_lineas[i]= (char *) malloc(sizeof(char)*MAX_LINEA);
      }
      rewind(archivo);
      linea = (char *) malloc(sizeof(char)*MAX_LINEA);
      i = 0;
      while(!feof(archivo)){ // obtengo las lineas del archivo completo y las paso a una matriz de char 
         if(fgets(linea,MAX_LINEA,archivo)==NULL){
            break;
         }
         diferencia = strncmp(delimitador,linea,14); // verifico si la linea leida es el final del documento 
         if(diferencia!=0){
            strcpy(total_lineas[i],linea);
            i++;
         }
      }
      int seguir = 1;
      rewind(archivo);
      int *cantidad_lineas_por_doc = (int *)malloc(sizeof(int)*cantidad_documentos);
      FILE * salida_documentos = fopen(index_documentos,"w");
      int primera_linea = 0;
      for (i = 0; i < cantidad_documentos; ++i){ // obtengo la cantidad de lineas por documento, del archivo. en un documento 

         cantidad_lineas_por_doc[i]=0;
         while(seguir==1){ 
            if(fgets(linea,MAX_LINEA,archivo)==NULL){
               break;
            }
            if(primera_linea==0) fprintf(salida_documentos, "%i, %s",i,linea);
            primera_linea=1;
            diferencia = strncmp(delimitador,linea,14); // verifico si la linea leida es el final del documento 
            if(diferencia!=0){
               cantidad_lineas_por_doc[i]++;
            }else{
               seguir=0;
            }
         }
         seguir=1;
         primera_linea=0;
      }
      rewind(archivo);

      Documentos= (char **)malloc(sizeof(char *)*cantidad_documentos);
      for (i = 0; i < cantidad_documentos; ++i){
         Documentos[i]= (char *)malloc(sizeof(char)*(cantidad_lineas_por_doc[i]*MAX_LINEA));
      }


      /*for (i = 0; i < cantidad_documentos; ++i){
         printf("Documento %i : Lineas -> %i\n",i,cantidad_lineas_por_doc[i] );
      }*/
      int indice=0;
      for (i = 0; i < cantidad_documentos; ++i){ // obtengo las lineas correspondientes a cada documento y las concateno
         strcpy(Documentos[i],total_lineas[indice]);
         indice++;
         for(k=1;k<cantidad_lineas_por_doc[i];k++){
            strcat(Documentos[i],total_lineas[indice]);
            indice++;
         }
      }
      int tamano_envio=0;
      int h;
      // creo el indice de documentos
      
      fclose(salida_documentos);

      // ------------------------------
      for (i = 0; i < cantidad_documentos; ++i){
         tamano_envio = cantidad_lineas_por_doc[i]*MAX_LINEA;
         //MPI_Send(Documentos[i],cantidad_lineas_por_doc[i]*MAX_LINEA,MPI_CHAR,i+1,cantidad_lineas_por_doc[i]*MAX_LINEA,MPI_COMM_WORLD);
         MPI_Send(&tamano_envio,1,MPI_INT,(i+1)%size,0,MPI_COMM_WORLD); // envio tamano del documento
         MPI_Send(Documentos[i],cantidad_lineas_por_doc[i]*MAX_LINEA,MPI_CHAR,(i+1)%size,1,MPI_COMM_WORLD);
         MPI_Send(&tamanoVectorStop,1,MPI_INT,(i+1)%size,2,MPI_COMM_WORLD);

         for(h=0;h<tamanoVectorStop;h++){ // envio las palabras del stopword
            MPI_Send(vectorStop[h],MAX_PALABRA,MPI_CHAR,(i+1)%size,3,MPI_COMM_WORLD);
         }
      }

      free(Documentos);
      free(linea);
      free(total_lineas);

  }
   MPI_Barrier(MPI_COMM_WORLD);
// SS2 -------------------------------------------------------------------------------------------------
// Obtengo las palabras del documento y creo el vocabulario local
   if(rank!=0){ // comienzo a recivir mi documento asignado
      char **vectorStop;
      int tamanoVectorStopword,t;
      int tamano,seguir =1,cantidad_palabras=0,*repeticion_palabras;
      char *documento_local, **palabras_del_documento, *documento_local_temp,**vocabulario;
      char car_inutiles[4]=" \n\t";
      char *palabra;

      // recibo la informacion que me enviaron
      MPI_Status *estado = (MPI_Status*)malloc(sizeof(MPI_Status));
      MPI_Recv(&tamano,1,MPI_INT,0,0,MPI_COMM_WORLD,estado);
      MPI_Recv(&tamanoVectorStopword,1,MPI_INT,0,2,MPI_COMM_WORLD,estado);
      documento_local = (char *)malloc(sizeof(char)*tamano);
      documento_local_temp = (char *)malloc(sizeof(char)*tamano);
      MPI_Recv(documento_local,tamano,MPI_CHAR,0,1,MPI_COMM_WORLD,estado);
      strcpy(documento_local_temp,documento_local);
      // cargo el vector de las stopwords 
      vectorStop= (char **)malloc(sizeof(char*)*tamanoVectorStopword);
      for(t=0;t<tamanoVectorStopword;t++){
         vectorStop[t]=(char*)malloc(sizeof(char)*MAX_PALABRA);
      }
      for(t=0;t<tamanoVectorStopword;t++){ // recibo el vector con las palabras a eliminar del vocabulario
         MPI_Recv(vectorStop[t],MAX_PALABRA,MPI_CHAR,0,3,MPI_COMM_WORLD,estado);
         //printf("VECTOR %i: %s\n",rank,vectorStop[t] );
      }

      // -------------------------------------
      //printf("%i\n",tamanoVectorStopword );

      palabra = strtok( documento_local, car_inutiles );    // Primera llamada => Primer token
      cantidad_palabras++;
      while( (palabra = strtok( NULL, car_inutiles )) != NULL )    // Posteriores llamadas
        {cantidad_palabras++;}
      //printf("Proceso [ %i ] Con Paalabras: %i\n",rank,cantidad_palabras );

      palabras_del_documento = (char **)malloc(sizeof(char *)*cantidad_palabras);
      for (i = 0; i < cantidad_palabras; ++i)
      {
         palabras_del_documento[i]=(char*)malloc(sizeof(char)*MAX_PALABRA);
      }

      int k = 0;
      palabra = strtok( documento_local_temp, car_inutiles );    // Primera llamada => Primer token // quito los espacios y enter
      strcpy(palabras_del_documento[k],palabra);
      palabra=NULL;
      k++;      
      while( (palabra = strtok( NULL, car_inutiles )) != NULL )    // Posteriores llamadas
        {
         strcpy(palabras_del_documento[k],palabra);
         k++;
      }

      repeticion_palabras = (int *)malloc(sizeof(int)*cantidad_palabras);
      int cantidad=0,j;
      for (i = 0; i < cantidad_palabras; ++i)
      {
         cantidad = repeticion(palabras_del_documento[i],palabras_del_documento,cantidad_palabras);
         repeticion_palabras[i]=cantidad;
      }
      // marco las stopwords con un 0 para no contarlas de aqui en adelante
      int palabras_no_stopword=cantidad_palabras;
      /*for(i=0;i<cantidad_palabras;i++){
         if(esStopword(palabras_del_documento[i],stopwords,tamanoVectorStopword)==1){
            repeticion_palabras[i]=0;
            palabras_no_stopword--;
         }
      }*/
      //printf("PALABRAS: %i - NO STOPWORDS: %i\n",cantidad_palabras, palabras_no_stopword );
      // quito las palabras repetidas y las agrego a mi vocabulario local
      int cantidad_palabras_no_repetidas=0;
      for(i=0;i<cantidad_palabras;i++){
         if(repeticion_palabras[i]!=0){
            cantidad_palabras_no_repetidas++;
            repeticion_palabras[i]=0;
            for (j = i; j < cantidad_palabras; j++)
            {
               if(strncmp(palabras_del_documento[i],palabras_del_documento[j],MAX_PALABRA)==0){
                  repeticion_palabras[j]=0;
               }
            }
         } // obtengo la cantidad de palabras UNICAS del documento
      }
      //printf("%i - Cantidad Palabras no repetidas: %i \n",rank,cantidad_palabras_no_repetidas);

      vocabulario = (char **)malloc(sizeof(char*)*cantidad_palabras_no_repetidas);
      for(i=0;i<cantidad_palabras_no_repetidas;i++){
         vocabulario[i]=(char *)malloc(sizeof(char)*MAX_PALABRA);
      }
      j = 0;
      for(i=0;i<cantidad_palabras;i++){ // agrego estas palabras unicaas al vocabulario que entregare al proceso 0
         if(existe(palabras_del_documento[i],cantidad_palabras_no_repetidas,vocabulario)==0){ // si la palabra aun no ha sido agregada, se agrega al voc local
            strcpy(vocabulario[j],palabras_del_documento[i]);
            j++;
         }
      }
      /*for (i = 0; i < cantidad_palabras_no_repetidas; ++i)
      {
         printf("[%i] : %s\n",rank,vocabulario[i] );
      }*/

         MPI_Send(&cantidad_palabras_no_repetidas,1,MPI_INT,0,0,MPI_COMM_WORLD); // envio tamano del vocabulario
         for(i=0;i<cantidad_palabras_no_repetidas;i++){
<<<<<<< HEAD
            MPI_Send(vocabulario[i],MAX_PALABRA,MPI_CHAR,0,1,MPI_COMM_WORLD);
=======
            MPI_Send(vocabulario[i],strlen(vocabulario[i]),MPI_CHAR,0,1,MPI_COMM_WORLD);
>>>>>>> e3e531e9b1e04897ce79406c92d417c414fd3c66
            //printf("PALABRA ENVADA: %s\n",vocabulario[i] );
         }
         //MPI_Send(vocabulario,cantidad_palabras_no_repetidas*MAX_PALABRA,MPI_CHAR,0,1,MPI_COMM_WORLD);
         //printf("ENVIO %i palabras\n",cantidad_palabras_no_repetidas );
         free(vocabulario);
         free(repeticion_palabras);
         free(palabras_del_documento);
   }

   MPI_Barrier(MPI_COMM_WORLD);  
// SS3 ----------------------------------------------------------------------------------------------
// se le envio al proceso 0 el vocabulario local de cada una de los documentos, el los junta e indexa aquella informacion
   if(rank==0){
      char ** vocabulario_general,**vocabulario_auxiliar;
      int cantidad_auxiliar_palabras=0,cantidad_final_palabras=0,a,b,c;
      char ***vocabulario_todos_documentos;
      vocabulario_todos_documentos=(char***)malloc(sizeof(char**)*cantidad_documentos);
      int **cantidad_palabras_por_documento;
      cantidad_palabras_por_documento=(int**)malloc(sizeof(int*)*cantidad_documentos);
      for(i=0;i<cantidad_documentos;i++) cantidad_palabras_por_documento[i] = (int *)malloc(sizeof(int));

      for(i=0;i<cantidad_documentos;i++){
         int j,k,cantidad_palabras_local;
         char **vocabulario_local;

         MPI_Status *estado = (MPI_Status*)malloc(sizeof(MPI_Status));
         MPI_Status *estado2 = (MPI_Status*)malloc(sizeof(MPI_Status));
         MPI_Recv(&cantidad_palabras_local,1,MPI_INT,i+1,0,MPI_COMM_WORLD,estado);
         cantidad_auxiliar_palabras+=cantidad_palabras_local;
         //printf("Palabras Recibidas: %i\n", cantidad_palabras_local);
         vocabulario_local=(char**)malloc(sizeof(char*)*cantidad_palabras_local);
         for(j=0;j<cantidad_palabras_local;j++) vocabulario_local[j]=(char*)malloc(sizeof(char)*MAX_PALABRA);
        // MPI_Recv(vocabulario_local,MAX_PALABRA*cantidad_palabras_local,MPI_CHAR,i+1,1,MPI_COMM_WORLD,estado2);
         for(k=0;k<cantidad_palabras_local;k++){ // recibo por proceso todas las palabras y las guardo en un vocabulario local (por documento)
            MPI_Recv(vocabulario_local[k],MAX_PALABRA,MPI_CHAR,i+1,1,MPI_COMM_WORLD,estado2);
         }
         cantidad_palabras_por_documento[i][0]=cantidad_palabras_local;
         vocabulario_todos_documentos[i]=vocabulario_local;
      }
      // creo un vocabulario auxiliar que pasaremos a reducir
      vocabulario_auxiliar = (char **)malloc(sizeof(char*)*cantidad_auxiliar_palabras);
      for(i=0;i<cantidad_auxiliar_palabras;i++) vocabulario_auxiliar[i] = (char *)malloc(sizeof(char)*MAX_PALABRA);
         c=0;
      for(i=0;i<cantidad_documentos;i++){
        // printf("Documento %i - Palabras: %i\n", i,cantidad_palabras_por_documento[i][0]);
         for(a=0;a<cantidad_palabras_por_documento[i][0];a++){
            strcpy(vocabulario_auxiliar[c],vocabulario_todos_documentos[i][a]);
            c++;
         }
      }
      /*for(i=0;i<cantidad_auxiliar_palabras;i++){
         printf("Palabra: %s\n",vocabulario_auxiliar[i] );
      }*/

      int *repeticion_palabras = (int *) malloc(sizeof(int)*cantidad_auxiliar_palabras);
      int cantidad=0,j;
      for (i = 0; i < cantidad_auxiliar_palabras; ++i)
      {
         cantidad = repeticion(vocabulario_auxiliar[i],vocabulario_auxiliar,cantidad_auxiliar_palabras);
         repeticion_palabras[i]=cantidad;
      }
      // quito las palabras repetidas y las agrego a mi vocabulario local
      int cantidad_palabras_no_repetidas=0;
      for(i=0;i<cantidad_auxiliar_palabras;i++){
         if(repeticion_palabras[i]!=0){
            cantidad_palabras_no_repetidas++;
            repeticion_palabras[i]=0;
            for (j = i; j < cantidad_auxiliar_palabras; j++)
            {
               if(strncmp(vocabulario_auxiliar[i],vocabulario_auxiliar[j],MAX_PALABRA)==0){
                  repeticion_palabras[j]=0;
               }
            }
         }
      }


      vocabulario_general=(char**)malloc(sizeof(char*)*cantidad_palabras_no_repetidas);
      for(i=0;i<cantidad_palabras_no_repetidas;i++) vocabulario_general[i]=(char*)malloc(sizeof(char)*MAX_PALABRA);
      j = 0;
      for(i=0;i<cantidad_auxiliar_palabras;i++){ // agrego estas palabras unicaas al vocabulario que entregare al proceso 0
         if(existe(vocabulario_auxiliar[i],cantidad_palabras_no_repetidas,vocabulario_general)==0){ // si la palabra aun no ha sido agregada, se agrega al voc local
            strcpy(vocabulario_general[j],vocabulario_auxiliar[i]);
            j++;
         }
      }
   
      free(vocabulario_auxiliar);
      //free(cantidad_auxiliar_palabras); // CREO INDEX
      int *palabras_en_los_documentos = (int *)malloc(sizeof(int)*cantidad_palabras_no_repetidas);
      for(i=0;i<cantidad_palabras_no_repetidas;i++) palabras_en_los_documentos[i]=0;
      for(i=0;i<cantidad_palabras_no_repetidas;i++){
         for(j=0;j<cantidad_documentos;j++){
            for(k=0;k<cantidad_palabras_por_documento[j][0];k++){
               if(strncmp(vocabulario_general[i],vocabulario_todos_documentos[j][k],MAX_PALABRA)==0){
                  palabras_en_los_documentos[i]+=1;
                  break;
               }
            }
         }
      }
      // ESCRIBO EN EL Vocabulario.txt la salida del index
      /*for(i=0;i<cantidad_palabras_no_repetidas;i++){
         printf("%s, %i, %i\n",vocabulario_general[i],i,palabras_en_los_documentos[i] );
      }*/

      FILE *salida = fopen("vocabulario.txt","w");
      for(i=0;i<cantidad_palabras_no_repetidas;i++){
         fprintf(salida,"%s, %i, %i\n",vocabulario_general[i],i,palabras_en_los_documentos[i] );
      }
      fclose(salida);

   }  


   MPI_Finalize();                               /* Finaliza MPI */
   return 0;
}

int repeticion(char *palabra, char ** palabras_documento, int cantidad_palabras){
   int i,resultado=0,diferencia;
   for(i=0;i<cantidad_palabras;i++){
      diferencia=strncmp(palabra,palabras_documento[i],MAX_PALABRA);
      if(diferencia==0){
         resultado++;
      }
   }
   return resultado;
}

int existe(char *palabra,int cantidad_palabras,char **vocabulario){
   int i;
   for(i=0;i<cantidad_palabras;i++){
      if(strncmp(palabra,vocabulario[i],MAX_PALABRA)==0){
         return 1;
      }
   }return 0;
}

int esStopword(char *palabra,char **stopwords,int tamanoVectorStop){
   int i;
   /*for(i=0;i<tamanoVectorStop;i++){
      printf("COMPARANDO- %s - %s\n",palabra,stopwords[i] );
      if(strncmp(palabra,stopwords[i],MAX_PALABRA)==0){
         return 1;
      }
   }*/
   return 0;

}