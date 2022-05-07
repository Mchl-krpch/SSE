// File with functions of set calculation.
// Here are functions that calculate the position of
// a point in the set and color calculators.

#ifndef _mandelbrot_
#define _mandelbrot_

// For __cpuid
#ifdef _WIN32
#include <intrin.h>
#else
#include <cpuid.h>
#include <immintrin.h>
#endif

#include <SFML/Graphics.hpp>

typedef enum
{
	NoOptimizationMode = 0,
	OptimizationSSE,
	OptimizationAVX256,
	OptimizationAVX512,
} RenderMode;

 // For AVX 256
const __m256 FULL_COLORED = _mm256_cmp_ps(_mm256_set1_ps(0xFF), _mm256_set1_ps(0xFF), _CMP_EQ_OQ);
const __m256 MUL_2        = _mm256_set1_ps(2.0);
const __m256 R_NEED       = _mm256_set1_ps(100);

 // For SSE 128
const __m128 MUL = _mm_set1_ps(2.0);
const __m128 FULL_COLORED_128 = _mm_cmp_ps(_mm_set1_ps(0), _mm_set1_ps(0), _CMP_EQ_OQ);

 // For NO OPTIMIZATION
static int NORM_FULL_COLORED = 1;
static int NORM_R_NEED       = 100;
const __m128 R_NEED_128      = _mm_set1_ps(100);

typedef struct
{
	sf::Text modeString;
	sf::Text DetailLevelString;
	sf::Text fpsString;

	int *pixels = NULL;
	
	float scale   = 0.23;
	float x_shift = -0.3;
	float y_shift =    0;

	int MAX_CHECK = 256;
	
	const int      mouseMovementSensivity  = 25000;
	const uint32_t BLACK_COLOR_PIXEL       = 0xFF000000;
	const float    MOUSE_WHEEL_SENSITIVITY = 0.05;

	const float  R2 = 20;

	void setModeString(sf::Font& font, const int height, RenderMode mode)
	{
		char string[256] = "";
		strcpy(string, "render mode:");

		if (mode == RenderMode::NoOptimizationMode)
		{
			modeString.setFillColor(sf::Color(228, 84, 64, 200));
			strcat(string, "no optimization");
		}

		if (mode == RenderMode::OptimizationSSE)
		{
			modeString.setFillColor(sf::Color(228, 212, 64, 200));
			strcat(string, "sse");
		}

		if (mode == RenderMode::OptimizationAVX256)
		{
			modeString.setFillColor(sf::Color(64, 228, 130, 200));
			strcat(string, "avx");
		}

		if (mode == RenderMode::OptimizationAVX512)
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
		DetailLevelString.setFillColor(sf::Color(255, 255, 255, 200));

		char string[256] = "";
		char num[5] = "";
		strcpy(string, "Detail level:");
		sprintf(num, "%u", MAX_CHECK);
		strcat(string, num);
		DetailLevelString.setString(string);

		DetailLevelString.setFont(font);
		DetailLevelString.setPosition(10, height);
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

bool IsAVX512InTouch();

void renderSetAVX512f(const int windowWidth, const int windowHeight, mandelbrot *set);

void renderSetAVX(const int windowWidth, const int windowHeight, mandelbrot *set);

void renderSetSSE(const int windowWidth, const int windowHeight, mandelbrot *set);

void renderSetNoOptimization(const int windowWidth, const int windowHeight, mandelbrot *set);

#endif