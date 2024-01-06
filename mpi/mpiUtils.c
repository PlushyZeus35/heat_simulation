#include <mpi.h>
#include <stdio.h>
#include "constants.h"
#include "simulationUtils.h"
#include "logUtils.h"

int isMaster(void){
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	return rank == MASTER_RANK;
}

int getProcessRank(void){
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	return rank;
}

Neigs getNeighbors(void){
	int rank, nProc;
	Neigs neighbors;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProc);
	if(rank == 0){
		neighbors.top = -1;
		neighbors.bottom = rank+1;
	}else if(rank == nProc-1){
		neighbors.top = rank-1;
		neighbors.bottom = -1;
	}else{
		neighbors.top = rank-1;
		neighbors.bottom = rank+1;
	}
	return neighbors;
}

void sendRowToNeigs(float* arr, Neigs neigs, int fromRow, int toRow){
	float* rowToSend;
	// Send top row
	if(neigs.top>=0){
		rowToSend = arr;
		rowToSend = rowToSend + getArrIndex(fromRow, 0);
		//printf("\n");
		//showArr(rowToSend, ARR_X_LENGTH);
		//printf("\n");
		MPI_Send(rowToSend, ARR_X_LENGTH, MPI_FLOAT, neigs.top, 1, MPI_COMM_WORLD);
	}
	
	// Send bottom row
	if(neigs.bottom>=0){
		rowToSend = arr;
		rowToSend = rowToSend + getArrIndex(toRow-1, 0);
		MPI_Send(rowToSend, ARR_X_LENGTH, MPI_FLOAT, neigs.bottom, 0, MPI_COMM_WORLD);
	}
}

void receiveRowFromNeigs(float* arr, Neigs neigs, int fromRow, int toRow){
	float* rowToReceive;
	if(neigs.top>=0){
		rowToReceive = arr;
		rowToReceive = rowToReceive + getArrIndex(fromRow-1, 0);
		MPI_Recv(rowToReceive, ARR_X_LENGTH, MPI_FLOAT, neigs.top, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		/*float* copyAux = arr;
		copyAux = copyAux + getArrIndex(fromRow-1, 0);
		for(int i=0; i<ARR_X_LENGTH; i++){
			copyAux[i] = rowToReceive[i];
		}*/
	}
	//free(rowToReceive);
	//rowToReceive = (float*)malloc(ARR_X_LENGTH * sizeof(float));
	if(neigs.bottom>=0){
		rowToReceive = arr;
		rowToReceive = rowToReceive + getArrIndex(toRow+1, 0);
		MPI_Recv(rowToReceive, ARR_X_LENGTH, MPI_FLOAT, neigs.bottom, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		/*float* copyAux = arr;
		copyAux = copyAux + getArrIndex(toRow+1, 0);
		for(int i=0; i<ARR_X_LENGTH; i++){
			copyAux[i] = rowToReceive[i];
		}*/
	}
	//free(rowToReceive);
}

void sendUpdateToMaster(float* arr, int rowInit, int rowEnd){
	float* aux;
	aux = arr;
	aux = aux + getArrIndex(rowInit, 0);
	int howMany = (rowEnd - rowInit) * ARR_X_LENGTH;
	howMany = howMany + ARR_X_LENGTH;
	MPI_Send(aux, howMany, MPI_FLOAT, MASTER_RANK, 2, MPI_COMM_WORLD);
}

void receiveUpdatesFromProcess(float* arr, int nProcs){
	float* rowToReceive;
	int howMany;
	int rest = ARR_Y_LENGTH % nProcs;
	int rowsPerProcess = (ARR_Y_LENGTH-rest)/nProcs;
	
	for(int i=0; i<nProcs; i++){
		if(i != MASTER_RANK){
			int rowInit = i * rowsPerProcess;
			rowToReceive = arr;
			rowToReceive = rowToReceive + getArrIndex(rowInit, 0);
			int rowEnd = rowInit + rowsPerProcess - 1;
			// Hay que sumarle al último proceso el resto (las filas que quedan)
			if(i==nProcs-1){
				rowEnd = ARR_Y_LENGTH - 1;
			}
			howMany = (rowEnd - rowInit) * ARR_X_LENGTH;
			howMany = howMany + ARR_X_LENGTH;
			MPI_Recv(rowToReceive, howMany, MPI_FLOAT, i, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		}
	}
}













