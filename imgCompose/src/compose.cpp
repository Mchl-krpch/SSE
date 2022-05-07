#include <stdio.h>
#include <SDL.h>
#include <stdint.h>

#include "compose.h"

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

int CalculateNewImage(uint32_t *backgroungImage, uint32_t *forgroundImage, uint32_t *outputImage)
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

int CalculateNewImageSlow(uint32_t *backgroungImage, uint32_t *forgroundImage, uint32_t *outputImage)
{
	int color = 0x00000000;
	// __m128i forgroundLow  = _mm_load_si128((__m128i *)forgroundImage);
	// __m128i backgroungLow = _mm_load_si128((__m128i *)backgroungImage);
	int forgroundColor  = *forgroundImage;
	int backgroungColor = *backgroungImage;

	// __m128i forgroundHigh  = (__m128i)_mm_movehl_ps((__m128)ZERO_M128, (__m128)forgroundLow);
	// __m128i backgroungHigh = (__m128i)_mm_movehl_ps((__m128)ZERO_M128, (__m128)backgroungLow);
	int forgroundHigh  = forgroundColor  & 0xFFFF0000;
	int backgroungHigh = backgroungColor & 0xFFFF0000;

	// forgroundLow  = _mm_cvtepu8_epi16(forgroundLow);
	// forgroundHigh = _mm_cvtepu8_epi16(forgroundHigh);
	int forgroundLow  = forgroundColor  & 0x0000FFFF;
	int backgroungLow = backgroungColor & 0x0000FFFF;

	int fgAlphaChannel = (forgroundColor  >> 24) & 0x0FF;
	int bgAlphaChannel = (backgroungColor >> 24) & 0x0FF;

	// __m128i alphaLow  = _mm_shuffle_epi8(forgroundLow,  moveAlphaMask);
	// __m128i alphaHigh = _mm_shuffle_epi8(forgroundHigh, moveAlphaMask);
	forgroundLow  *= fgAlphaChannel / 255;
	forgroundHigh *= fgAlphaChannel / 255;

	// forgroundLow  = _mm_mullo_epi16(forgroundLow,  alphaLow);
	// forgroundHigh = _mm_mullo_epi16(forgroundHigh, alphaHigh);
	// backgroungLow  = _mm_mullo_epi16(backgroungLow,  _mm_sub_epi16(MAX_NUM_M128, alphaLow));
	// backgroungHigh = _mm_mullo_epi16(backgroungHigh, _mm_sub_epi16(MAX_NUM_M128, alphaHigh));
	backgroungLow  *= ((255 - fgAlphaChannel) / 255);
	backgroungHigh *= ((255 - fgAlphaChannel) / 255);

	// __m128i sumLow  = _mm_add_epi16(forgroundLow,  backgroungLow);
	// __m128i sumHigh = _mm_add_epi16(forgroundHigh, backgroungHigh);

	// static const __m128i moveSumMask = _mm_set_epi8(0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 
	// 												 0xF,  0xD,  0xB,  0x9,  0x7,  0x5,  0x3,  0x1);
	// sumLow  = _mm_shuffle_epi8(sumLow,  moveSumMask);
	// sumHigh = _mm_shuffle_epi8(sumHigh, moveSumMask);
	// __m128i color = (__m128i)_mm_movelh_ps((__m128)sumLow, (__m128)sumHigh);
	int colorResult = (forgroundLow + forgroundHigh) + (backgroungLow + backgroungHigh);

	// _mm_store_si128 ((__m128i *)outputImage, color);
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