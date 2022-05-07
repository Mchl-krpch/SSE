// File with implementation of functions for calculating a picture in a set.

#include "winxdConfig.h"

static int getDotColor(double t)
{
	return (int)(0x10000 * (9   * (int)(  ((double)1 - t) *              t  *              t  * t * 255)))
		 + (int)(0x00100 * (15  * (int)(  ((double)1 - t) * ((double)1 - t) *              t  * t * 255)))
		 + (int)(0x00001 * (8.5 * (int)(  ((double)1 - t) * ((double)1 - t) * ((double)1 - t) * t * 255)));
}

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

				double t = (double)n / (double)(set->MAX_CHECK);

				for (int i_cmp = 0; i_cmp < 16; i_cmp++)
				{
					if (1 & (cmp >> (i_cmp)))
					{
						set->pixels[pixels_pos + ((int64_t)15 - i_cmp)] += getDotColor(t);
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

				double t = (double)n / (double)(set->MAX_CHECK);

				for (int i_cmp = 0; i_cmp < 8; i_cmp++)
				{
					if (*((int *)&cmp + i_cmp))
					{
						set->pixels[pixels_pos + ((int64_t)7 - i_cmp)] += getDotColor(t);
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

				double t = (double)n / (double)(set->MAX_CHECK);

				for (int i_cmp = 0; i_cmp < 4; i_cmp++)
				{
					if (*((int *)&cmp + i_cmp))
					{
						set->pixels[pixels_pos + ((int64_t)3 - i_cmp)] += getDotColor(t);
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

				double t = (double)n / (double)(set->MAX_CHECK);

				if (cmp)
				{
					set->pixels[y * windowWidth + x] += getDotColor(t);
				}

				colored |= cmp;
			}

		}

	}

	return;
}