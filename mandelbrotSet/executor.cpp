// My module
#ifdef _WIN32
#include <windows.h>
#endif

#include <time.h>

#include "winXd.hpp"
#include "mandelbrot.hpp"

const int WIDTH  = 1920;
const int HEIGHT = 1080;

const int ATTEMPTS = 20;

int main()
{
	// winXd frame;
	// frame.create("Mandelbrot set");

	mandelbrot set = {};
	set.pixels = (unsigned int *)calloc(WIDTH * HEIGHT, sizeof(unsigned int));

	time_t start;


	start = clock() / CLOCKS_PER_SEC;
	for (int index = 0; index < ATTEMPTS; index++)
	{
		renderSetNoOptimization(WIDTH, HEIGHT, &set);
	}
	printf("%-25s%f\n", "TIME NO OPTIMIZATION:", ((float)clock() / CLOCKS_PER_SEC - start) / ATTEMPTS);


	start = clock() / CLOCKS_PER_SEC;
	for (int index = 0; index < ATTEMPTS; index++)
	{
		renderSetSSE(WIDTH, HEIGHT, &set);
	}
	printf("%-25s%f\n", "TIME SSE:",             ((float)clock() / CLOCKS_PER_SEC - start) / ATTEMPTS);


	start = clock() / CLOCKS_PER_SEC;
	for (int index = 0; index < ATTEMPTS; index++)
	{
		renderSetAVX(WIDTH, HEIGHT, &set);
	}
	printf("%-25s%f\n", "TIME AVX:",             ((float)clock() / CLOCKS_PER_SEC - start) / ATTEMPTS);

	// start = clock();
	// renderSetAVX512f(WIDTH, HEIGHT, &set);
	// printf("TIME AVX512: %ld\n", clock() - start);

	return 0;
}