// constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

#define DIFFUSION_CONSTANT 0.1
#define X_GRID 0.01
#define Y_GRID 0.01
#define ARR_X_LENGTH 400
#define ARR_Y_LENGTH 500
#define REGULAR_TEMP 50.0
#define MAX_TEMP 100.0
#define MIN_TEMP 0.1
#define EMPTY -1.0
#define EACH_STAMP 1000
#define NUM_STEPS 5000
#define DEBUG 0

#define RADIUS 2000  // Radio del círculo
#define CENTER_X 50  // Coordenada x del centro
#define CENTER_Y 50  // Coordenada y del centro

#define MASTER_RANK 0

struct ThreadData {
	int howMany;
	int* cells;
};

#endif // CONSTANTS_H
