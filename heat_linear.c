/*
 * Based on CSC materials from:
 * 
 * https://github.com/csc-training/openacc/tree/master/exercises/heat
 *
 */
//#include <algorithm>
#define VACIO -1.00
#define nx 200
#define ny 200

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include <semaphore.h>

#include "pngwriter.h"

void Gen_matrix(float* matriz, int radius2, int center_x, int center_y);

/* Convert 2D index layout to unrolled 1D layout
 *
 * \param[in] i      Row index
 * \param[in] j      Column index
 * \param[in] width  The width of the area
 * 
 * \returns An index in the unrolled 1D array.
 */
int getIndex(const int i, const int j, const int width)
{
    return i*width + j;
}

    float* Un;
    float* Unp1;
    float* tmp;
    
    const float a = 0.1;     // Diffusion constant

    const float dx = 0.01;   // Horizontal grid spacing 
    const float dy = 0.01;   // Vertical grid spacing

    const float dx2 = dx*dx;
    const float dy2 = dy*dy;

    const float dt = dx2 * dy2 / (2.0 * a * (dx2 + dy2)); // Largest stable time step
    const int numSteps = 5000;                             // Number of time steps
    const int outputEvery = 1000;                          // How frequently to write output image

    int numElements = nx*ny;
  long n_threads=4;  
void main()
{
    long       thread;
    pthread_t* thread_handles;
    



    // Allocate two sets of data for current and next timesteps
    Un   = (float*)calloc(numElements, sizeof(float));
    Unp1 = (float*)calloc(numElements, sizeof(float));
    thread_handles = malloc(n_threads*sizeof(pthread_t));

    float radius2 = (nx/6.0) * (nx/6.0);
    Gen_matrix(Un,radius2, nx/2, 5*ny/6);

    // Fill in the data on the next step to ensure that the boundaries are identical.
    memcpy(Unp1, Un, numElements*sizeof(float));

    // Timing
    struct timespec start, finish;
    double elapsed;
    //clock_t start = clock();
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Main loop
    for (int n = 0; n <= numSteps; n++)
    {
        // Going through the entire area
        for (int i = 0; i < nx; i++)
        {
            for (int j = 1; j < ny-1; j++)
            {
                const int index = getIndex(i, j, ny);
                float uij = Un[index];
		            float uim1j;
	              if (i>0) // No est� en la primera fila y entonces existe la fila superior
                	uim1j = Un[getIndex(i-1, j, ny)];
                else
                  uim1j = uij; // Si estaba en la primera fila no hay nada arriba y ponemos su mismo valor para que no haya transferencia de calor
                if (uim1j==VACIO) // Si el hueco estaba vacio ponemos su mismo valor para que no haya transferencia de calor
                  uim1j=uij;


                float uijm1 = Un[getIndex(i, j-1, ny)];  // Nunca estamos en la primera columna porque el bucle empieza en j=1
                if (uijm1==VACIO)
                  uijm1=uij ; // Si el hueco estaba vacio ponemos su mismo valor para que no haya transferencia de calor

                float uip1j;
                if (i<nx-1) // No est� en la �ltima fila y entonces existe la fila inferior
	                uip1j = Un[getIndex(i+1, j, ny)];
                else
                  uip1j = uij; // Si estaba en la �ltima fila no hay nada abajo y ponemos su mismo valor para que no haya transferencia de calor

                if (uip1j==VACIO) // Si el hueco estaba vacio ponemos su mismo valor para que no haya transferencia de calor
                  uip1j=uij ;
                
                float uijp1 = Un[getIndex(i, j+1, ny)]; // Nunca estamos en la �ltima columna porque el bucle termina en ny-1
                if (uijp1==VACIO)
                  uijp1=uij ; // Si el hueco estaba vacio ponemos su mismo valor para que no haya transferencia de calor
                
                // Explicit scheme
                if (uij==VACIO)  // Si era un hueco lo dejamos hueco
                  Unp1[index]=uij;
                else   // Si no aplicamos la formula de transferencia de calor
                	Unp1[index] = uij + a * dt * ( (uim1j - 2.0*uij + uip1j)/dx2 + (uijm1 - 2.0*uij + uijp1)/dy2 );
            }
        }
        // Write the output if needed
        //if (n % outputEvery == 0)
        //{
        //    char filename[64];
        //    sprintf(filename, "heat_%05d.png", n);
        //    save_png(Un, nx, ny, filename, 'c');
        //}
        // Swapping the pointers for the next timestep
        //std::swap(Un, Unp1);
	tmp=Un;
	Un=Unp1;
	Unp1=tmp;
    }

    // Timing
    //clock_t finish = clock();
    clock_gettime(CLOCK_MONOTONIC, &finish);
    elapsed = (finish.tv_sec - start.tv_sec);
elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("It took %f seconds\n", (double)elapsed );
    //printf("It took %f seconds\n", (double)(finish - start) / CLOCKS_PER_SEC);

    // Release the memory
    free(Un);
    free(Unp1);
    printf( "Memory freed\n" );
    
    return ;
}

/*------------------------------------------------------------------
 * Function: Gen_matrix
 * Purpose:  genera un hueco circular en el punto y radio dados e inicializa el resto de valores
 *    the entries in A
 * In args:  m, n
 * Out arg:  A
 */
void Gen_matrix(float* matriz, int radius2, int center_x, int center_y) {
// Initializing the data with a pattern of disk of radius of 1/6 of the width
     for (int i = 0; i < nx; i++)
    {
        for (int j = 1; j < ny-1; j++)
        {
            int index = getIndex(i, j, ny);
            // Distance of point i, j from the origin
            float ds2 = (i - center_x) * (i -center_x) + (j - center_y)*(j - center_y);
            if (ds2 < radius2)
            {
                matriz[index] = VACIO;
            }
            else
            {
                matriz[index] = 50.0;
            }
        }
        {
            int index = getIndex(i, 0, ny);
            matriz[index] = 0.0;
        }
        {
            int index = getIndex(i, nx-1, ny);
            matriz[index] = 100.0;
        }
    }
}  /* Gen_matrix */
