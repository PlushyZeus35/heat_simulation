
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "pngwriter.h"
#define nx 200
#define ny 200
#define a 0.1
#define dx 0.01
#define dy 0.01
#define numSteps 5000
#define outputEvery 1000
#define empty -1.0
#define REGULAR_TEMP 50.0
#define MIN_TEMP 0.1
#define MAX_TEMP 100.0

struct ProblemConfiguration {
	float dx2;
    float dy2;
    float dt;
	float* arr;
    float* auxArr;
};

int getIndex(int, int);
void initializeArr(float*);
void calcPointHeat(struct ProblemConfiguration*, int, int);
float heatFormula(struct ProblemConfiguration*, float, float, float, float, float);
void saveStatusPng(float*, int);
void main()
{
    const float dx2 = dx*dx;
    const float dy2 = dy*dy;
    const float dt = dx2 * dy2 / (2.0 * a * (dx2 + dy2)); // Largest stable time step

    int numElements = nx*ny;

    // Allocate two sets of data for current and next timesteps
    float* Un   = (float*)calloc(numElements, sizeof(float));
    float* auxArr = (float*)calloc(numElements, sizeof(float));
    float* temp;

    // Set problem configuration
    struct ProblemConfiguration* problemConfiguration;
    problemConfiguration = malloc(1*sizeof(struct ProblemConfiguration));
    problemConfiguration->dx2 = dx*dx;
    problemConfiguration->dy2 = dy*dy;
    problemConfiguration->dt = problemConfiguration->dx2 * problemConfiguration->dy2 / (2.0 * a * (problemConfiguration->dx2 + problemConfiguration->dy2));
    problemConfiguration->arr = Un;
    problemConfiguration->auxArr = auxArr;

    // Initializing the data with a pattern of disk of radius of 1/6 of the width
    initializeArr(Un);
    memcpy(auxArr, Un, numElements*sizeof(float));
    // Timing
    clock_t start = clock();

    // Main loop
    for (int n = 0; n <= numSteps; n++)
    {
        // Going through the entire area
        // Loop each row (y axis)
        for (int i = 0; i < nx; i++)
        {
            // Loop each column (x axis)
            for (int j = 1; j < ny-1; j++)
            {
                calcPointHeat(problemConfiguration, i, j);
            }
        }
        // Write the output if needed
        if (n % outputEvery == 0)
        {
            saveStatusPng(Un, n);
        }
        // 
        temp=Un;
        Un=auxArr;
        auxArr=temp;
    }

    // Timing
    clock_t finish = clock();
    printf("It took %f seconds\n", (double)(finish - start) / CLOCKS_PER_SEC);

    // Release the memory
    free(Un);
    free(auxArr);
    printf( "Memory freed\n" );
    
    return ;
}

int getIndex(int i, int j)
{
    return i*nx + j;
}

void calcPointHeat(struct ProblemConfiguration* problemConfiguration, int indexY, int indexX){
    const int index = getIndex(indexY, indexX);
    float actual = problemConfiguration->arr[index];
    // 
    float prevY;
    if (indexY>0)
        prevY = problemConfiguration->arr[getIndex(indexY-1, indexX)];
    else
        prevY = empty; // no existe como el centro del disco

    if (prevY<0)
        prevY=actual; //si no existe no hay tranferencia de calor por lo es como si vale igual que el punto a evaluar


    float prevX = problemConfiguration->arr[getIndex(indexY, indexX-1)];
    if (prevX<0)
        prevX=actual;  //si no existe no hay tranferencia de calor por lo es como si vale igual que el punto a evaluar

    float postY;
    if (indexY<nx-1)
        postY = problemConfiguration->arr[getIndex(indexY+1, indexX)];
    else
        postY = empty; // no existe como el centro del disco

    if (postY<0)
        postY=actual;  //si no existe no hay tranferencia de calor por lo es como si vale igual que el punto a evaluar


    float postX = problemConfiguration->arr[getIndex(indexY, indexX+1)];
    if (postX<0)
        postX=actual;  //si no existe no hay tranferencia de calor por lo es como si vale igual que el punto a evaluar
    
    //Un[index] = calcHeat(actual, prevY, postY, prevX, postX);
    // Explicit scheme
    if (actual<0.0)
        problemConfiguration->arr[index]=actual;  // si es un punto que no existe (centro del disco no hay que evaluar su calor)
    else
        problemConfiguration->arr[index] = heatFormula(problemConfiguration, actual, prevY, postY, prevX, postX);
        //problemConfiguration->arr[index] = actual + a * problemConfiguration->dt * ( (prevY - 2.0*actual + postY)/problemConfiguration->dx2 + (prevX - 2.0*actual + postX)/problemConfiguration->dy2 );
}

float heatFormula(struct ProblemConfiguration* problemConfiguration, float actual, float prevY, float postY, float prevX, float postX){
    return actual + a * problemConfiguration->dt * ( (prevY - 2.0*actual + postY)/problemConfiguration->dx2 + (prevX - 2.0*actual + postX)/problemConfiguration->dy2 );
}

void saveStatusPng(float* arr, int stepNum){
    char filename[64];
    sprintf(filename, "temp/heat_%05d.png", stepNum);
    save_png(arr, nx, ny, filename, 'c');
}

void initializeArr(float* arr){
    printf("\x1b[32mInicializando array\x1b[0m\n");
    float radius2 = (nx/6.0) * (nx/6.0);
    for (int i = 0; i < nx; i++)
    {
        for (int j = 1; j < ny-1; j++)
        {
            int index = getIndex(i, j);
            // Distance of point i, j from the origin
            float ds2 = (i - nx/2) * (i - nx/2) + (j - 5*ny/6)*(j - 5*ny/6); // El centro del disco está desplazado
            //float ds2 = (i - nx/2) * (i - nx/2) + (j - ny/2)*(j - ny/2); // El centro del disco está centrado
            if (ds2 < radius2)
            {
                arr[index] = empty; //el centro del disco tiene un valor negativo para diferenciarlo
            }
            else
            {
                arr[index] = REGULAR_TEMP;
            }
        }
        {
            int index = getIndex(i, 0);
            arr[index] = MIN_TEMP;
        }
        {
            int index = getIndex(i, nx-1);
            arr[index] = MAX_TEMP;
        }
    }
}