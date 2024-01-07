#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> 
#include <string.h>
#include "pngwriter.h"
#include <semaphore.h>
#include <sys/time.h>

// Problem configuration
#define DIFFUSION_CONSTANT 0.1
#define X_GRID 0.01
#define Y_GRID 0.01
#define ARR_X_LENGTH 500
#define ARR_Y_LENGTH 500
#define REGULAR_TEMP 50.0
#define MAX_TEMP 100.0
#define MIN_TEMP 0.1
#define EMPTY -1.0
#define EACH_STAMP 1000

#define RADIUS 500  // Radio del círculo
#define CENTER_X 50  // Coordenada x del centro
#define CENTER_Y 50  // Coordenada y del centro

// Global data
int THREAD_NUMBER=8;
int NUM_STEPS=5000;
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
void linealExecution();
void initArrData(struct ThreadData*);
void* threadExecution(void*);
int getArrIndex(int, int);
void showArr(float*);
void saveStatusPng(float*, int);
float heatFormula(float, float, float, float, float);
void initProblemConfiguration(int, char *args[]);
void showInitMessage(int);
void calcPointHeat(int);
int isIndexInLastColumn(int);
int isIndexInFirstColumn(int);
int isIndexInLastRow(int);
int isIndexInFirstRow(int);
int isIndexAbleToEvaluate(float*, int);
int getYaxis(int);
int getXaxis(int);
void showFinishMessage(double, int, int);

int main(int argc, char *argv[]){
	float* temp;
	struct ThreadData* threadData;

	// Init main info
	initProblemConfiguration(argc, argv);
	threadData = malloc((THREAD_NUMBER+1)*sizeof(struct ThreadData));
	plateInfo = malloc(totalCells * sizeof(float));
	oldPlateInfo = malloc(totalCells * sizeof(float));

	// Sync utilities
	pthread_barrier_init(&barrier, NULL, THREAD_NUMBER+1);
	sem_init(&sem, 0, THREAD_NUMBER);
	
	initArrData(threadData);
	memcpy(oldPlateInfo, plateInfo, totalCells * sizeof(float));

	if(argc>1 && atoi(argv[1])!=-1 && atoi(argv[1])==0){
		// Ejecutar version lineal
		linealExecution();
		exit(1);
	}

	showInitMessage(1);

	// Create threads
	int threadId;
	pthread_t* threadHandlers;
	threadHandlers = malloc(THREAD_NUMBER*sizeof(pthread_t));
	
	gettimeofday(&start, NULL);

	for(threadId=0; threadId<THREAD_NUMBER; threadId++){
		pthread_create(&threadHandlers[threadId], NULL, threadExecution, &threadData[threadId]);
	}

	int i,j;
	for(j=0; j<NUM_STEPS; j++){
		for(i=0; i<threadData[THREAD_NUMBER].howMany; i++){
			calcPointHeat(threadData[THREAD_NUMBER].cells[i]);
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

		for (int i = 0; i < THREAD_NUMBER; i++) {
        	sem_post(&sem); // Incrementar el semáforo en 1 valor
    	}
	}

	// Wait for threads
	for(threadId=0; threadId<THREAD_NUMBER; threadId++){
		pthread_join(threadHandlers[threadId], NULL);
	}
	
	gettimeofday(&end, NULL);
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + useconds / 1e6;
	showFinishMessage(elapsed, THREAD_NUMBER, NUM_STEPS);

	// Free memory 
	free(threadHandlers);
	free(plateInfo);
	free(oldPlateInfo);
	free(threadData);
	pthread_barrier_destroy(&barrier);
	sem_destroy(&sem);
}

void linealExecution(){
	showInitMessage(0);
	float* temp;
	gettimeofday(&start, NULL);
	for (int n = 0; n <= NUM_STEPS; n++)
    {
        // Going through the entire area
        // Loop each row (y axis)
        for (int i = 0; i < totalCells; i++)
        {
            calcPointHeat(i);
        }
        // Write the output if needed
        if (n % EACH_STAMP == 0)
        {
            saveStatusPng(plateInfo, n);
        }
        // 
        temp=plateInfo;
        plateInfo=oldPlateInfo;
        oldPlateInfo=temp;
    }
	gettimeofday(&end, NULL);
    seconds = end.tv_sec - start.tv_sec;
    useconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + useconds / 1e6;
	showFinishMessage(elapsed, 1, NUM_STEPS);
}

void initArrData(struct ThreadData* threadData){
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
	int numOfThreads = THREAD_NUMBER +1;
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
	for(j=0; j<NUM_STEPS; j++){
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

void initProblemConfiguration(int argc, char *args[]){
	if(argc==3){
		if(atoi(args[2])!=-1)
			NUM_STEPS = atoi(args[2]);
	}
	if(argc==4){
		if(atoi(args[2])!=-1)
			NUM_STEPS = atoi(args[2]);
		if(atoi(args[3])!=-1)
			THREAD_NUMBER = atoi(args[3]);
	}
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

void showFinishMessage(double time, int numThreads, int numSteps){
	printf("SIMULACIÓN FINALIZADA\n");
	printf("----------------------\n");
	printf("Tamaño de matriz %d x %d\n", ARR_X_LENGTH, ARR_Y_LENGTH);
	printf("Numero de iteraciones: %d\n", NUM_STEPS);
	printf("Numero de hilos: %d\n", numThreads);
	printf("\x1b[34mTiempo total de ejecución: %f\n", time);
	printf("\x1b[0m-------------------------------------\n");
}

void showInitMessage(int mode){
	if(mode){
		// Ejecucion paralelizada
		printf("INICIANDO SIMULACIÓN PARALELIZADA\n");
		printf("Numero de hilos: %d\n", THREAD_NUMBER);
		printf("Numero de iteraciones de tiempo: %d\n", NUM_STEPS);
		printf("-------------------------------------\n");
	}else{
		// Ejecucion lineal
		printf("INICIANDO SIMULACIÓN LINEAL\n");
		printf("Numero de iteraciones de tiempo: %d\n", NUM_STEPS);
		printf("-------------------------------------\n");
	}
}
