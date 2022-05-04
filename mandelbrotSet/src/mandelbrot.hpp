#ifndef _mandelbrot_
#define _mandelbrot_

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <immintrin.h>
#include <emmintrin.h>
// For __cpuid
#ifdef _WIN32
#include <intrin.h>
#endif
#include <SFML/Graphics.hpp>

// FOR AVX 256
const __m256 FULL_COLORED = _mm256_cmp_ps(_mm256_set1_ps(0), _mm256_set1_ps(0), _CMP_EQ_OQ);
const __m256 MUL_2        = _mm256_set1_ps(2.0);
const __m256 R_NEED       = _mm256_set1_ps(100);

// FOR SSE 128
const __m128 MUL = _mm_set1_ps(2.0);
const __m128 FULL_COLORED_128 = _mm_cmp_ps(_mm_set1_ps(0), _mm_set1_ps(0), _CMP_EQ_OQ);

// FOR NO OPTIMIZATION
int NORM_FULL_COLORED = (0 == 0);
int NORM_R_NEED       = 100;
const __m128 R_NEED_128 = _mm_set1_ps(100);

typedef struct
{
	sf::Text modeString;
	sf::Text checkInfo;
	sf::Text fpsString;

	unsigned *pixels = NULL;
	
	float scale   = 0.23;
	float x_shift = -0.3;
	float y_shift =    0;

	uint32_t MAX_CHECK = 32;
	
	const int      mouseMovementSensivity  = 25000;
	const uint32_t BLACK_COLOR_PIXEL       = 0xFF000000;
	const float    MOUSE_WHEEL_SENSITIVITY = 0.05;

	const float  R2 = 20;

	void setModeString(sf::Font& font, const int height, int mode)
	{
		char string[256] = "";
		strcpy(string, "render mode:");

		if (mode == 0)
		{
			modeString.setFillColor(sf::Color(228, 84, 64, 200));
			strcat(string, "no optimization");
		}

		if (mode == 1)
		{
			modeString.setFillColor(sf::Color(228, 212, 64, 200));
			strcat(string, "sse");
		}

		if (mode == 2)
		{
			modeString.setFillColor(sf::Color(64, 228, 130, 200));
			strcat(string, "avx");
		}

		if (mode == 3)
		{
			modeString.setFillColor(sf::Color(228, 64, 182, 200));
			strcat(string, "avx512");
		}

		modeString.setString(string);
		modeString.setCharacterSize(14);

		modeString.setFont(font);
		modeString.setPosition(10, height);
	}

	void setCheckText(sf::Font& font, const int height)
	{
		checkInfo.setFillColor(sf::Color(255, 255, 255, 200));

		char string[256] = "";
		char num[5] = "";
		strcpy(string, "Detail level:");
		sprintf(num, "%u", MAX_CHECK);
		strcat(string, num);
		checkInfo.setString(string);

		checkInfo.setFont(font);
		checkInfo.setPosition(10, height);
	}

	// FPS part.
	sf::Clock  clock = {};
	sf::Time   time  = clock.getElapsedTime();

	float time_prev     = time.asSeconds();
	float time_now      = 0;
	float time_last_out = 0;

	const float FPS_DELAY = 0.15;
	char str[32] = "fps:";

	void renew(sf::Text& fpsLabel)
	{
		time     = clock.getElapsedTime  ();
		time_now = time.asSeconds        ();

		if (time_now - time_last_out > FPS_DELAY)
		{
			float cur_fps = (float)(1 / (time_now - time_prev));

			snprintf(str + 4, 26, "%.2lf\n", (float)(1 / (time_now - time_prev)));
			fpsLabel.setString(str);

			time_last_out = time_now;
		}

		time_prev = time_now;
	}

} mandelbrot;

typedef struct
{
	float rel_x_coef = 0;
	float rel_y_coef = 0;

	void getCoordinates(sf::RenderWindow& window, mandelbrot *set, const int width, const int height)
	{
		sf::Vector2f position;

		position.x = sf::Mouse::getPosition().x - window.getPosition().x - (width / 2);
		position.y = sf::Mouse::getPosition().y - window.getPosition().y - (height / 2);

		rel_x_coef = ((float)(-1 * (float)position.x) / set->mouseMovementSensivity);
		rel_y_coef = ((float)(-1 * (float)position.y) / set->mouseMovementSensivity);
	}

} coordinates;

#ifdef _WIN32
bool IsAVX512InTouch()
{
	// if num liaves < 7, then avx512 is not availavle.
	int cpuInfo[4] = {};
	__cpuid(cpuInfo, 0);

	int numLeaves = cpuInfo[0];
	if (numLeaves >= 7)
	{
		__cpuidex(cpuInfo, 7, 0);

		// Check avx512
		return ((cpuInfo[1]) >> 16) & 0x01;
	}

	return false;
}
#else
bool IsAVX512InTouch()
{
	return true;
}
#endif

