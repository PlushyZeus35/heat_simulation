#include <stdio.h>
#include <pthread.h>
#include <stdlib.h> 
#include <string.h>
#include "pngwriter.h"

// Problem configuration
#define DIFFUSION_CONSTANT 0.1
#define X_GRID 0.01
#define Y_GRID 0.01
#define ARR_X_LENGTH 20
#define ARR_Y_LENGTH 20
#define REGULAR_TEMP 50.0
#define MAX_TEMP 100.0
#define MIN_TEMP 0.1
#define EMPTY -1.0

#define RADIUS 0  // Radio del círculo
#define CENTER_X 50  // Coordenada x del centro
#define CENTER_Y 50  // Coordenada y del centro

int THREAD_NUMBER = 30;

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

// Function definitions
void initArrData(float*);
void* threadExecution(void*);
int getArrIndex(int, int);
void showArr(float*);
void saveStatusPng(float*, int);
float heatFormula(struct ProblemConfiguration*, float, float, float, float, float);
void initProblemConfiguration(struct ProblemConfiguration*, float*, float*);
void initThreadData(struct ThreadData*, struct ProblemConfiguration*);
void calcPointHeat(struct ProblemConfiguration*, int);
int isIndexInLastColumn(int);
int isIndexInFirstColumn(int);
int isIndexInLastRow(int);
int isIndexInFirstRow(int);

int main(){
	printf("19 es %d\n", isIndexInLastColumn(19));
	float* arrayData;
	float* auxArrData;
	float* temp;
	struct ProblemConfiguration* problemConfiguration;
	arrayData = malloc(ARR_X_LENGTH * ARR_Y_LENGTH * sizeof(float));
	auxArrData = malloc(ARR_X_LENGTH * ARR_Y_LENGTH * sizeof(float));
	problemConfiguration = malloc(1 * sizeof(struct ProblemConfiguration));
	initArrData(arrayData);
	memcpy(auxArrData, arrayData, ARR_X_LENGTH * ARR_Y_LENGTH * sizeof(float));
	saveStatusPng(arrayData, 0);
	//saveStatusPng(arrayData, 2);
	initProblemConfiguration(problemConfiguration, arrayData, auxArrData);

	// Create threads
	long threadId;
	pthread_t* threadHandlers;
	threadHandlers = malloc(THREAD_NUMBER*sizeof(pthread_t));
	struct ThreadData* threadData;
	threadData = malloc(THREAD_NUMBER*sizeof(struct ThreadData));
	initThreadData(threadData, problemConfiguration);
	for(threadId=0; threadId<THREAD_NUMBER; threadId++){
		pthread_create(&threadHandlers[threadId], NULL, threadExecution, &threadData[threadId]);
	}

	// Wait for threads
	for(threadId=0; threadId<THREAD_NUMBER; threadId++){
		pthread_join(threadHandlers[threadId], NULL);
	}
	saveStatusPng(arrayData, 1);
	// Free memory 
	free(threadHandlers);
	free(arrayData);
	free(auxArrData);
	free(threadData);
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

//! Dividir las celdas a calcular entre el número de hilos (ponerlo en cada threadData)
void initThreadData(struct ThreadData* threadData, struct ProblemConfiguration* problemConfiguration){
	int i;
	int totalCells;
	for(i=0; i<(ARR_X_LENGTH*ARR_Y_LENGTH); i++){
		if(!isIndexInFirstColumn(i) && !isIndexInFirstColumn(i+1) && problemConfiguration->arr[i]!=EMPTY){
			totalCells++;
		}
	}
	printf("total cells %d\n", totalCells);
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
	/*
	int threadId;
	int totalCells = ARR_X_LENGTH * ARR_Y_LENGTH;
	totalCells = totalCells - (ARR_Y_LENGTH*2);
	int rest = totalCells%THREAD_NUMBER;
	int numCells = (totalCells-rest)/THREAD_NUMBER;
	int cellIndex = 0;
	for(threadId=0; threadId<THREAD_NUMBER; threadId++){
		int howManyCells = numCells;
		int j;
		if(rest){
			howManyCells++;
			rest--;
		}
		threadData[threadId].cells = malloc(howManyCells*sizeof(int));
		threadData[threadId].howMany = howManyCells;
		threadData[threadId].problemConfiguration = problemConfiguration;
		for(j=0; j<(ARR_X_LENGTH*ARR_Y_LENGTH); j++){
			printf("Evaluando %d\n", cellIndex);
			if(!isIndexInFirstColumn(cellIndex) && !isIndexInFirstColumn(cellIndex+1)){
				printf("Poniendolo\n");
				threadData[threadId].cells[j] = cellIndex;
			}
			cellIndex++;
		}
	}*/
}

//! Esta función no funciona
int isIndexInLastColumn(int index){
	int next = index+1;
	int total = ARR_X_LENGTH*ARR_Y_LENGTH;

	if(index==19){
		printf("next %d total %d y resto %d\n", next, total, next%total);
	}
	if(next%total==0){
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
	int cellsTam = sizeof(threadData->cells)/sizeof(threadData->cells[0]);
	printf("Soy hilo y tengo %d a mi cargo %d\n", threadData->howMany, cellsTam);
	int i;
	for(i=0; i<threadData->howMany; i++){
		//calcPointHeat(threadData->problemConfiguration, threadData->cells[i]);
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

float heatFormula(struct ProblemConfiguration* problemConfiguration, float actual, float prevY, float postY, float prevX, float postX){
    return actual + DIFFUSION_CONSTANT * problemConfiguration->dt * ( (prevY - 2.0*actual + postY)/problemConfiguration->dx2 + (prevX - 2.0*actual + postX)/problemConfiguration->dy2 );
}

void saveStatusPng(float* arr, int stepNum){
    char filename[64];
    sprintf(filename, "temp/heat_%05d.png", stepNum);
    save_png(arr, ARR_Y_LENGTH, ARR_X_LENGTH, filename, 'c');
}

void initProblemConfiguration(struct ProblemConfiguration* probConfig, float* arr, float* auxArr){
	probConfig->arr = arr;
	probConfig->auxArr = auxArr;
	probConfig->dx2 = X_GRID*X_GRID;
	probConfig->dy2 = Y_GRID*Y_GRID;
	probConfig->dt = probConfig->dx2 * probConfig->dy2 / (2.0 * DIFFUSION_CONSTANT * (probConfig->dx2 + probConfig->dy2));
}

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
        problemConfiguration->arr[cellNumber]=actual;  // si es un punto que no existe (centro del disco no hay que evaluar su calor)
    else
        problemConfiguration->arr[cellNumber] = heatFormula(problemConfiguration, actual, prevY, postY, prevX, postX);
        //problemConfiguration->arr[index] = actual + a * problemConfiguration->dt * ( (prevY - 2.0*actual + postY)/problemConfiguration->dx2 + (prevX - 2.0*actual + postX)/problemConfiguration->dy2 );
}