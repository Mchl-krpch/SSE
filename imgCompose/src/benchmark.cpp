#include <stdio.h>
#include <time.h>
#include <stdint.h>

#include "compose.h"

static const int ATTEMPTS =  10;
static const int width    = 500;
static const int height   = 400;

#undef main

int main()
{
	uint32_t *foreground = (uint32_t *)calloc(width * height, sizeof(uint32_t));
	uint32_t *background = (uint32_t *)calloc(width * height, sizeof(uint32_t));
	uint32_t *output     = (uint32_t *)calloc(width * height, sizeof(uint32_t));

	time_t startTime = 0;

	startTime = clock();
	for (int index = 0; index < ATTEMPTS; index++)
	{
		for (int index = 0; index < 20; index++)
		{
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x += 4)
				{
					CalculateNewPixels(
						foreground + width * y + x, 
						background + width * y + x, 
						output     + width * y + x);
				}
			}
		}
	}

	printf("%20s: time spend approximation: %.3lf\n", "SSE-OPTIMIZATION",
		((double)(clock()) - startTime) / (ATTEMPTS * CLOCKS_PER_SEC) );

	startTime = clock();
	for (int index = 0; index < ATTEMPTS; index++)
	{
		for (int index = 0; index < 20; index++)
		{
			for (int y = 0; y < height; y++)
			{
				for (int x = 0; x < width; x += 1)
				{
					CalculateNewPixelsSlow(
						foreground + width * y + x, 
						background + width * y + x, 
						output     + width * y + x);
				}
			}
		}
	}

	printf("%20s: time spend approximation: %.3lf\n", "NO-OPTIMIZATION",
		((double)(clock()) - startTime) / (ATTEMPTS * CLOCKS_PER_SEC) );

	free(foreground);
	free(background);
	free(output);

	return 0;
}