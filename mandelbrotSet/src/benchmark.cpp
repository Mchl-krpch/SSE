// File-tester.
// Checks speed of calculus of different functions.

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "setCalculationFunctions.h"

const int WIDTH  = 1920;
const int HEIGHT = 1080;
const int ATTEMPTS = 20;

int main()
{
	mandelbrot set = {};
	set.pixels = (int *)calloc(WIDTH * HEIGHT, sizeof(int));

	time_t start;

	// No optimization render.
	/////////////////////////////////////////////////////////
	
	start = clock() / CLOCKS_PER_SEC;

	for (int index = 0; index < ATTEMPTS; index++)
		renderSetNoOptimization(WIDTH, HEIGHT, &set);
	
	printf("%-25s%f\n", "TIME NO OPTIMIZATION:", ((float)clock() / CLOCKS_PER_SEC - start) / ATTEMPTS);

	// Render with SSE.
	/////////////////////////////////////////////////////////

	start = clock() / CLOCKS_PER_SEC;
	
	for (int index = 0; index < ATTEMPTS; index++)
		renderSetSSE(WIDTH, HEIGHT, &set);

	printf("%-25s%f\n", "TIME SSE:",             ((float)clock() / CLOCKS_PER_SEC - start) / ATTEMPTS);

	// Render with AVX.
	/////////////////////////////////////////////////////////

	start = clock() / CLOCKS_PER_SEC;
	for (int index = 0; index < ATTEMPTS; index++)
		renderSetAVX(WIDTH, HEIGHT, &set);

	printf("%-25s%f\n", "TIME AVX:",             ((float)clock() / CLOCKS_PER_SEC - start) / ATTEMPTS);

	// Render with AVX512.
	/////////////////////////////////////////////////////////

	if (IsAVX512InTouch())
	{	
		start = clock();
		renderSetAVX512f(WIDTH, HEIGHT, &set);
		printf("TIME AVX512: %f\n", ((float)clock() / CLOCKS_PER_SEC - start) / ATTEMPTS);

		// Render with AVX512F
		/////////////////////////////////////////////////////////

		start = clock() / CLOCKS_PER_SEC;

		for (int index = 0; index < ATTEMPTS; index++)
			renderSetAVX512f(WIDTH, HEIGHT, &set);

		printf("%-25s%f\n", "TIME AVX512:",      ((float)clock() / CLOCKS_PER_SEC - start) / ATTEMPTS);
	}

	else
	{
		printf("AVX512 is not supported\n");
	}

	return 0;
}