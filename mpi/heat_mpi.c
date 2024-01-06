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
	float* tempAux;
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
	arrAux = initArrAuxData(arr);
	//arrAux = (float *)malloc(ARR_X_LENGTH * ARR_Y_LENGTH * sizeof(float));
	//memcpy(arrAux, arr, ARR_X_LENGTH * ARR_Y_LENGTH * sizeof(float));
	MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
	if(isMaster()){
		//showInitMessage();
		//stampArray(arr, 1, getProcessRank());
	}
	
	int rest = ARR_Y_LENGTH % nProcs;
	int rowsPerProcess = (ARR_Y_LENGTH-rest)/nProcs;
	int rowInit = getProcessRank() * rowsPerProcess;
	int rowEnd = rowInit + rowsPerProcess - 1;
	// Hay que sumarle al último proceso el resto (las filas que quedan)
	if(getProcessRank()==nProcs-1){
		rowEnd = ARR_Y_LENGTH - 1;
	}
	Neigs neigs = getNeighbors();
	printf("%d/%d [%d-%d] %d - %d\n", getProcessRank(), nProcs-1, rowInit, rowEnd, neigs.top, neigs.bottom);
	
	for(int k=0; k<NUM_STEPS; k++){
		for(int i=rowInit; i<=rowEnd; i++){
			for(int j=0; j<ARR_X_LENGTH; j++){
				if(getArrIndex(i,j)>=99999){
					printf("%d calculando fila %d columna %d celda %d\n", getProcessRank(), i, j, getArrIndex(i,j));
				}
				calcPointHeat(arrAux, arr, getArrIndex(i, j));
				//calcTest(arrAux, i, j);
			}
		}
		
		// Enviar a neigs la fila de valores necesaria
		sendRowToNeigs(arrAux, neigs, rowInit, rowEnd);
		// Recibir de sus neigs la final de valores necesaria
		receiveRowFromNeigs(arrAux, neigs, rowInit, rowEnd);
		
		// In each stamp all processes needs to merge all info
		if(k%EACH_STAMP==0){
			if(isMaster()){
				// Recibir una comunicación por cada proceso y juntar la info
				receiveUpdatesFromProcess(arr, nProcs);
				stampArray(arr, k, getProcessRank());
			}else{
				// Enviar a master todo su array
				sendUpdateToMaster(arr, rowInit, rowEnd);
			}
		}
		
		// Intercambiar array
		tempAux = arr;
		arr = arrAux;
		arrAux = tempAux;
		//printf("uno\n");
	}
	
	
	
	free(arr);
	free(arrAux);
	
	
	
	MPI_Finalize();
	
}
