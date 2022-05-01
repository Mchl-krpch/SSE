#include <stdio.h>
#include <stdint.h>
#include <immintrin.h>
#include <intrin.h>
#include <SFML/Graphics.hpp>
#include <windows.h>

const int WIDTH   = 1280;
const int HEIGHT  =  720;
const int BUF_LEN =  256;
const int STD_FONT_SIZE = 20;

const __m256 FULL_COLORED = _mm256_cmp_ps(_mm256_set1_ps(0), _mm256_set1_ps(0), _CMP_EQ_OQ);
const __m256 MUL_2 = _mm256_set1_ps(2.0);
const __m256 R_NEED = _mm256_set1_ps(100);
//////////////////////////////////////////

const uint16_t FULL_COLORED512 = 0xFFFF;
const uint32_t BLACK_COLOR_PIXEL = 0xFF000000;

typedef enum class renderMode: char
{
	NO_OPTIMIZATION = 0,
	SSE,
	AVX256,
	AVX512,

} RENDER_MODE;

typedef struct set {
	sf::Texture texture;
	sf::Sprite image;
	int MAX_CHECK = 10;

	float scale = 0.23;
	float x_shift = -0.3;
	float y_shift = 0;

	set() {

		texture.create(WIDTH, HEIGHT);
		image.setTextureRect(sf::IntRect(0, 0, WIDTH, HEIGHT));
		image.setTexture(texture, false);
	}

} mandelbrot;

void renderSetNoSSe(mandelbrot& set, unsigned int* pixels)
{
	// printf("ren\n!");
	for (int32_t y = 0; y < HEIGHT; y++)
	{
		float im = ((float)y / WIDTH - 0.5 * HEIGHT / WIDTH) / set.scale + set.y_shift;

		uint32_t colored = 0;

        for (int32_t x = 0; x < WIDTH; x++) {

            float re = (float)x / WIDTH - 0.5 * set.scale + set.x_shift;

            float re1 = re;
			float im1 = im;

			float cmp = 0;

            for (int n = 0; n < set.MAX_CHECK; n++) {

				float re2 = re1 * re1 + im1 * im1 + re;
				float im2 =   2 * re1 * im1       + im;

				re1 = re2;
                im1 = im2;

				cmp = ~colored + cmp;

				if (   *(   (int32_t*)&cmp   ) )
				{
					pixels[y * WIDTH + x] = 0x00000000 + 0x00FF0000 - (0x0010000 * (n)+01000000 * 5 * n);
				}
			}
        }
	}
}

void renderSetAVX256(mandelbrot& set, unsigned int* pixels)
{
	int pixels_pos = 0;

	for (int y = 0; y < HEIGHT; y++) {

		float Im_num = ((float)y / WIDTH - 0.5 * HEIGHT / WIDTH) / set.scale + set.y_shift;

		for (int x = 0; x < WIDTH; x += 8) {

			__m256 Re = _mm256_set_ps(0, 1, 2, 3, 4, 5, 6, 7);

			Re = _mm256_add_ps(Re, _mm256_set1_ps((float)x));
			Re = _mm256_div_ps(Re, _mm256_set1_ps(WIDTH));
			Re = _mm256_sub_ps(Re, _mm256_set1_ps(0.5));
			Re = _mm256_div_ps(Re, _mm256_set1_ps(set.scale));
			Re = _mm256_add_ps(Re, _mm256_set1_ps(set.x_shift));

			__m256 Re0 = Re;

			// sse move
			__m256 Im = _mm256_set1_ps(Im_num);
			__m256 Im0 = Im;

			__m256 colored = _mm256_set1_ps(0);

			// black screen
			for (int i_pixel = 0; i_pixel < 8; i_pixel++)
				*(pixels + pixels_pos + i_pixel) = BLACK_COLOR_PIXEL;

			__m256 Re_2 = _mm256_mul_ps(Re, Re);
			__m256 Im_2 = _mm256_mul_ps(Im, Im);

			for (size_t n = 0; n < set.MAX_CHECK && !_mm256_testc_ps(colored, FULL_COLORED); n++) {

				Im = _mm256_fmadd_ps(MUL_2, _mm256_mul_ps(Re, Im), Im0);
				Re = _mm256_add_ps(_mm256_sub_ps(Re_2, Im_2), Re0);

				Re_2 = _mm256_mul_ps(Re, Re);
				Im_2 = _mm256_mul_ps(Im, Im);

				__m256 cmp = _mm256_cmp_ps(_mm256_add_ps(Re_2, Im_2), R_NEED, _CMP_GT_OQ);

				cmp = _mm256_andnot_ps(colored, cmp);

				for (int i_cmp = 0; i_cmp < 8; i_cmp++) {
					if (*((int*)&cmp + i_cmp)) {
						pixels[pixels_pos + ((int64_t)7 - i_cmp)] =
							(uint32_t)(BLACK_COLOR_PIXEL + 0x00FF0000 - (0x0010000 * (n)+01000000 * 5 * n));
					}
				}

				colored = _mm256_or_ps(colored, cmp);
			}

			pixels_pos += 8;
		}

	}

	return;
}

