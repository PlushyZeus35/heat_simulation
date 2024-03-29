#ifndef LOGUTILS_H
#define LOGUTILS_H

void showFinishMessage(double time, int nProcs, int nThreads);
void showInitMessage(void);
void stampArray(float* arr, int iteration, int rank);
void showArr(float* arr, int counter);

#endif
