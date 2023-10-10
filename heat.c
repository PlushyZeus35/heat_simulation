#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> 
#include <string.h>

int THREAD_NUMBER = 100;
int ARR_X_LENGHT = 50;
int ARR_Y_LENGHT = 5;
int sum = 0;
struct ThreadData {
	long threadId;
	char name[25];
};

void* threadFunction(void *arg);
int getArrIndex(int x, int y);
void showArr(int* arr);

int main(){
	long threadId;
	// Initialize array data
	int* arrData;
	arrData = malloc(ARR_X_LENGHT * ARR_Y_LENGHT * sizeof(int));
	int i,j;
	for(i=0; i<ARR_Y_LENGHT; i++){
		for(j=0; j<ARR_X_LENGHT; j++){
			arrData[getArrIndex(i,j)] = 50;
		}
	}
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

void* threadFunction(void *arg){
	struct ThreadData *threadData = (struct ThreadData *)arg;
	sum++;
	// printf("Soy %ld, con nombre %s y sumo %d\n", threadData->threadId, threadData->name, sum);
} 

int getArrIndex(int y, int x){
	return y * ARR_X_LENGHT + x;
}

void showArr(int *arr){
	int i,j;
	for(i=0; i<ARR_Y_LENGHT; i++){
		printf("|");
		for(j=0; j<ARR_X_LENGHT; j++){
			printf(" %d ", arr[getArrIndex(i,j)]);
		}
		printf("|\n");
	}
}
