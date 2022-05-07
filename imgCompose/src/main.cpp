#include <SDL.h>
#undef main

#include <stdio.h>
#include <windows.h>
#include <time.h>

#include "compose.h"

static void updateFps(time_t *timeSpend, time_t *startTime, size_t *frames, char *strInfo, const char *mode)
{
	*timeSpend += clock() - *startTime;
	sprintf(strInfo, "fps:%zd mode:%s", *frames, mode);

	if ((*timeSpend) > CLOCKS_PER_SEC)
	{
		*timeSpend = 0;
		printf("\r%s", strInfo);
		*frames = 0;
	}
}

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		MessageBox(NULL,
			"You are not starting the project correctly.\n"
			"try specifying the filenames in the make so that\n"
			"the program knows where to get the pictures from\n\n\n"
			"   Makefile:\n\n"
			"   IMAGES_TO_COMPOSE = \"...\"   \"...\"\n"
			"   ^~~~~~   This variable should contain two names",

			"undefined path to images",

			0);

		return 1;
	}

	composePicturesFrame *frame = frameCtor(argv[1], argv[2]);

	SDL_Event event = {};
	time_t timeSpend = 0;
	size_t frames    = 0;

	if (frame->renderer == NULL)
	{
		MessageBox(NULL,
			"You are not starting the project correctly.\n"
			"try specifying the filenames in the make so that\n"
			"the program knows where to get the pictures from\n\n\n"
			"   Makefile:\n\n"
			"   IMAGES_TO_COMPOSE = \"...\"   \"...\"\n"
			"   ^~~~~~   This variable should contain two names",

			"undefined path to images",

			0);
		return 1;
	}

	system("cls");
	printf("Pictures compose program v0.4\n"
		"This is a study application to compare the speed of rendering\n"
		"frames per second with optimization based on SSE functions and\n"
		"normal picture rendering.\n\n");

	char strInfo[200] = "";
	char mode[200] = "SSE optimization";
	bool isOptimizated = true;

	time_t startTime = clock();

	while (frame->isRunning)
	{
		frames++;

		SDL_PollEvent(&frame->event);

		switch (frame->event.type)
		{
			case (SDL_QUIT):
			{
				frame->isRunning = false;
				break;
			}

			case (SDL_KEYDOWN):
			{
				const Uint8* state = SDL_GetKeyboardState(NULL);

				if (state[SDL_SCANCODE_T])
				{
					if (isOptimizated)
					{
						isOptimizated = false;
						strcpy(mode, "No optimization ");

						sprintf(strInfo, "fps:%zd mode:%s", frames, mode);
						printf("\r%s", strInfo);

					}
					else
					{
						isOptimizated = true;
						strcpy(mode, "SSE optimization");

						sprintf(strInfo, "fps:%zd mode:%s", frames, mode);
						printf("\r%s", strInfo);
					}
				}
			}

			default:
			{
				break;
			}
		}

		startTime = clock();

		SDL_Surface *outputImage = SDL_CreateRGBSurface(0, frame->width, frame->height, 32, 0, 0, 0, 0);

		if (isOptimizated)
		{
			for (int index = 0; index < 20; index++)
			{
				for (int y = 0; y < frame->height; y++)
				{
					for (int x = 0; x < frame->width; x += 4)
					{
						CalculateNewImage((uint32_t *)frame->foreground->pixels + frame->width * y + x, 
										  (uint32_t *)frame->background->pixels + frame->width * y + x, 
										  (uint32_t *)      outputImage->pixels + frame->width * y + x);
					}
				}
			}
		}
		else
		{
			for (int index = 0; index < 20; index++)
			{
				for (int y = 0; y < frame->height; y++)
				{
					for (int x = 0; x < frame->width; x += 1)
					{
						CalculateNewImageSlow((uint32_t *)frame->foreground->pixels + frame->width * y + x,
											  (uint32_t *)frame->background->pixels + frame->width * y + x,
											  (uint32_t *)      outputImage->pixels + frame->width * y + x);
					}
				}
			}
		}

		updateRender(frame, outputImage);
		updateFps(&timeSpend, &startTime, &frames, strInfo, mode);
	}

	return 0;
}