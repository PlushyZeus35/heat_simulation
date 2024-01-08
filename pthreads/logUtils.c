#include <stdio.h>
#include "pngwriter.h"
#include <stdlib.h>
#include "constants.h"

void showFinishMessage(double time, int nProcs){
	printf("SIMULACIÓN FINALIZADA\n");
	printf("----------------------\n");
	printf("Tamaño de matriz %d x %d\n", ARR_X_LENGTH, ARR_Y_LENGTH);
	printf("Numero de iteraciones: %d\n", NUM_STEPS);
	printf("Numero de procesos: %d\n", nProcs);
	printf("\x1b[34mTiempo total de ejecucion: %f\n", time);
	printf("\x1b[0m-------------------------------------\n");
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
