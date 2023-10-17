#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> 
#include <string.h>
#include "pngwriter.h"
#include <semaphore.h>

// Problem configuration
#define DIFFUSION_CONSTANT 0.1
#define X_GRID 0.01
#define Y_GRID 0.01
#define ARR_X_LENGTH 200
#define ARR_Y_LENGTH 200
#define REGULAR_TEMP 50.0
#define MAX_TEMP 100.0
#define MIN_TEMP 0.1
#define EMPTY -1.0
#define NUM_STEPS 5000
#define EACH_STAMP 1000

#define RADIUS 500  // Radio del círculo
#define CENTER_X 50  // Coordenada x del centro
#define CENTER_Y 50  // Coordenada y del centro

// Global data
int THREAD_NUMBER = 30;
float* plateInfo;
float* oldPlateInfo;
float dt;
float dx2;
float dy2;
int totalCells;

struct ProblemConfiguration {
	float dx2;
    float dy2;
    float dt;
	float* arr;
    float* auxArr;
};

struct ThreadData {
	int howMany;
	int* cells;
	struct ProblemConfiguration* problemConfiguration;
};
pthread_barrier_t barrier;
sem_t sem;

// Function definitions
void initArrData(struct ThreadData*);
void* threadExecution(void*);
int getArrIndex(int, int);
void showArr(float*);
void saveStatusPng(float*, int);
float heatFormula(float, float, float, float, float);
void initProblemConfiguration();
void initThreadData(struct ThreadData*, struct ProblemConfiguration*);
void calcPointHeat(int, int);
int isIndexInLastColumn(int);
int isIndexInFirstColumn(int);
int isIndexInLastRow(int);
int isIndexInFirstRow(int);
int isIndexAbleToEvaluate(float*, int);
int getYaxis(int);
int getXaxis(int);
void showProblemConfig(struct ProblemConfiguration* problemConfig);

