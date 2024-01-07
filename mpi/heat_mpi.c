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
	double startTime, endTime;
	MPI_Init(&argc, &argv);
	
	arr = initArrData();
	arrAux = initArrAuxData(arr);
	
	MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
	// Init time
	startTime = MPI_Wtime();
	
	int rest = ARR_Y_LENGTH % nProcs;
	int rowsPerProcess = (ARR_Y_LENGTH-rest)/nProcs;
	int rowInit = getProcessRank() * rowsPerProcess;
	int rowEnd = rowInit + rowsPerProcess - 1;
	// Hay que sumarle al último proceso el resto (las filas que quedan)
	if(getProcessRank()==nProcs-1){
		rowEnd = ARR_Y_LENGTH - 1;
	}
	Neigs neigs = getNeighbors();
	if(DEBUG==1){
		printf("%d/%d [%d-%d] %d - %d\n", getProcessRank(), nProcs-1, rowInit, rowEnd, neigs.top, neigs.bottom);
	}
	
	for(int k=0; k<NUM_STEPS; k++){
		for(int i=rowInit; i<=rowEnd; i++){
			for(int j=0; j<ARR_X_LENGTH; j++){
				calcPointHeat(arrAux, arr, getArrIndex(i, j));
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
	
	endTime = MPI_Wtime();
	
	if(isMaster()){
		showFinishMessage(endTime-startTime, nProcs);
	}
	
	free(arr);
	free(arrAux);
	
	
	
	MPI_Finalize();
	
}
