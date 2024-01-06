#include <stdio.h>
#include "pngwriter.h"
#include <stdlib.h>
#include "constants.h"
#include <mpi.h>

void showFinishMessage(double time){
	printf("SIMULACIÓN FINALIZADA\n");
	printf("----------------------\n");
	printf("\x1b[34mTiempo total de ejecución: %f\n", time);
	printf("\x1b[0m-------------------------------------\n");
}

void showInitMessage(){
	int nProcs;
	MPI_Comm_size(MPI_COMM_WORLD, &nProcs);
	printf("INICIANDO SIMULACIÓN CON MPI\n");
	printf("Numero de procesos: %d\n", nProcs);
	printf("Numero de iteraciones de tiempo: %d\n", NUM_STEPS);
	printf("-----------------------------------------------\n");
}

void stampArray(float* arr, int iteration, int rank){
    char filename[64];
    sprintf(filename, "temp/heat_%d_%d.png", iteration, rank);
    save_png(arr, ARR_Y_LENGTH, ARR_X_LENGTH, filename, 'c');
}

void showArr(float* arr, int counter){
	for(int i=0; i<counter; i++){
		printf(" %f ", arr[i]);
	}
}