void renderSetAVX512f(const int windowWidth, const int windowHeight, mandelbrot *set)
{
	// FOR AVX 512
	const uint16_t FULL_COLORED512 = 0xFFFF;
	const __m512 MUL_512   = _mm512_set1_ps(2.0);
	const __m512 R_NEED512 = _mm512_set1_ps(100);

	int pixels_pos = 0;

	for (int y = 0; y < windowHeight; y++)
	{
		float Im_num = ((float)y / windowWidth - 0.5 * windowHeight / windowWidth) / set->scale + set->y_shift;

		for (int x = 0; x < windowWidth; x += 16)
		{
			__m512 Re = _mm512_set_ps(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

			Re = _mm512_add_ps(Re, _mm512_set1_ps((float)x    ));
			Re = _mm512_div_ps(Re, _mm512_set1_ps(windowWidth ));
			Re = _mm512_sub_ps(Re, _mm512_set1_ps(0.5         ));
			Re = _mm512_div_ps(Re, _mm512_set1_ps(set->scale  ));
			Re = _mm512_add_ps(Re, _mm512_set1_ps(set->x_shift));

			__m512 Re0 = Re;

			// sse move
			__m512 Im = _mm512_set1_ps(Im_num);
			__m512 Im0 = Im;

			uint16_t colored = 0;

			// black screen
			for (int i_pixel = 0; i_pixel < 16; i_pixel++)
				*(set->pixels + pixels_pos + i_pixel) = set->BLACK_COLOR_PIXEL;

			__m512 Re_2 = _mm512_mul_ps(Re, Re);
			__m512 Im_2 = _mm512_mul_ps(Im, Im);

			for (size_t n = 0; (n < set->MAX_CHECK) && colored != FULL_COLORED512; n++) {

				Im = _mm512_fmadd_ps(MUL_512, _mm512_mul_ps(Re, Im), Im0);
				Re = _mm512_add_ps(_mm512_sub_ps(Re_2, Im_2), Re0);

				Re_2 = _mm512_mul_ps(Re, Re);
				Im_2 = _mm512_mul_ps(Im, Im);

				uint16_t cmp = (uint16_t)_mm512_cmp_ps_mask(_mm512_add_ps(Re_2, Im_2), R_NEED512, _CMP_GT_OQ);


				cmp = (~colored) & cmp;

				for (int i_cmp = 0; i_cmp < 16; i_cmp++)
				{
					if (1 & (cmp >> (i_cmp)))
					{
						set->pixels[pixels_pos + ((int64_t)15 - i_cmp)] =
							(uint32_t)(set->BLACK_COLOR_PIXEL + 0x00FF0000 - (0x0010000 * (n)+01000000 * 5 * n));
					}
				}

				colored |= cmp;
			}

			pixels_pos += 16;
		}

	}

	return;
}


void renderSetAVX(const int windowWidth, const int windowHeight, mandelbrot *set)
{
	int pixels_pos = 0;

	for (int y = 0; y < windowHeight; y++) {

		float Im_num = ((float)y - 0.5 * windowHeight) / (windowWidth * set->scale) + set->y_shift;

		for (int x = 0; x < windowWidth; x += 8)
		{
			__m256 Re = _mm256_set_ps(0, 1, 2, 3, 4, 5, 6, 7);

			Re = _mm256_add_ps(Re, _mm256_set1_ps((float)x   ));
			Re = _mm256_div_ps(Re, _mm256_set1_ps(windowWidth));
			Re = _mm256_sub_ps(Re, _mm256_set1_ps(0.5        ));
			Re = _mm256_div_ps(Re, _mm256_set1_ps(set->scale      ));
			Re = _mm256_add_ps(Re, _mm256_set1_ps(set->x_shift    ));

			__m256 Re0 = Re;

			// sse move
			__m256 Im = _mm256_set1_ps(Im_num);
			__m256 Im0 = Im;

			__m256 colored = _mm256_set1_ps(0);

			// black screen
			for (int i_pixel = 0; i_pixel < 8; i_pixel++)
				*(set->pixels + pixels_pos + i_pixel) = set->BLACK_COLOR_PIXEL;

			__m256 Re_2 = _mm256_mul_ps(Re, Re);
			__m256 Im_2 = _mm256_mul_ps(Im, Im);

			for (int n = 0; n < set->MAX_CHECK && !_mm256_testc_ps(colored, FULL_COLORED); n++)
			{
				Im = _mm256_fmadd_ps(MUL_2, _mm256_mul_ps(Re, Im), Im0);
				Re = _mm256_add_ps(_mm256_sub_ps(Re_2, Im_2), Re0);

				Re_2 = _mm256_mul_ps(Re, Re);
				Im_2 = _mm256_mul_ps(Im, Im);

				__m256 cmp = _mm256_cmp_ps(_mm256_add_ps(Re_2, Im_2), R_NEED, _CMP_GT_OQ);

				cmp = _mm256_andnot_ps(colored, cmp);

				for (int i_cmp = 0; i_cmp < 8; i_cmp++)
				{
					if (*((int *)&cmp + i_cmp))
					{
						set->pixels[pixels_pos + ((int64_t)7 - i_cmp)] =
							(uint32_t)(set->BLACK_COLOR_PIXEL + 0x00FF0000 - (0x0010000 * (n)+01000000 * 5 * n));
					}
				}

				colored = _mm256_or_ps(colored, cmp);
			}

			pixels_pos += 8;
		}

	}

	return;
}

void renderSetSSE(const int windowWidth, const int windowHeight, mandelbrot *set)
{
	int pixels_pos = 0;

	for (int y = 0; y < windowHeight; y++) {

		float Im_num = ((float)y - 0.5 * windowHeight) / (windowWidth * set->scale) + set->y_shift;

		for (int x = 0; x < windowWidth; x += 4)
		{
			__m128 Re = _mm_set_ps(0, 1, 2, 3);

			Re = _mm_add_ps(Re, _mm_set1_ps((float)x   ));
			Re = _mm_div_ps(Re, _mm_set1_ps(windowWidth));
			Re = _mm_sub_ps(Re, _mm_set1_ps(0.5        ));
			Re = _mm_div_ps(Re, _mm_set1_ps(set->scale      ));
			Re = _mm_add_ps(Re, _mm_set1_ps(set->x_shift    ));

			__m128 Re0 = Re;

			// sse move
			__m128 Im = _mm_set1_ps(Im_num);
			__m128 Im0 = Im;

			__m128 colored = _mm_set1_ps(0);

			// black screen
			for (int i_pixel = 0; i_pixel < 8; i_pixel++)
				*(set->pixels + pixels_pos + i_pixel) = set->BLACK_COLOR_PIXEL;

			__m128 Re_2 = _mm_mul_ps(Re, Re);
			__m128 Im_2 = _mm_mul_ps(Im, Im);

			for (int n = 0; n < set->MAX_CHECK && !_mm_testc_ps(colored, FULL_COLORED_128); n++)
			{
				Im = _mm_fmadd_ps(MUL, _mm_mul_ps(Re, Im), Im0);
				Re = _mm_add_ps(_mm_sub_ps(Re_2, Im_2), Re0);

				Re_2 = _mm_mul_ps(Re, Re);
				Im_2 = _mm_mul_ps(Im, Im);

				__m128 cmp = _mm_cmp_ps(_mm_add_ps(Re_2, Im_2), R_NEED_128, _CMP_GT_OQ);

				cmp = _mm_andnot_ps(colored, cmp);

				for (int i_cmp = 0; i_cmp < 4; i_cmp++) {
					if (*((int *)&cmp + i_cmp)) {
						set->pixels[pixels_pos + ((int64_t)3 - i_cmp)] =
							(uint32_t)(set->BLACK_COLOR_PIXEL + 0x00FF0000 - (0x0010000 * (n)+01000000 * 5 * n));
					}
				}

				colored = _mm_or_ps(colored, cmp);
			}

			pixels_pos += 4;
		}

	}

	return;
}

void renderSetNoOptimization(const int windowWidth, const int windowHeight, mandelbrot *set)
{
	for (int y = 0; y < windowHeight; y++)
	{
		float Im_num = ((float)y / windowWidth - 0.5 * windowHeight / windowWidth) / set->scale + set->y_shift;

		for (int x = 0; x < windowWidth; x += 1)
		{

			float Re = ((float)x / windowWidth - 0.5) / set->scale + set->x_shift;

			float Re0 = Re;

			// sse move
			float Im  = Im_num;
			float Im0 = Im;

			int colored = 0;

			*(set->pixels + x + y * windowWidth) = set->BLACK_COLOR_PIXEL;

			float Re_2 = Re * Re;
			float Im_2 = Im * Im;

			for (int n = 0; n < set->MAX_CHECK && ((colored & NORM_FULL_COLORED) == 0); n++)
			{
				Im = 2.0f * Re * Im + Im0;

				Re = Re_2 - Im_2 + Re0;

				Re_2 = Re * Re;

				Im_2 = Im * Im;
				bool cmp = (Re_2 + Im_2) > 100;

				cmp = ~(colored >= 1) & cmp;

				if (cmp)
				{
					set->pixels[y * windowWidth + x] = (uint32_t)(set->BLACK_COLOR_PIXEL + 0x00FF0000 - (0x0010000 * (n)+01000000 * 5 * n));
				}

				colored |= cmp;
			}

		}

	}

	return;
}

#endif