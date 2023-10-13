/*
 * Based on CSC materials from:
 * 
 * https://github.com/csc-training/openacc/tree/master/exercises/heat
 *
 */
//#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "pngwriter.h"

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

void main()
{
    const int nx = 200;   // Width of the area
    const int ny = 200;   // Height of the area

    const float a = 0.1;     // Diffusion constant

    const float dx = 0.01;   // Horizontal grid spacing 
    const float dy = 0.01;   // Vertical grid spacing

    const float dx2 = dx*dx;
    const float dy2 = dy*dy;

    const float dt = dx2 * dy2 / (2.0 * a * (dx2 + dy2)); // Largest stable time step
    const int numSteps = 5000;                             // Number of time steps
    const int outputEvery = 1000;                          // How frequently to write output image

    int numElements = nx*ny;

    // Allocate two sets of data for current and next timesteps
    float* Un   = (float*)calloc(numElements, sizeof(float));
    float* Unp1 = (float*)calloc(numElements, sizeof(float));
    float* tmp;

    // Initializing the data with a pattern of disk of radius of 1/6 of the width
    float radius2 = (nx/6.0) * (nx/6.0);
    for (int i = 0; i < nx; i++)
    {
        for (int j = 1; j < ny-1; j++)
        {
            int index = getIndex(i, j, ny);
            // Distance of point i, j from the origin
            float ds2 = (i - nx/2) * (i - nx/2) + (j - 5*ny/6)*(j - 5*ny/6); // El centro del disco está desplazado
            //float ds2 = (i - nx/2) * (i - nx/2) + (j - ny/2)*(j - ny/2); // El centro del disco está centrado
            if (ds2 < radius2)
            {
                Un[index] = -1.0; //el centro del disco tiene un valor negativo para diferenciarlo
            }
            else
            {
                Un[index] = 50.0;
            }
        }
        {
            int index = getIndex(i, 0, ny);
            Un[index] = 0.1;
        }
        {
            int index = getIndex(i, nx-1, ny);
            Un[index] = 100.0;
        }
    }

    // Fill in the data on the next step to ensure that the boundaries are identical.
    memcpy(Unp1, Un, numElements*sizeof(float));

    // Timing
    clock_t start = clock();

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
		        if (i>0)
                	uim1j = Un[getIndex(i-1, j, ny)];
                else
			        uim1j = -1.0; // no existe como el centro del disco
		
                if (uim1j<0)
			        uim1j=uij; //si no existe no hay tranferencia de calor por lo es como si vale igual que el punto a evaluar


                float uijm1 = Un[getIndex(i, j-1, ny)];
                if (uijm1<0)
			        uijm1=uij ;  //si no existe no hay tranferencia de calor por lo es como si vale igual que el punto a evaluar

		        float uip1j;
                if (i<nx-1)
	                uip1j = Un[getIndex(i+1, j, ny)];
                else
                    uip1j = -1.0; // no existe como el centro del disco

                if (uip1j<0)
			        uip1j=uij ;  //si no existe no hay tranferencia de calor por lo es como si vale igual que el punto a evaluar


                float uijp1 = Un[getIndex(i, j+1, ny)];
 		        if (uijp1<0)
			        uijp1=uij ;  //si no existe no hay tranferencia de calor por lo es como si vale igual que el punto a evaluar
                // Explicit scheme
                if (uij<0.0)
			        Unp1[index]=uij;  // si es un punto que no existe (centro del disco no hay que evaluar su calor)
		        else
                	Unp1[index] = uij + a * dt * ( (uim1j - 2.0*uij + uip1j)/dx2 + (uijm1 - 2.0*uij + uijp1)/dy2 );
            }
        }
        // Write the output if needed
        if (n % outputEvery == 0)
        {
            char filename[64];
            sprintf(filename, "temp/heat_%05d.png", n);
            save_png(Un, nx, ny, filename, 'c');
        }
        // Swapping the pointers for the next timestep
        //std::swap(Un, Unp1);
	tmp=Un;
	Un=Unp1;
	Unp1=tmp;
    }

    // Timing
    clock_t finish = clock();
    printf("It took %f seconds\n", (double)(finish - start) / CLOCKS_PER_SEC);

    // Release the memory
    free(Un);
    free(Unp1);
    printf( "Memory freed\n" );
    
    return ;
}