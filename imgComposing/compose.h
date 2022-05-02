#ifndef IMAGE_MERGE_H
#define IMAGE_MERGE_H

#include <SDL.h>

#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>
#include <emmintrin.h>
#include <nmmintrin.h>
#include <smmintrin.h>

typedef uint32_t RGBA;

// SDL consts
const int WIDTH  = 800;
const int HEIGHT = 600;

// SSE consts
const __m128i ZERO_M128    =                   _mm_set_epi8(0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0);
const __m128i MAX_NUM_M128 = _mm_cvtepu8_epi16(_mm_set_epi8(255u, 255u, 255u, 255u, 255u, 255u, 255u, 255u, 255u, 255u, 255u, 255u, 255u, 255u, 255u, 255u));

int MergeImages(SDL_Window *mainWindow, char *backgroungImagePath, char *forgroundImagePath);

int UpdateScreen(SDL_Surface *surface, SDL_Renderer *renderer);

int CalculateNewImage(uint32_t *backgroungImage, uint32_t *forgroundImage, uint32_t *outputImage);

#endif // IMAGE_MERGE_H