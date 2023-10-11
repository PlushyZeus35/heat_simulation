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

int THREAD_NUMBER = 100;
int ARR_X_LENGHT = 50;
int ARR_Y_LENGHT = 5;
int REGULAR_TEMP = 50.0;
int sum = 0;
struct ThreadData {
	long threadId;
	char name[25];
};

// Function definitions
void initArrData(float*);
void* threadFunction(void*);
int getArrIndex(int, int);
void showArr(float*);
float calcTemp(float, float, float, float, float);

int main(){
	long threadId;
	// Initialize array data
	float* arrData;
	arrData = malloc(ARR_X_LENGHT * ARR_Y_LENGHT * sizeof(float));
	initArrData(arrData);
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

void initArrData(float *arr){
	int i,j;
        for(i=0; i<ARR_Y_LENGHT; i++){
                for(j=0; j<ARR_X_LENGHT; j++){
                        arr[getArrIndex(i,j)] = REGULAR_TEMP;
                }
        }
}

void* threadFunction(void *arg){
	struct ThreadData *threadData = (struct ThreadData *)arg;
	sum++;
	// printf("Soy %ld, con nombre %s y sumo %d\n", threadData->threadId, threadData->name, sum);
} 

int getArrIndex(int y, int x){
	return y * ARR_X_LENGHT + x;
}

void showArr(float *arr){
	int i,j;
	for(i=0; i<ARR_Y_LENGHT; i++){
		printf("|");
		for(j=0; j<ARR_X_LENGHT; j++){
			printf(" %f ", arr[getArrIndex(i,j)]);
		}
		printf("|\n");
	}
}

float calcTemp(float prevX, float prevY, float postX, float postY, float actual){
	return 1.0;
}
