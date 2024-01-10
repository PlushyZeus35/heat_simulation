#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include "mpiUtils.h"
#include "simulationUtils.h"
#include "logUtils.h"
#include "constants.h"
#include <string.h>

int main(int argc, char** argv){
	int nProcs, thread;
	float* arr;
	float* arrAux;
	float* tempAux;
	double startTime, endTime;
	int nThreads = 8;

	if(argc>=1){
		nThreads = atoi(argv[1]);
	}
	
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
	int cellInit = rowInit * ARR_X_LENGTH;
	int cellEnd = (rowEnd * ARR_X_LENGTH) + ARR_X_LENGTH - 1;
	// Hay que sumarle al último proceso el resto (las filas que quedan)
	if(getProcessRank()==nProcs-1){
		rowEnd = ARR_Y_LENGTH - 1;
	}
	Neigs neigs = getNeighbors();
	if(DEBUG==1){
		printf("%d/%d [%d-%d] %d - %d\n", getProcessRank(), nProcs-1, rowInit, rowEnd, neigs.top, neigs.bottom);
	}
	
	#pragma omp parallel num_threads(nThreads) private(thread)
	{
		for(int k=0; k<NUM_STEPS; k++){
			// Split all cells into all created threads
			#pragma omp for
			for(int i=cellInit; i<=cellEnd; i++){
				calcPointHeat(arrAux, arr, i);
			}
			
			// Wait for all threads
			#pragma omp barrier
			
			// Only one thread does the communication between process, stamp array and switch plates
			// The other threads waits due to implicit barrier at the end of pragma single
			#pragma omp single
			{
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
			}
		}
	}
	
	// Meassure time
	endTime = MPI_Wtime();
	
	if(isMaster()){
		showFinishMessage(endTime-startTime, nProcs, nThreads);
	}
	
	free(arr);
	free(arrAux);
	
	
	
	MPI_Finalize();
	
}
