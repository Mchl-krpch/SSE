#ifndef _compose_
#define _compose_

#include <SDL.h>
#include <stdint.h>

const int BUF_LEN = 255;

const __m128i ZERO_M128 =
	_mm_set_epi8(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

const __m128i MAX_NUM_M128 = _mm_cvtepu8_epi16(
	_mm_set_epi8(255, 255, 255, 255, 255, 255, 255, 255,
			     255, 255, 255, 255, 255, 255, 255, 255));

const char MSG_NOT_ENOUGH_PICTURES[] =
 "You are not starting the project correctly.\n"
 "try specifying the filenames in the make so that\n"
 "the program knows where to get the pictures from\n\n\n"
 "   Makefile:\n\n"
 "   IMAGES_TO_COMPOSE = \"...\"   \"...\"\n"
 "   ^~~~~~   This variable should contain two names";

const char MSG_BAD_RENDERER[] =
 "Unfortunately, the SDL library renderer was not created\n"
 "for some reason, try reinstalling the library";

const char GREETINGS[] =
 "Pictures compose program v0.4\n"
 "This is a study application to compare the speed of rendering\n"
 "frames per second with optimization based on SSE functions and\n"
 "normal picture rendering.\n\n";

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

void execute(composePicturesFrame *frame);

void crossPlatformMessage(const char *title, const char *msg);

composePicturesFrame *frameCtor(const char *bgPtr, const char *fgPtr);
composePicturesFrame *frameDtor(composePicturesFrame *frame);

void updateRender(composePicturesFrame *frame, SDL_Surface *outputImage);

void picturesComposeSlow(const uint32_t width, const uint32_t height, composePicturesFrame *frame, SDL_Surface *outputImage);
void picturesComposeSSE(const uint32_t width, const uint32_t height, composePicturesFrame *frame, SDL_Surface *outputImage);

int CalculateNewPixels    (uint32_t *backgroungImage, uint32_t *forgroundImage, uint32_t *outputImage);
int CalculateNewPixelsSlow(uint32_t *backgroungImage, uint32_t *forgroundImage, uint32_t *outputImage);

static void updateFps(time_t *timeSpend, time_t *startTime, size_t *frames, char *strInfo, const char *mode);

#endif