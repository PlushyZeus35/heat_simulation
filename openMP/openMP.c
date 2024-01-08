#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "constants.h"
#include "simulationUtils.h"
#include "logUtils.h"
#include <sys/time.h>

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
			// Split all cells into all created threads
			#pragma omp for
			for(int i=0; i<(ARR_X_LENGTH*ARR_Y_LENGTH); i++){
				calcPointHeat(arrAux, arr, i);
			}
			
			// Barrier to stamp array and switch plates
			#pragma omp barrier
			if(omp_get_thread_num() == 0){
				
				if(k%EACH_STAMP==0){
					stampArray(arr, k, 0);
				}
				temp = arr;
				arr = arrAux;
				arrAux = temp;
			}
			#pragma omp barrier // Wait to master thread to switch plates
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


