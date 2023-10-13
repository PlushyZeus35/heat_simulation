#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> 
#include <string.h>
#include <math.h>
#include "pngwriter.h"

// Problem configuration
#define DIFFUSION_CONSTANT 0.1
#define X_GRID 0.01
#define Y_GRID 0.01
#define X_GRID2 X_GRID*X_GRID
#define Y_GRID2 Y_GRID*Y_GRID
#define DT X_GRID2 * Y_GRID2 / (2.0 * DIFFUSION_CONSTANT * (X_GRID2 + Y_GRID2))
#define ARR_X_LENGTH 200
#define ARR_Y_LENGTH 200
#define REGULAR_TEMP 50.0
#define MAX_TEMP 100.0
#define MIN_TEMP 0.1
#define EMPTY -1.0

#define RADIUS 500  // Radio del círculo
#define CENTER_X 50  // Coordenada x del centro
#define CENTER_Y 50  // Coordenada y del centro

int THREAD_NUMBER = 100;

struct ProblemConfiguration {
	float dx2;
    float dy2;
    float dt;
	float* arr;
    float* auxArr;
};

// Function definitions
void initArrData(float*);
//void* threadFunction(void*);
int getArrIndex(int, int);
void showArr(float*);
void saveStatusPng(float*, int);
float calcTemp(float, float, float, float, float);
void initProblemConfiguration(struct ProblemConfiguration*, float*, float*);

int main(){
	float* arrayData;
	struct ProblemConfiguration* problemConfiguration;
	arrayData = malloc(ARR_X_LENGTH * ARR_Y_LENGTH * sizeof(float));
	problemConfiguration = malloc(1 * sizeof(struct ProblemConfiguration));
	initArrData(arrayData);
	saveStatusPng(arrayData, 2);
	//initProblemConfiguration(problemConfiguration);

	/*
	long threadId;
	// Initialize array data
	float* arrData;
	arrData = malloc(ARR_X_LENGTH * ARR_Y_LENGTH * sizeof(float));
	// Set radius hole
	float radius = (ARR_X_LENGTH/6.0) * (ARR_X_LENGTH/6.0);
	initArrData(arrData, radius, ARR_X_LENGTH/2, 5*ARR_Y_LENGTH/6);
	showArr(arrData);

	// Initialize struct data
	struct ThreadData* threadData;	
	threadData = malloc(THREAD_NUMBER*sizeof(struct ThreadData));

	// Initialize thread handlers
	pthread_t* threadHandlers;
	threadHandlers = malloc(THREAD_NUMBER*sizeof(pthread_t));
	// Create threads
	for(threadId=0; threadId<THREAD_NUMBER; threadId++){
		// Initialize struct data
		threadData[threadId].threadId = threadId;
		strcpy(threadData[threadId].name, "Test");
		pthread_create(&threadHandlers[threadId], NULL, threadFunction, &threadData[threadId]);
	}

	// Wait for threads
	for(threadId=0; threadId<THREAD_NUMBER; threadId++){
		pthread_join(threadHandlers[threadId], NULL);
	}
	printf("suma total %d", sum);

	// Free memory
	free(threadHandlers);
	free(threadData);
	free(arrData);*/
}

void initArrData(float *arr){
	int i, j;
	for(i=0; i<ARR_Y_LENGTH; i++){
		for(j=0; j<ARR_X_LENGTH; j++){
			arr[getArrIndex(i, j)] = REGULAR_TEMP;

			// Draw empty circle
			double distance = (i - CENTER_X) * (i - CENTER_X) + (j - CENTER_Y) * (j - CENTER_Y);
            if (distance <= RADIUS) {
                arr[getArrIndex(i, j)] = EMPTY;
            }
		}
		arr[getArrIndex(i, 0)] = MIN_TEMP;
		arr[getArrIndex(i, ARR_X_LENGTH-1)] = MAX_TEMP;
	}
}

/*void* threadFunction(void *arg){
	struct ThreadData *threadData = (struct ThreadData *)arg;
	sum++;
	// printf("Soy %ld, con nombre %s y sumo %d\n", threadData->threadId, threadData->name, sum);
} */

int getArrIndex(int y, int x){
	return y * ARR_X_LENGTH + x;
}

void showArr(float *arr){
	int i,j;
	for(i=0; i<ARR_Y_LENGTH; i++){
		printf("|");
		for(j=0; j<ARR_X_LENGTH; j++){
			printf(" %f ", arr[getArrIndex(i,j)]);
		}
		printf("|\n");
	}
}

float calcTemp(float prevX, float prevY, float postX, float postY, float actual){
	return actual + DIFFUSION_CONSTANT * DT * ( (prevY - 2.0*actual + postY)/X_GRID2 + (prevX - 2.0*actual + postX)/Y_GRID2);
}

void saveStatusPng(float* arr, int stepNum){
    char filename[64];
    sprintf(filename, "temp/heat_%05d.png", stepNum);
    save_png(arr, ARR_Y_LENGTH, ARR_X_LENGTH, filename, 'c');
}

int isDiabatic(float* arr, int indexY, int indexX){
	if(indexX>=ARR_X_LENGTH || indexX<0 || indexY<0 || indexY>=ARR_Y_LENGTH){
		return 1;
	}
	if(arr[getArrIndex(indexY, indexX)]==EMPTY){
		return 1;
	}
	return 0;
}

void initProblemConfiguration(struct ProblemConfiguration* probConfig, float* arr, float* auxArr){
	probConfig->arr = arr;
	probConfig->auxArr = auxArr;
}