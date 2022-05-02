#include "compose.h"

int MergeImages(SDL_Window *mainWindow, char *backgroungImagePath, char *forgroundImagePath)
{	
	SDL_Surface *backgroungImage = SDL_LoadBMP(backgroungImagePath);
	SDL_Surface *forgroundImage  = SDL_LoadBMP(forgroundImagePath);
	assert(backgroungImage && forgroundImage);

	SDL_Surface *outputImage = SDL_CreateRGBSurface(0, WIDTH, HEIGHT, 32, 0, 0, 0, 0);
	assert(outputImage);

    SDL_Renderer *windowRenderer = SDL_CreateRenderer(mainWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    assert(windowRenderer);

	SDL_Event event = {};
	time_t timeSpend = 0;
    size_t frames    = 0;

	for (bool running = true; running; SDL_PollEvent(&event), frames++)
	{
		if (event.type == SDL_QUIT ||
            event.key.keysym.sym == SDLK_ESCAPE) { running = false; }

		time_t startTime = clock();
	    SDL_LockSurface(outputImage);

		for (int curY = 0; curY < HEIGHT; curY++)
		{
			if (event.type == SDL_QUIT || event.key.keysym.sym == SDLK_ESCAPE)
			{
				running = false;
			}

			for (int curX = 0; curX < WIDTH; curX += 4)
			{
				CalculateNewImage((uint32_t *)backgroungImage->pixels + WIDTH * curY + curX, 
								  (uint32_t *) forgroundImage->pixels + WIDTH * curY + curX, 
								  (uint32_t *)    outputImage->pixels + WIDTH * curY + curX);
			}
		}

        SDL_UnlockSurface(outputImage);
        UpdateScreen(outputImage, windowRenderer);
        timeSpend += clock() - startTime;

        if (timeSpend > CLOCKS_PER_SEC)
        {
            timeSpend = 0;
            fprintf(stderr, "\rFPS: %zd", frames);
            frames = 0;
        }
	}

	fprintf(stderr, "\r");
	return 0;
}

int UpdateScreen(SDL_Surface *surface, SDL_Renderer *renderer)
{
	SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    assert(texture);

    SDL_SetRenderTarget(renderer, texture);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    
    SDL_DestroyTexture(texture);

    return 0;
}

int CalculateNewImage(uint32_t *backgroungImage, uint32_t *forgroundImage, uint32_t *outputImage)
{	
	__m128i forgroundLow   =  _mm_load_si128((__m128i *)forgroundImage);
	__m128i backgroungLow  =  _mm_load_si128((__m128i *)backgroungImage);
	
	__m128i forgroundHigh  =  (__m128i)_mm_movehl_ps((__m128)ZERO_M128, (__m128)forgroundLow);
	__m128i backgroungHigh =  (__m128i)_mm_movehl_ps((__m128)ZERO_M128, (__m128)backgroungLow);

	forgroundLow   =  _mm_cvtepu8_epi16   (forgroundLow);
	forgroundHigh  =  _mm_cvtepu8_epi16   (forgroundHigh);

	backgroungLow  =  _mm_cvtepu8_epi16   (backgroungLow);
	backgroungHigh =  _mm_cvtepu8_epi16   (backgroungHigh);

	static const __m128i moveAlphaMask = _mm_set_epi8(0x80u, 0xEu, 0x80u, 0xEu, 0x80u, 0xEu, 0x80u, 0xEu,
	                                                  0x80u, 0x6u, 0x80u, 0x6u, 0x80u, 0x6u, 0x80u, 0x6u);
	__m128i alphaLow  =  _mm_shuffle_epi8 (forgroundLow,  moveAlphaMask);
	__m128i alphaHigh =  _mm_shuffle_epi8 (forgroundHigh, moveAlphaMask);

	forgroundLow      =  _mm_mullo_epi16  (forgroundLow,  alphaLow);
	forgroundHigh     =  _mm_mullo_epi16  (forgroundHigh, alphaHigh);

	backgroungLow     =  _mm_mullo_epi16  (backgroungLow,  _mm_sub_epi16(MAX_NUM_M128, alphaLow));
	backgroungHigh    =  _mm_mullo_epi16  (backgroungHigh, _mm_sub_epi16(MAX_NUM_M128, alphaHigh));

	__m128i sumLow    =  _mm_add_epi16    (forgroundLow,  backgroungLow);
	__m128i sumHigh   =  _mm_add_epi16    (forgroundHigh, backgroungHigh);

	static const __m128i moveSumMask = _mm_set_epi8(0x80u, 0x80u, 0x80u, 0x80u, 0x80u, 0x80u, 0x80u, 0x80u, 
	                                                0xFu,  0xDu,  0xBu,  0x9u,  0x7u,  0x5u,  0x3u,  0x1u);
	sumLow   =  _mm_shuffle_epi8(sumLow,  moveSumMask);
	sumHigh  =  _mm_shuffle_epi8(sumHigh, moveSumMask);

	__m128i color = (__m128i)_mm_movelh_ps((__m128)sumLow, (__m128)sumHigh);

	_mm_store_si128 ((__m128i *)outputImage, color);

	return 0;
}