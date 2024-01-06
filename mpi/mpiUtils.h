#ifndef MPIUTILS_H
#define MPIUTILS_H
#include "constants.h"

int isMaster(void);
int getProcessRank(void);
Neigs getNeighbors(void);
void sendRowToNeigs(float*, Neigs, int, int);
void receiveRowFromNeigs(float*, Neigs, int, int);
void sendUpdateToMaster(float* arr, int rowInit, int rowEnd);
void receiveUpdatesFromProcess(float* arr, int nProcs);

#endif
