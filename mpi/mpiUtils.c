#include <mpi.h>
#include "constants.h"
#define MASTER_RANK 0

int isMaster(void){
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	return rank == MASTER_RANK;
}

int getProcessRank(void){
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	return rank;
}

Neigs getNeighbors(void){
	int rank, nProc;
	Neigs neighbors;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &nProc);
	if(rank == 0){
		neighbors.top = -1;
		neighbors.bottom = rank+1;
	}else if(rank == nProc-1){
		neighbors.top = rank-1;
		neighbors.bottom = -1;
	}else{
		neighbors.top = rank-1;
		neighbors.bottom = rank+1;
	}
	return neighbors;
}