int main(){
	float* temp;
	struct ThreadData* threadData;
	// Sync utilities
	pthread_barrier_init(&barrier, NULL, THREAD_NUMBER+1);
	sem_init(&sem, 0, THREAD_NUMBER);

	// Init main info
	initProblemConfiguration();
	threadData = malloc((THREAD_NUMBER+1)*sizeof(struct ThreadData));
	plateInfo = malloc(totalCells * sizeof(float));
	oldPlateInfo = malloc(totalCells * sizeof(float));
	
	initArrData(threadData);
	memcpy(oldPlateInfo, plateInfo, totalCells * sizeof(float));

	// Create threads
	int threadId;
	pthread_t* threadHandlers;
	threadHandlers = malloc(THREAD_NUMBER*sizeof(pthread_t));
	for(threadId=0; threadId<THREAD_NUMBER; threadId++){
		pthread_create(&threadHandlers[threadId], NULL, threadExecution, &threadData[threadId]);
	}

	int i,j;
	for(j=0; j<NUM_STEPS; j++){
		for(i=0; i<threadData[THREAD_NUMBER].howMany; i++){
			int y = getYaxis(threadData[THREAD_NUMBER].cells[i]);
			int x = getXaxis(threadData[THREAD_NUMBER].cells[i]);
			calcPointHeat(y, x);
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

	// Free memory 
	free(threadHandlers);
	free(plateInfo);
	free(oldPlateInfo);
	free(threadData);
	pthread_barrier_destroy(&barrier);
	sem_destroy(&sem);
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
		int index = actualCell;
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

//! Dividir las celdas a calcular entre el número de hilos (ponerlo en cada threadData)
void initThreadData(struct ThreadData* threadData, struct ProblemConfiguration* problemConfiguration){
	int i;
	int totalCells;
	for(i=0; i<(ARR_X_LENGTH*ARR_Y_LENGTH); i++){
		if(!isIndexInFirstColumn(i) && !isIndexInFirstColumn(i+1) && problemConfiguration->arr[i]!=EMPTY){
			totalCells++;
		}
	}
	int rest = totalCells%THREAD_NUMBER;
	int numCells = (totalCells-rest)/THREAD_NUMBER;
	int indexRecord = 0;
	for(i=0; i<THREAD_NUMBER; i++){
		int howManyCells = numCells;
		int j;
		if(rest){
			howManyCells++;
			rest--;
		}
		threadData[i].cells = malloc(howManyCells*sizeof(int));
		threadData[i].problemConfiguration = problemConfiguration;
		int auxIndex = indexRecord;
		int counter=0;
		while(counter<howManyCells){
			if(!isIndexInFirstColumn(auxIndex) && !isIndexInFirstColumn(auxIndex+1) && problemConfiguration->arr[auxIndex]!=EMPTY){
				threadData[i].cells[counter] = auxIndex;
				counter++;
			}
			auxIndex++;
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
	float* temp;
	for(j=0; j<NUM_STEPS; j++){
		sem_wait(&sem);
		int counter =0;
		for(i=0; i<threadData->howMany; i++){
			int y = getYaxis(threadData->cells[i]);
			int x = getXaxis(threadData->cells[i]);
			calcPointHeat(y, x);
		}
		
		//int ret = pthread_barrier_wait(&(threadArgs->barrier));
		pthread_barrier_wait(&barrier);
	}
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

/*
void calcPointHeat(struct ProblemConfiguration* problemConfiguration, int cellNumber){
    float actual = problemConfiguration->arr[cellNumber];
    // 
    float prevY;
    // Evaluar si hay trans de calor, sino poner valor actual
	if((cellNumber-ARR_X_LENGTH)<0){
		prevY = actual;
	}else if(problemConfiguration->arr[cellNumber-ARR_X_LENGTH]==EMPTY){
		prevY = actual;
	}else{
		prevY = problemConfiguration->arr[cellNumber-ARR_X_LENGTH];
	}

    float prevX;
	if(cellNumber%ARR_X_LENGTH==0){
		prevX = actual;
	}else if(problemConfiguration->arr[cellNumber-1]==EMPTY){
		prevX = actual;
	}else{
		prevX = problemConfiguration->arr[cellNumber-1];
	}

    float postY;
    if((cellNumber+ARR_X_LENGTH)>(ARR_X_LENGTH*ARR_Y_LENGTH)){
		postY = actual;
	}else if(problemConfiguration->arr[cellNumber+ARR_X_LENGTH]==EMPTY){
		postY = actual;
	}else{
		postY = problemConfiguration->arr[cellNumber+ARR_X_LENGTH];
	}

    float postX;
	if(cellNumber+1>=ARR_X_LENGTH*ARR_Y_LENGTH){
		postX = actual;
	}else if(cellNumber+1%ARR_X_LENGTH==0){
		postX = actual;
	}else if(problemConfiguration->arr[cellNumber+1]==EMPTY){
		postX = actual;
	}else{
		postX = problemConfiguration->arr[cellNumber+1];
	}
    
    //Un[index] = calcHeat(actual, prevY, postY, prevX, postX);
    // Explicit scheme
    if (actual<0.0)
        problemConfiguration->auxArr[cellNumber]=actual;  // si es un punto que no existe (centro del disco no hay que evaluar su calor)
    else
        problemConfiguration->auxArr[cellNumber] = heatFormula(problemConfiguration, actual, prevY, postY, prevX, postX);
        //problemConfiguration->arr[index] = actual + a * problemConfiguration->dt * ( (prevY - 2.0*actual + postY)/problemConfiguration->dx2 + (prevX - 2.0*actual + postX)/problemConfiguration->dy2 );
}*/


void calcPointHeat(int indexY, int indexX){
    const int index = getArrIndex(indexY, indexX);
    float actual = oldPlateInfo[index];
    // 
    float prevY;
    if (indexY>0)
        prevY = oldPlateInfo[getArrIndex(indexY-1, indexX)];
    else
        prevY = EMPTY; // no existe como el centro del disco

    if (prevY<0)
        prevY=actual; //si no existe no hay tranferencia de calor por lo es como si vale igual que el punto a evaluar


    float prevX = oldPlateInfo[getArrIndex(indexY, indexX-1)];
    if (prevX<0)
        prevX=actual;  //si no existe no hay tranferencia de calor por lo es como si vale igual que el punto a evaluar

    float postY;
    if (indexY<ARR_X_LENGTH-1)
        postY = oldPlateInfo[getArrIndex(indexY+1, indexX)];
    else
        postY = EMPTY; // no existe como el centro del disco

    if (postY<0)
        postY=actual;  //si no existe no hay tranferencia de calor por lo es como si vale igual que el punto a evaluar


    float postX = oldPlateInfo[getArrIndex(indexY, indexX+1)];
    if (postX<0)
        postX=actual;  //si no existe no hay tranferencia de calor por lo es como si vale igual que el punto a evaluar
    
    // Explicit scheme
    if (actual<0.0)
        plateInfo[index]=actual;  // si es un punto que no existe (centro del disco no hay que evaluar su calor)
    else
        plateInfo[index] = heatFormula(actual, prevY, postY, prevX, postX);
        //problemConfiguration->arr[index] = actual + a * problemConfiguration->dt * ( (prevY - 2.0*actual + postY)/problemConfiguration->dx2 + (prevX - 2.0*actual + postX)/problemConfiguration->dy2 );
}

int getYaxis(int index){
	int rest = index % ARR_X_LENGTH;
	int auxIndex = index - rest;
	return auxIndex / ARR_X_LENGTH;
}

int getXaxis(int index){
	return index % ARR_X_LENGTH;
}

void showProblemConfig(struct ProblemConfiguration* problemConfig){
	printf("PROBLEM CONFIG\n");
	printf("DIFFUSION CONSTANT %f \n", DIFFUSION_CONSTANT);
	printf("ARR X LENGTH %d\n", ARR_X_LENGTH);
	printf("ARR Y LENGTH %d \n", ARR_Y_LENGTH);
	printf("DX %f \n", X_GRID);
	printf("DY %f \n", Y_GRID);
	printf("DX2 %f \n", problemConfig->dx2);
	printf("DY2 %f \n", problemConfig->dy2);
	printf("DT %f \n", problemConfig->dt);
}