#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "constants.h"
#include "simulationUtils.h"
#include "logUtils.h"
#include <time.h>

int main(int argc, char* argv[]){
	int threadsNumber = THREADS_DEFAULT;
	int thread;
	float* arr;
	float* arrAux;
	float* temp;
	// Time data
	struct timeval start, end;
	long seconds, useconds;
    
	if(argc>1){
		threadsNumber = atoi(argv[1]);
	}
	
	arr = initArrData();
	arrAux = initArrAuxData(arr);
	// Init time log
	gettimeofday(&start, NULL);
	#pragma omp parallel num_threads(threadsNumber) private(thread)
	{
		for(int k=0; k<=NUM_STEPS; k++){
			thread = omp_get_thread_num();
			//printf("soy %d con iteracion %d\n",thread, k);
			#pragma omp for
			for(int i=0; i<(ARR_X_LENGTH*ARR_Y_LENGTH); i++){
				int num_threads = omp_get_num_threads();
				int thread_id = omp_get_thread_num();
				//printf("soy %d y calculo celda %d en iteracion %d\n", thread_id, i, k);
				calcPointHeat(arrAux, arr, i);
			}
			
			int num_threads = omp_get_num_threads();
			int thread_id = omp_get_thread_num();
			//printf("soy %d de %d en iteracion %d\n", thread_id, num_threads, k);
			#pragma omp barrier
			if(thread_id == 0){
				
				if(k%EACH_STAMP==0){
					stampArray(arr, k, 0);
				}
				temp = arr;
				arr = arrAux;
				arrAux = temp;
				//printf("Iteración %d, cambiados los plates!\n", k);
			}
			#pragma omp barrier
		}
			
	}
	
	// Calculate execution time
	gettimeofday(&end, NULL);
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + useconds / 1e6;
	showFinishMessage(elapsed, threadsNumber);
	return 0;
}


