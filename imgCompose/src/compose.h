#ifndef _compose_
#define _compose_

#include <SDL.h>
#include <stdint.h>

const __m128i ZERO_M128 = _mm_set_epi8( 0, 0, 0, 0, 0, 0, 0, 0,
										0, 0, 0, 0, 0, 0, 0, 0);

const __m128i MAX_NUM_M128 = _mm_cvtepu8_epi16(
						  _mm_set_epi8(255u, 255u, 255u, 255u, 255u, 255u, 255u, 255u,
									   255u, 255u, 255u, 255u, 255u, 255u, 255u, 255u));

const uint32_t TRANSPARENT_PIXEL = 0x00FFFFFF;

typedef struct 
{
	// Process control variables
	/////////////////////////////////
	bool isFullScreen = false;
	bool isMinimized  = false;
	bool isRunning    = true; 

	short width       =  500;
	short height      =  400;

	// Main objects in window
	//////////////////////////////////
	// Render assets
	SDL_Surface   *surface     =  NULL;
	SDL_Renderer  *renderer    =  NULL;
	// Window assets
	SDL_Window    *window      =  NULL;
	SDL_Event      event       =   {0};

	// Pictures.
	SDL_Surface   *foreground  =  NULL;
	SDL_Surface   *background  =  NULL;

} composePicturesFrame;

composePicturesFrame *frameCtor(const char *bgPtr, const char *fgPtr);
void updateRender(composePicturesFrame *frame, SDL_Surface *outputImage);

int CalculateNewImage    (uint32_t *backgroungImage, uint32_t *forgroundImage, uint32_t *outputImage);
int CalculateNewImageSlow(uint32_t *backgroungImage, uint32_t *forgroundImage, uint32_t *outputImage);

#endif