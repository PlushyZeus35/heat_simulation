#include <stdlib.h>
#include <stdio.h>
#include "constants.h"
#include <string.h>
#include "simulationUtils.h"

float heatFormula(float actual, float prevY, float postY, float prevX, float postX){
	float dx2 = X_GRID * X_GRID;
	float dy2 = Y_GRID * Y_GRID;
	float dt = dx2 * dy2 / (2.0 * DIFFUSION_CONSTANT * (dx2 + dy2));
    return actual + DIFFUSION_CONSTANT * dt * ( (prevY - 2.0*actual + postY)/dx2 + (prevX - 2.0*actual + postX)/dy2 );
}

void calcPointHeat(float* oldPlateInfo, float* plateInfo, int index){
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

int getArrIndex(int y, int x){
	return y * ARR_X_LENGTH + x;
}

float* initArrData(void){
	int totalCells = ARR_X_LENGTH * ARR_Y_LENGTH;
	float* arr = (float *)malloc(totalCells * sizeof(float));
	for(int i=0; i<ARR_Y_LENGTH; i++){
		for(int j=0; j<ARR_X_LENGTH; j++){
			double distance = (i - CENTER_X) * (i - CENTER_X) + (j - CENTER_Y) * (j - CENTER_Y);
            if (distance <= RADIUS) {
            	arr[getArrIndex(i, j)] = EMPTY;
            }else{
				arr[getArrIndex(i,j)] = REGULAR_TEMP;
			}
		}
		arr[getArrIndex(i, 0)] = MIN_TEMP;
		arr[getArrIndex(i, ARR_X_LENGTH-1)] = MAX_TEMP;
	}
	return arr;
}

float* initArrAuxData(float* arr){
	int totalCells = ARR_X_LENGTH * ARR_Y_LENGTH;
	float* arrAux = (float *)malloc(totalCells * sizeof(float));
	memcpy(arrAux, arr, totalCells * sizeof(float));
	return arrAux;
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

void calcTest(float* arr, int i, int j){
	arr[getArrIndex(i,j)] = 80.0;
}
