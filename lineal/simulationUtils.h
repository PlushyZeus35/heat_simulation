
#ifndef SIMULATIONUTILS_H
#define SIMULATIONUTILS_H
float* initArrData(void);
float* initArrAuxData(float* arr);
int getArrIndex(int y, int x);
void calcTest(float* arr, int i, int j);
float heatFormula(float actual, float prevY, float postY, float prevX, float postX);
void calcPointHeat(float* oldPlateInfo, float* plateInfo, int index);

#endif
