#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> 
#include <string.h>
#include "pngwriter.h"
#include <semaphore.h>
#include <sys/time.h>
#include "constants.h"
#include "simulationUtils.h"
#include "logUtils.h"

// Global data
float* plateInfo;
float* oldPlateInfo;
float dt;
float dx2;
float dy2;
int totalCells;

// Time data
struct timeval start, end;
long seconds, useconds;

// Synchronization Elements
pthread_barrier_t barrier;
sem_t sem;

// Thread function prop
void* threadExecution(void*);
int main(int argc, char *argv[]){
	float* temp;
	struct ThreadData* threadData;
	int threadsNumber = 8;
	int totalCells = ARR_X_LENGTH * ARR_Y_LENGTH;
	
	if(argc>1){
		threadsNumber = atoi(argv[1]);
	}
	
	// Init time log
	gettimeofday(&start, NULL);
	
	// Init main info
	plateInfo = (float*)malloc( totalCells * sizeof(float) );
	oldPlateInfo = (float*)malloc( totalCells * sizeof(float) );
	threadData = (struct ThreadData*)malloc((threadsNumber+1)*sizeof(struct ThreadData));

	// Sync utilities
	pthread_barrier_init(&barrier, NULL, threadsNumber+1);
	sem_init(&sem, 0, threadsNumber);
	
	initArrData(plateInfo, threadData, threadsNumber);
	memcpy(oldPlateInfo, plateInfo, totalCells * sizeof(float));

	// Create threads
	int threadId;
	pthread_t* threadHandlers;
	threadHandlers = malloc(threadsNumber*sizeof(pthread_t));

	for(threadId=0; threadId<threadsNumber; threadId++){
		pthread_create(&threadHandlers[threadId], NULL, threadExecution, &threadData[threadId]);
	}

	int i,j;
	for(j=0; j<=NUM_STEPS; j++){
		for(i=0; i<threadData[threadsNumber].howMany; i++){
			calcPointHeat(oldPlateInfo, plateInfo, threadData[threadsNumber].cells[i]);
		}
		pthread_barrier_wait(&barrier);

		// switch plates
		temp = plateInfo;
		plateInfo = oldPlateInfo;
		oldPlateInfo = temp;

		if (j % EACH_STAMP == 0)
        {
        	stampArray(plateInfo, j, 0);
        }

		for (int i = 0; i < threadsNumber; i++) {
        	sem_post(&sem); // Incrementar el semáforo en 1 valor
    	}
	}

	// Wait for threads
	for(threadId=0; threadId<threadsNumber; threadId++){
		pthread_join(threadHandlers[threadId], NULL);
	}
	
	// Calculate execution time
	gettimeofday(&end, NULL);
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + useconds / 1e6;
	showFinishMessage(elapsed, threadsNumber);

	// Free memory 
	free(threadHandlers);
	free(plateInfo);
	free(oldPlateInfo);
	free(threadData);
	pthread_barrier_destroy(&barrier);
	sem_destroy(&sem);
}

void* threadExecution(void* arg){
	struct ThreadData *threadData = (struct ThreadData *)arg;
	int i,j;
	for(j=0; j<=NUM_STEPS; j++){
		// Wait for the main thread to swap data pointers
		sem_wait(&sem);
		// Calc heat on thread points
		for(i=0; i<threadData->howMany; i++){
			calcPointHeat(oldPlateInfo, plateInfo, threadData->cells[i]);
		}
		// Wait for all threads
		pthread_barrier_wait(&barrier);
	}
	// End of execution
	pthread_exit(NULL);
}