void renderSetAVX(mandelbrot& set, unsigned int* pixels)
{
	size_t pixels_pos = 0;

	static const __m512 MUL_512 = _mm512_set1_ps(2.0);
	static const __m512 R_NEED512 = _mm512_set1_ps(100);

	for (size_t y = 0; y < WIDTH; y++) {

		float Im_num = ((float)y / WIDTH - 0.5 * WIDTH / HEIGHT) / set.scale + set.y_shift;

		for (size_t x = 0; x < WIDTH; x += 16) {

			__m512   Re = _mm512_set_ps(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

			Re = _mm512_add_ps(Re, _mm512_set1_ps((float)x));
			Re = _mm512_div_ps(Re, _mm512_set1_ps(WIDTH));
			Re = _mm512_sub_ps(Re, _mm512_set1_ps(0.5));
			Re = _mm512_div_ps(Re, _mm512_set1_ps(set.scale));
			Re = _mm512_add_ps(Re, _mm512_set1_ps(set.x_shift));

			__m512   Re0 = Re;

			// sse move
			__m512   Im = _mm512_set1_ps(Im_num);
			__m512   Im0 = Im;

			uint16_t colored = 0;

			// black screen
			for (int i_pixel = 0; i_pixel < 16; i_pixel++)
				*(pixels + pixels_pos + i_pixel) = BLACK_COLOR_PIXEL;

			__m512   Re_2 = _mm512_mul_ps(Re, Re);
			__m512   Im_2 = _mm512_mul_ps(Im, Im);

			for (size_t n = 0; n < set.MAX_CHECK && colored != FULL_COLORED512; n++) {

				Im = _mm512_fmadd_ps(MUL_512, _mm512_mul_ps(Re, Im), Im0);
				Re = _mm512_add_ps(_mm512_sub_ps(Re_2, Im_2), Re0);

				Re_2 = _mm512_mul_ps(Re, Re);
				Im_2 = _mm512_mul_ps(Im, Im);

				uint16_t cmp = (uint16_t)_mm512_cmp_ps_mask(_mm512_add_ps(Re_2, Im_2), R_NEED512, _CMP_GT_OQ);


				cmp = (~colored) & cmp;
				// cmp = _mm512_andnot_ps(colored, cmp);

				for (int i_cmp = 0; i_cmp < 16; i_cmp++) {
					if (1 & (cmp >> (i_cmp))) {
						pixels[pixels_pos + ((int64_t)15 - i_cmp)] =
							(uint32_t)(BLACK_COLOR_PIXEL + 0x00FF0000 - (0x0010000 * (n)+01000000 * 5 * n));
					}
				}

				colored |= cmp;
			}

			pixels_pos += 16;
		}

	}

	return;
}

bool IsAVX512InTouch()
{


	// if num liaves < 7, then avx512 is not availavle.
	int cpuInfo[4] = {};
	__cpuid(cpuInfo, 0);

	int numLeaves = cpuInfo[0];
	if (numLeaves >= 7)
	{
		__cpuidex(cpuInfo, 7, 0);

		// Check avx512!
		return ((cpuInfo[1]) >> 16) & 0x01;
	}

	return false;
}

int main()
{
	// Load font for window.
	///////////////////////////////////////////////////////////
	sf::Font font;
	font.loadFromFile("res/reg.ttf");
	char *infoString = (char *)calloc(BUF_LEN, sizeof(char));
	

	// String for mode of rendering.
	///////////////////////////////////////////////////////////
	// TODO: remove magic numbers
	sf::Text modeString("mode", font, STD_FONT_SIZE);
	modeString.setPosition(10, 10);

	// available
	bool AVX512_AVAILABLE = IsAVX512InTouch();
	if (!AVX512_AVAILABLE)
	{
		// 4й аргумент - стиль иконки, кнопки
		MessageBoxW(NULL, L"AVX512 IS NOT AVAILABLE!", L"AVX512 ERROR", 0);
	}

	mandelbrot set;

	char *tempBuffer  = (char *)calloc(BUF_LEN, sizeof(char));
	char *tempBuffer2 = (char *)calloc(BUF_LEN, sizeof(char));
	sprintf(tempBuffer2, "%d", set.MAX_CHECK);

	strcpy(tempBuffer, "current max check:");
	strcat(tempBuffer, tempBuffer2);

	sf::Text info(tempBuffer, font, STD_FONT_SIZE);
	char* count = (char*)calloc(BUF_LEN, sizeof(char));
	info.setPosition(10, 35);

	sf::RenderWindow window(sf::VideoMode(WIDTH, HEIGHT), "mandelbrot", sf::Style::None);

	RENDER_MODE mode = RENDER_MODE::NO_OPTIMIZATION;

	uint32_t *pixels = (uint32_t *)calloc(WIDTH * HEIGHT, sizeof(uint32_t));

	while (window.isOpen())
	{
		sf::Event event;

		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
				{
					window.close();
				}

				if (event.key.code == sf::Keyboard::L)
				{
					set.MAX_CHECK++;

					strcpy(count, "current max check:");
					sprintf(tempBuffer, "%d", set.MAX_CHECK);
					strcat(count, tempBuffer);

					info.setString(count);
				}

				if (event.key.code == sf::Keyboard::K)
				{
					if (set.MAX_CHECK == 0)
					{
						MessageBoxW(NULL, L"YOU CAN'T MAKE \'MAX_CHECK\' VALUE MAKE LESS THAN ZERO", L"MAX CHANK ERROR", 0);
					}
					else
					{
						set.MAX_CHECK--;
					}

					strcpy(count, "current max check:");
					char tempBuffer[4] = {};
					sprintf(tempBuffer, "%d", set.MAX_CHECK);
					strcat(count, tempBuffer);

					info.setString(count);
				}

				if (event.key.code == sf::Keyboard::T)
				{
					if (mode == RENDER_MODE::NO_OPTIMIZATION)
					{
						mode = RENDER_MODE::SSE;

						break;
					}

					if (mode == RENDER_MODE::SSE)
					{
						mode = RENDER_MODE::AVX256;

						break;
					}

					if (mode == RENDER_MODE::AVX256)
					{
						mode = RENDER_MODE::AVX512;

						break;
					}

					if (mode == RENDER_MODE::AVX512)
					{
						mode = RENDER_MODE::NO_OPTIMIZATION;

						break;
					}
				}
			}
		}

		switch (mode)
		{
			case(RENDER_MODE::NO_OPTIMIZATION):
			{
				// renderSetNoSSe(set, pixels);

				strcpy(infoString, "mandelbrot:");
				strcat(infoString, "NO_OPTIMIZATION");

				modeString.setString(infoString);

				break;
			}

			case(RENDER_MODE::AVX256):
			{
				renderSetAVX256(set, pixels);

				strcpy(infoString, "mandelbrot:");
				strcat(infoString, "SSE");

				modeString.setString(infoString);

				break;
			}

			case(RENDER_MODE::AVX512):
			{
				if (AVX512_AVAILABLE)
				{
					renderSetAVX(set, pixels);
				}
				else
				{
					printf("NOT AVAILABLE\n");
				}

				strcpy(infoString, "mandelbrot:");
				strcat(infoString, "AVX");

				modeString.setString(infoString);

				break;
			}

			default:
			{
				renderSetAVX256(set, pixels);

				modeString.setString(infoString);

				break;
			}
		}

		window.clear();

		set.texture.update((uint8_t *)pixels);
		window.draw(set.image);
		window.draw(modeString);
		window.draw(info);

		window.display();
	}

	return 0;
}