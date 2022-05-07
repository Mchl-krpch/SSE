#include <stdio.h>
#include <SDL.h>
#include <stdint.h>
#include <windows.h>

#include <time.h>

#include "compose.h"

void execute(composePicturesFrame *frame)
{
	SDL_Event event = {};
	time_t timeSpend = 0;
	size_t frames    = 0;

	char strInfo[BUF_LEN] = "";
	char mode[BUF_LEN]    = "SSE optimization";
	bool isOptimizated    = true;

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
				picturesComposeSSE(frame->width, frame->height, frame, outputImage);
			}
		}
		else
		{
			for (int index = 0; index < 20; index++)
			{
				picturesComposeSlow(frame->width, frame->height, frame, outputImage);
			}
		}

		updateRender(frame, outputImage);
		updateFps(&timeSpend, &startTime, &frames, strInfo, mode);
	}
}

void crossPlatformMessage(const char *title, const char *msg)
{
	#ifdef _WIN32
	 // 4-th argument - the style of the icon, button.
	MessageBox(NULL, msg, title, 0);
	#else
	fprintf(stderr, msg);
	#endif
}

composePicturesFrame *frameCtor(const char *bgPtr, const char *fgPtr)
{
	composePicturesFrame *frame = (composePicturesFrame *)calloc(1, sizeof(composePicturesFrame));

	frame->isFullScreen = false;
	frame->isMinimized  = false;
	frame->isRunning    = true;

	frame->width  = 500;
	frame->height = 400;

	// Main objects in window
	frame->window = NULL;
	frame->event  = {0};

	frame->window = SDL_CreateWindow(
		"SDL window",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		frame->width, frame->height,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

	frame->renderer = SDL_CreateRenderer(
		frame->window, -1, 
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	// Image part
	//////////////////////////////////////
	frame->background = SDL_LoadBMP(bgPtr);
	if (frame->background == NULL)
	{
		printf("invalid background\n");
		return NULL;
	}

	frame->foreground = SDL_LoadBMP(fgPtr);
	if (frame->foreground == NULL)
	{
		printf("invalid foreground\n");
		return NULL;
	}

	return frame;
}

void picturesComposeSSE(const uint32_t width, const uint32_t height, composePicturesFrame *frame, SDL_Surface *outputImage)
{
	for (int y = 0; y < frame->height; y++)
	{
		for (int x = 0; x < frame->width; x += 4)
		{
			// picturesComposeSlow(width, height, frame);
			CalculateNewPixels((uint32_t *)frame->foreground->pixels + frame->width * y + x, 
							  (uint32_t *)frame->background->pixels + frame->width * y + x, 
							  (uint32_t *)      outputImage->pixels + frame->width * y + x);
		}
	}
}

void picturesComposeSlow(const uint32_t width, const uint32_t height, composePicturesFrame *frame, SDL_Surface *outputImage)
{
	for (int y = 0; y < frame->height; y++)
	{
		for (int x = 0; x < frame->width; x += 1)
		{
			 CalculateNewPixelsSlow((uint32_t *)frame->foreground->pixels + frame->width * y + x,
								  (uint32_t *)frame->background->pixels + frame->width * y + x,
								  (uint32_t *)      outputImage->pixels + frame->width * y + x);
		}
	}
}

int CalculateNewPixels(uint32_t *backgroungImage, uint32_t *forgroundImage, uint32_t *outputImage)
{
	__m128i forgroundLow  = _mm_load_si128((__m128i *)forgroundImage);
	__m128i backgroungLow = _mm_load_si128((__m128i *)backgroungImage);
	
	__m128i forgroundHigh  = (__m128i)_mm_movehl_ps((__m128)ZERO_M128, (__m128)forgroundLow);
	__m128i backgroungHigh = (__m128i)_mm_movehl_ps((__m128)ZERO_M128, (__m128)backgroungLow);

	forgroundLow  = _mm_cvtepu8_epi16(forgroundLow);
	forgroundHigh = _mm_cvtepu8_epi16(forgroundHigh);

	backgroungLow  = _mm_cvtepu8_epi16(backgroungLow);
	backgroungHigh = _mm_cvtepu8_epi16(backgroungHigh);

	static const __m128i moveAlphaMask = _mm_set_epi8(0x80, 14, 0x80, 14, 0x80, 14, 0x80, 14,
													  0x80,  6, 0x80,  6, 0x80,  6, 0x80,  6);

	__m128i alphaLow  = _mm_shuffle_epi8(forgroundLow,  moveAlphaMask);
	__m128i alphaHigh = _mm_shuffle_epi8(forgroundHigh, moveAlphaMask);

	forgroundLow  = _mm_mullo_epi16(forgroundLow,  alphaLow);
	forgroundHigh = _mm_mullo_epi16(forgroundHigh, alphaHigh);

	backgroungLow  = _mm_mullo_epi16(backgroungLow,  _mm_sub_epi16(MAX_NUM_M128, alphaLow));
	backgroungHigh = _mm_mullo_epi16(backgroungHigh, _mm_sub_epi16(MAX_NUM_M128, alphaHigh));

	__m128i sumLow  = _mm_add_epi16(forgroundLow,  backgroungLow);
	__m128i sumHigh = _mm_add_epi16(forgroundHigh, backgroungHigh);

	static const __m128i moveSumMask = _mm_set_epi8(0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
													 0xF,  0xD,  0xB,  0x9,  0x7,  0x5,  0x3,  0x1);

	sumLow  = _mm_shuffle_epi8(sumLow,  moveSumMask);
	sumHigh = _mm_shuffle_epi8(sumHigh, moveSumMask);

	__m128i color = (__m128i)_mm_movelh_ps((__m128)sumLow, (__m128)sumHigh);

	_mm_store_si128 ((__m128i *)outputImage, color);

	return 0;
}

int CalculateNewPixelsSlow(uint32_t *backgroungImage, uint32_t *forgroundImage, uint32_t *outputImage)
{
	int color = 0x00000000;

	int forgroundColor  = *forgroundImage;
	int backgroungColor = *backgroungImage;

	int forgroundHigh  = forgroundColor  & 0xFFFF0000;
	int backgroungHigh = backgroungColor & 0xFFFF0000;

	int forgroundLow  = forgroundColor  & 0x0000FFFF;
	int backgroungLow = backgroungColor & 0x0000FFFF;

	int fgAlphaChannel = (forgroundColor  >> 24) & 0x0FF;
	int bgAlphaChannel = (backgroungColor >> 24) & 0x0FF;

	forgroundLow  = ( ( (forgroundLow       ) & 0xFF) * fgAlphaChannel / 255)       |
					( ( (forgroundLow  >> 8 ) & 0xFF) * fgAlphaChannel / 255) << 8;

	forgroundHigh = ( ( (forgroundHigh >> 16) & 0xFF) * fgAlphaChannel / 255) << 16 |
					( ( (forgroundHigh >> 24) & 0xFF) * fgAlphaChannel / 255) << 24;

	backgroungLow  = ( ( (backgroungLow       ) & 0xFF) * (255 - fgAlphaChannel) / 255)        |
					 ( ( (backgroungLow  >> 8 ) & 0xFF) * (255 - fgAlphaChannel) / 255 ) << 8;

	backgroungHigh = ( ( (backgroungHigh >> 16) & 0xFF) * (255 - fgAlphaChannel) / 255 ) << 16 |
					 ( ( (backgroungHigh >> 24) & 0xFF) * (255 - fgAlphaChannel) / 255 ) << 24 +
					 0x0000002F; // << I add this value to understand that current mode is 'no optimization'

	int colorResult = (forgroundLow | forgroundHigh) | (backgroungLow | backgroungHigh);

	*outputImage = (uint32_t)colorResult;

	return 0;
}

void updateRender(composePicturesFrame *frame, SDL_Surface *outputImage)
{
	SDL_UnlockSurface(frame->background);

	SDL_Texture *texture = SDL_CreateTextureFromSurface(frame->renderer, outputImage);

	SDL_SetRenderTarget(frame->renderer, texture);
	SDL_RenderCopy(frame->renderer, texture, NULL, NULL);
	SDL_RenderPresent(frame->renderer);

	SDL_DestroyTexture(texture);
}

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