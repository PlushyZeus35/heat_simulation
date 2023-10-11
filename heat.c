#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> 
#include <string.h>

// Problem configuration
#define DIFFUSION_CONSTANT 0.1
#define X_GRID 0.01
#define Y_GRID 0.01
#define X_GRID2 X_GRID*X_GRID
#define Y_GRID2 Y_GRID*Y_GRID
#define DT X_GRID2 * Y_GRID2 / (2.0 * DIFFUSION_CONSTANT * (X_GRID2 + Y_GRID2))
#define ARR_X_LENGTH 200
#define ARR_Y_LENGTH 100
#define REGULAR_TEMP 50.0
#define MAX_TEMP 100.0
#define MIN_TEMP 0.0
#define EMPTY -1.0

int THREAD_NUMBER = 100;
int sum = 0;
struct ThreadData {
	long threadId;
	char name[25];
};

// Function definitions
void initArrData(float*, int, int, int);
void* threadFunction(void*);
int getArrIndex(int, int);
void showArr(float*);
float calcTemp(float, float, float, float, float);

int main(){
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
	free(arrData);
}

void initArrData(float *arr, int radius, int centerX, int centerY){
	int i,j;
        for(i=0; i<ARR_Y_LENGTH; i++){
                for(j=0; j<ARR_X_LENGTH; j++){
					int index = getArrIndex(i, j);
					float rad = (i-centerX) * (i-centerX) + (j-centerY)*(j-centerY);
					if(rad<radius){
						// Set empty hole
						arr[index] = EMPTY;
					}else{
						// Set regular temperature
						arr[index] = REGULAR_TEMP;
					}
					// Set max temp
					if(j==0){
						arr[index] = MAX_TEMP;
					}
					// Set min temp
					if(j==ARR_X_LENGTH-1){
						arr[index] = MIN_TEMP;
					}
                }
        }
}

void* threadFunction(void *arg){
	struct ThreadData *threadData = (struct ThreadData *)arg;
	sum++;
	// printf("Soy %ld, con nombre %s y sumo %d\n", threadData->threadId, threadData->name, sum);
} 

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
