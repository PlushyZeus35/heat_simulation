#include <stdio.h>
#include "simulationUtils.h"
#include "logUtils.h"
#include "constants.h"
#include <time.h>

int main(void){
	float* arr;
	float* arrAux;
	float* temp;
	arr = initArrData();
	arrAux = initArrAuxData(arr);
	clock_t start = clock();
	for(int k=0; k<=NUM_STEPS; k++){
		for(int i=0; i<ARR_Y_LENGTH; i++){
			for(int j=0; j<ARR_X_LENGTH; j++){
				calcPointHeat(arrAux, arr, getArrIndex(i, j));
			}
		}
		if(k%EACH_STAMP == 0){
			stampArray(arr, k, 1);
		}
		temp = arr;
		arr = arrAux;
		arrAux = temp;
	}
	clock_t finish = clock();
	double time = (double)(finish - start) / CLOCKS_PER_SEC;
	showFinishMessage(time);
}
