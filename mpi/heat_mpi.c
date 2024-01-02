#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include "mpiUtils.h"
#include "simulationUtils.h"
#include "logUtils.h"
#include "constants.h"
#include <string.h>

int main(int argc, char** argv){
	int nProcs, rank;
	float* arr;
	float* arrAux;
	MPI_Init(&argc, &argv);
	// Separación de comunicadores
	/*MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	int color = (rank < nProcs/2) ? 0 : 1;
	MPI_Comm newComm;
	MPI_Cart_create(MPI_COMM_WORLD,  color, 0, &newComm);*/
	
	// Nuevos valores con los comunicadores
	/*MPI_Comm_size(newComm, &nProcs);
	MPI_Comm_rank(newComm, &rank);
	if(rank==0){
		
		char mensaje[] = "sdf";
		int tam = 1;
		int etiqueta = 1;
		printf("Soy %d y envio a %d\n", rank, 1);
		MPI_Send(mensaje, tam, MPI_INT, 1, etiqueta, newComm);
	}else if(rank == 1){
		char mensaje[50];
		int tam = 1;
		int etiqueta = 1;
		MPI_Recv(&mensaje, tam, MPI_INT, 0, etiqueta, newComm, MPI_STATUS_IGNORE);
		printf("Soy %d y he recibido esto %s\n",1, mensaje);
	}*/
	arr = initArrData();
	arrAux = (float *)malloc(ARR_X_LENGTH * ARR_Y_LENGTH * sizeof(float));
	memcpy(arrAux, arr, ARR_X_LENGTH * ARR_Y_LENGTH * sizeof(float));
	MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
	if(isMaster()){
		//showInitMessage();
		//stampArray(arr, 1, getProcessRank());
	}
	
	int rest = ARR_X_LENGTH % nProcs;
	int rowsPerProcess = (ARR_X_LENGTH-rest)/nProcs;
	int rowInit = getProcessRank() * rowsPerProcess;
	int rowEnd = rowInit + rowsPerProcess - 1;
	// Hay que sumarle al último proceso el resto (las filas que quedan)
	if(getProcessRank()==nProcs-1){
		rowEnd = ARR_X_LENGTH - 1;
	}
	Neigs neigs = getNeighbors();
	printf("%d/%d [%d-%d] %d - %d\n", getProcessRank(), nProcs-1, rowInit, rowEnd, neigs.top, neigs.bottom);
	for(int i=rowInit; i<=rowEnd; i++){
		for(int j=0; j<ARR_Y_LENGTH; j++){
			calcPointHeat(arrAux, arr, getArrIndex(i, j));
			//calcTest(arrAux, i, j);
		}
	}
	// Enviar a neigs la fila de valores necesaria
	// Recibir de sus neigs la final de valores necesaria
	// Intercambiar array
	// Bucle iteraciones
	stampArray(arr, 1, getProcessRank());
	free(arr);
	
	/*if(isMaster()){
		printf("SOY MASTER\n");
		arr = initArrData();
		printf("%f", arr[0]);
		stampArray(arr, 1, 0);
		printf("SAVED");
		free(arr);
	}else{
		printf("NO SOY NAIDE\n");
	}*/
	
	//MPI_Comm_size(newComm, &nProcs);
	//MPI_Comm_rank(newComm, &rank);
	//printf("Hello from processor %d of %d\n", rank, nProcs-1);
	
	MPI_Finalize();
	
}
