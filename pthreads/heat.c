#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> 
#include <string.h>
#include "pngwriter.h"
#include <semaphore.h>
#include <sys/time.h>
#include "constants.h"

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

struct ThreadData {
	int howMany;
	int* cells;
};
pthread_barrier_t barrier;
sem_t sem;

// Function definitions
void initArrData(struct ThreadData*, int);
void* threadExecution(void*);
int getArrIndex(int, int);
void showArr(float*);
void saveStatusPng(float*, int);
float heatFormula(float, float, float, float, float);
void initProblemConfiguration(void);
void calcPointHeat(int);
int isIndexInLastColumn(int);
int isIndexInFirstColumn(int);
int isIndexInLastRow(int);
int isIndexInFirstRow(int);
int isIndexAbleToEvaluate(float*, int);
int getYaxis(int);
int getXaxis(int);
void showFinishMessage(double, int);

int main(int argc, char *argv[]){
	float* temp;
	struct ThreadData* threadData;
	int threadsNumber = 8;
	if(argc>1){
		threadsNumber = atoi(argv[1]);
	}
	
	// Init time log
	gettimeofday(&start, NULL);
	
	// Init main info
	initProblemConfiguration();
	threadData = malloc((threadsNumber+1)*sizeof(struct ThreadData));
	plateInfo = malloc(totalCells * sizeof(float));
	oldPlateInfo = malloc(totalCells * sizeof(float));

	// Sync utilities
	pthread_barrier_init(&barrier, NULL, threadsNumber+1);
	sem_init(&sem, 0, threadsNumber);
	
	initArrData(threadData, threadsNumber);
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
			calcPointHeat(threadData[threadsNumber].cells[i]);
		}
		pthread_barrier_wait(&barrier);

		// switch plates
		temp = plateInfo;
		plateInfo = oldPlateInfo;
		oldPlateInfo = temp;

		if (j % EACH_STAMP == 0)
        {
           saveStatusPng(plateInfo, j);
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

// This method initialize array plate data
// Also divide the array cells among different threads. 
// A cell is considered for computation if it does not have the maximum or minimum temperature and is not a hole.
void initArrData(struct ThreadData* threadData, int threadNumber){
	int i, j, cellsToEvaluate=0;
	for(i=0; i<ARR_Y_LENGTH; i++){
		for(j=0; j<ARR_X_LENGTH; j++){
			// Draw empty circle
			double distance = (i - CENTER_X) * (i - CENTER_X) + (j - CENTER_Y) * (j - CENTER_Y);
            if (distance <= RADIUS) {
                plateInfo[getArrIndex(i, j)] = EMPTY;
            }else{
				plateInfo[getArrIndex(i, j)] = REGULAR_TEMP;
				cellsToEvaluate++;
			}
		}
		plateInfo[getArrIndex(i, 0)] = MIN_TEMP;
		plateInfo[getArrIndex(i, ARR_X_LENGTH-1)] = MAX_TEMP;
	}

	// Split cells to evaluate between all threads
	cellsToEvaluate=cellsToEvaluate - ARR_Y_LENGTH*2;
	int numOfThreads = threadNumber +1;
	int rest = cellsToEvaluate%numOfThreads;
	int cellForEachThread = (cellsToEvaluate-rest)/numOfThreads;
	long threadId;
	int actualCell=0;
	for(threadId=0; threadId<numOfThreads; threadId++){
		int numCells = cellForEachThread;
		if(rest){
			rest--;
			numCells++;
		}
		threadData[threadId].howMany = numCells;
		threadData[threadId].cells = malloc(numCells*sizeof(int));
		int auxIndex = 0;
		while(numCells>0){
			if(isIndexAbleToEvaluate(plateInfo, actualCell)){
				threadData[threadId].cells[auxIndex] = actualCell;
				auxIndex++;
				numCells--;
			}
			actualCell++;
		}
	}
}

int isIndexAbleToEvaluate(float* arr, int index){
	if(isIndexInFirstColumn(index) || isIndexInFirstColumn(index+1) || arr[index]==EMPTY){
		return 0;
	}
	return 1;
}

int isIndexInLastColumn(int index){
	if(isIndexInFirstColumn(index+1)){
		return 1;
	}
	return 0;
}

int isIndexInFirstColumn(int index){
	if(index%ARR_X_LENGTH==0){
		return 1;
	}
	return 0;
}

int isIndexInLastRow(int index){
	if(index + ARR_X_LENGTH > (ARR_X_LENGTH*ARR_Y_LENGTH)){
		return 1;
	}
	return 0;
}

int isIndexInFirstRow(int index){
	if(index-ARR_X_LENGTH<0){
		return 1;
	}
	return 0;
}

void* threadExecution(void* arg){
	struct ThreadData *threadData = (struct ThreadData *)arg;
	int i,j;
	for(j=0; j<=NUM_STEPS; j++){
		// Wait for the main thread to swap data pointers
		sem_wait(&sem);
		// Calc heat on thread points
		for(i=0; i<threadData->howMany; i++){
			calcPointHeat(threadData->cells[i]);
		}
		// Wait for all threads
		pthread_barrier_wait(&barrier);
	}
	// End of execution
	pthread_exit(NULL);
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

float heatFormula(float actual, float prevY, float postY, float prevX, float postX){
    return actual + DIFFUSION_CONSTANT * dt * ( (prevY - 2.0*actual + postY)/dx2 + (prevX - 2.0*actual + postX)/dy2 );
}

void saveStatusPng(float* arr, int stepNum){
    char filename[64];
    sprintf(filename, "temp/heat_%05d.png", stepNum);
    save_png(arr, ARR_Y_LENGTH, ARR_X_LENGTH, filename, 'c');
}

void initProblemConfiguration(){
	totalCells = ARR_X_LENGTH * ARR_Y_LENGTH;
	dx2 = X_GRID * X_GRID;
	dy2 = Y_GRID * Y_GRID;
	dt = dx2 * dy2 / (2.0 * DIFFUSION_CONSTANT * (dx2 + dy2));
}

void calcPointHeat(int index){
    float actual = oldPlateInfo[index];
    if(actual < 0.0 || isIndexInFirstColumn(index) || actual == MAX_TEMP)
		return;
    float prevY;
	if(isIndexInFirstRow(index) || oldPlateInfo[index-ARR_X_LENGTH]==EMPTY){
		prevY = actual;
	}else{
		prevY = oldPlateInfo[index-ARR_X_LENGTH];
	}

    float prevX;
	if(isIndexInFirstColumn(index) || oldPlateInfo[index-1]==EMPTY){
		prevX = actual;
	}else{
		prevX = oldPlateInfo[index-1];
	}

    float postY;
	if(isIndexInLastRow(index) || oldPlateInfo[index+ARR_X_LENGTH]==EMPTY){
		postY = actual;
	}else{
		postY = oldPlateInfo[index+ARR_X_LENGTH];
	}

    float postX;
	if(isIndexInLastColumn(index) || oldPlateInfo[index+1]==EMPTY){
		postX = actual;
	}else{
		postX = oldPlateInfo[index+1];
	}
    
    // Explicit scheme
    if (actual<0.0)
        plateInfo[index]=actual;
    else
        plateInfo[index] = heatFormula(actual, prevY, postY, prevX, postX);
}

int getYaxis(int index){
	int rest = index % ARR_X_LENGTH;
	int auxIndex = index - rest;
	return auxIndex / ARR_X_LENGTH;
}

int getXaxis(int index){
	return index % ARR_X_LENGTH;
}

void showFinishMessage(double time, int numThreads){
	printf("SIMULACIÓN FINALIZADA\n");
	printf("----------------------\n");
	printf("Tamaño de matriz %d x %d\n", ARR_X_LENGTH, ARR_Y_LENGTH);
	printf("Numero de iteraciones: %d\n", NUM_STEPS);
	printf("Numero de hilos: %d\n", numThreads);
	printf("\x1b[34mTiempo total de ejecución: %f\n", time);
	printf("\x1b[0m-------------------------------------\n");
}

