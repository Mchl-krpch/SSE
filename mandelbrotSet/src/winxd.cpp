#include "winxdConfig.h"

void crossPlatformMessage(const char *title, const char *msg)
{
	#ifdef _WIN32
	 // 4-th argument - the style of the icon, button.
	MessageBox(NULL, msg, title, 0);
	#else
	fprintf(stderr, msg);
	#endif
}

// Взято из кода GCC т.к на некоторых версиях нет __cpuidex
static __inline void
__MY_ORIGINAL_cpuidex (int __cpuid_info[4], int __leaf, int __subleaf)
{
	__asm__ __volatile__ ("cpuid\n\t"							\
			: "=a" (__cpuid_info[0]), "=b" (__cpuid_info[1]), "=c" (__cpuid_info[2]), "=d" (__cpuid_info[3])		\
			: "0" (__leaf), "2" (__subleaf));
}


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
	// if num liaves < 7, then avx512 is not availavle.
	int cpuInfo[4] = {};
	__MY_ORIGINAL_cpuidex(cpuInfo, 0, 0);

	int numLeaves = __get_cpuid_max(0, NULL);
	if (numLeaves >= 7)
	{
		__MY_ORIGINAL_cpuidex(cpuInfo, 7, 0);

		// Check avx512
		return ((cpuInfo[1]) >> 16) & 0x01;
	}

	return false;
}
#endif

void winXd::checkMouseEvent(mandelbrot *set, sf::Event& event, coordinates *coords)
{
	if (event.type == sf::Event::MouseButtonPressed)
	{
		if (event.mouseButton.button == sf::Mouse::Left)
		{
			isLeftMousePressed = true;

			mousePosition.x = mouse.getPosition().x;
			mousePosition.y = mouse.getPosition().y;
		}
	}

	if (event.type == sf::Event::MouseMoved)
	{
		if (isLeftMousePressed)
		{
			window.setPosition(sf::Vector2i((window.getPosition().x + (mouse.getPosition().x - mousePosition.x)),
				window.getPosition().y + (mouse.getPosition().y - mousePosition.y)));

			mousePosition.x = mouse.getPosition().x;
			mousePosition.y = mouse.getPosition().y;
		}
	}

	if (event.type == sf::Event::MouseButtonReleased)
	{
		if (event.mouseButton.button == sf::Mouse::Left && isLeftMousePressed)
		{
			isLeftMousePressed = false;
		}
	}


	if (event.type == sf::Event::MouseWheelMoved)
	{
		coords->getCoordinates(window, set, WIDTH, HEIGHT);

		set->scale *= 1 + set->MOUSE_WHEEL_SENSITIVITY * event.mouseWheel.delta;

		set->x_shift -= (float)coords->rel_x_coef * event.mouseWheel.delta / set->scale;
		set->y_shift -= (float)coords->rel_y_coef * event.mouseWheel.delta / set->scale;
	}
}

void winXd::createFullSreenWindow(sf::RenderWindow& window, mandelbrot *set)
{
	TEMP_WIDTH  = WIDTH;
	TEMP_HEIGHT = HEIGHT;

	WIDTH  = FULL_SCREEN_WIDTH;
	HEIGHT = FULL_SCREEN_HEIGHT;

	window.create(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot set", sf::Style::Fullscreen);
	window.setIcon(ICON_SIZE, ICON_SIZE, icon.getPixelsPtr());

	set->pixels = (unsigned *)realloc(set->pixels, WIDTH * HEIGHT * sizeof(unsigned));

	setTexture.create(WIDTH, HEIGHT);
	setRender.setPosition(0, 0);
	setRender.setTextureRect(sf::IntRect(0, 0, WIDTH, HEIGHT));
	setRender.setTexture(setTexture, false);
	set->setModeString(reg, HEIGHT - 120, renderMode);

	setRectSettings(
		upPanel,
		sf::Vector2f(WIDTH, 20),
		sf::Color(1, 121, 216),
		0, 0);
}

void winXd::createCommonSreenWindow(sf::RenderWindow& window, mandelbrot *set)
{
	WIDTH  = TEMP_WIDTH;
	HEIGHT = TEMP_HEIGHT;

	window.create(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot set", sf::Style::None);
	window.setIcon(ICON_SIZE, ICON_SIZE, icon.getPixelsPtr());

	set->pixels = (unsigned *)realloc(set->pixels, WIDTH * HEIGHT * sizeof(unsigned));
	
	setTexture.create(WIDTH, HEIGHT);
	setRender.setPosition(0, 0);
	setRender.setTextureRect(sf::IntRect(0, 0, WIDTH, HEIGHT));
	setRender.setTexture(setTexture, false);

	setRectSettings(
		upPanel,
		sf::Vector2f(WIDTH, 20),
		sf::Color(1, 121, 216),
		0, 0);
}

void winXd::checkSetEvent(sf::RenderWindow& window, mandelbrot *set, sf::Event& event)
{
	if (event.key.code == sf::Keyboard::B)
	{
		if (isTesting == true)
		{
			isTesting = false;
			return;
		}
		if (isTesting == false)
		{
			isTesting = true;
			return;
		}
	}

	if (event.key.code == sf::Keyboard::T)
	{
		if (renderMode == RenderMode::NoOptimizationMode)
		{
			renderMode = RenderMode::OptimizationSSE;
			set->setModeString(reg, HEIGHT - 120, renderMode);

			return;
		}

		if (renderMode == RenderMode::OptimizationSSE)
		{
			renderMode = RenderMode::OptimizationAVX256;
			set->setModeString(reg, HEIGHT - 120, renderMode);

			return;
		}

		if (renderMode == RenderMode::OptimizationAVX256)
		{
			if (isAVX512supported)
			{
				renderMode = RenderMode::OptimizationAVX512;
			}
			else
			{
				renderMode = RenderMode::NoOptimizationMode;
			}

			set->setModeString(reg, HEIGHT - 120, renderMode);

			return;
		}

		if (renderMode == RenderMode::OptimizationAVX512 && isAVX512supported)
		{
			renderMode = RenderMode::NoOptimizationMode;
			set->setModeString(reg, HEIGHT - 120, renderMode);
		}
	}

	if (event.key.code == sf::Keyboard::Up)
	{
		set->MAX_CHECK++;
	}

	if (event.key.code == sf::Keyboard::Down)
	{
		if (set->MAX_CHECK == 0)
		{
			#ifdef _WIN32
			MessageBox(NULL,
				"You can not make the number\n"
				"of drawn levels negative)",

				"you are trying to make a negative number of colors",

				0);
			#endif
		}
		else
		{
			set->MAX_CHECK--;
		}
	}
}

void winXd::checkWindowEvent(sf::RenderWindow& window, mandelbrot *set, sf::Event& event)
{
	if (event.key.code == sf::Keyboard::Escape)
	{
		window.close();
	}

	if (event.key.code == sf::Keyboard::V)
	{
		#ifdef _WIN32
		SendNotifyMessageW(
			window.getSystemHandle(),
			WM_SYSCOMMAND,
			SC_MINIMIZE,
			0);
		#endif
	}

	if (event.key.code == sf::Keyboard::O)
	{
		if (!isFullScreen)
		{
			createFullSreenWindow(window, set);
			
			isFullScreen = true;
		}
		else
		{
			createCommonSreenWindow(window, set);

			isFullScreen = false;
		}
	}
}

/////////////////////////////////////////////////////////////// FUNCITONS ///////

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
						set->pixels[pixels_pos + ((int64_t)15 - i_cmp)] += 
							   (int)(0x10000 * (9   * (int)(  ((double)1 - t) *              t  *              t  * t * 255)) )
							 + (int)(0x00100 * (15  * (int)(  ((double)1 - t) * ((double)1 - t) *              t  * t * 255)) )
							 + (int)(0x00001 * (8.5 * (int)(  ((double)1 - t) * ((double)1 - t) * ((double)1 - t) * t * 255)));
						// set->pixels[pixels_pos + ((int64_t)15 - i_cmp)] =
						// 	(uint32_t)(set->BLACK_COLOR_PIXEL + 0x00FF0000 - (0x0010000 * (n)+01000000 * 5 * n));
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
						set->pixels[pixels_pos + ((int64_t)7 - i_cmp)] += 
							   (int)(0x10000 * (9   * (int)(  ((double)1 - t) *              t  *              t  * t * 255)) )
							 + (int)(0x00100 * (15  * (int)(  ((double)1 - t) * ((double)1 - t) *              t  * t * 255)) )
							 + (int)(0x00001 * (8.5 * (int)(  ((double)1 - t) * ((double)1 - t) * ((double)1 - t) * t * 255)));
 							
 							// (uint32_t)(set->BLACK_COLOR_PIXEL + 0x00FF0000 - (0x0010000 * (n) + 01000000 * 5 * n));
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

				for (int i_cmp = 0; i_cmp < 4; i_cmp++) {
					if (*((int *)&cmp + i_cmp)) {
						// set->pixels[pixels_pos + ((int64_t)3 - i_cmp)] =
						// 	(uint32_t)(set->BLACK_COLOR_PIXEL + 0x00FF0000 - (0x0010000 * (n)+01000000 * 5 * n));
						set->pixels[pixels_pos + ((int64_t)3 - i_cmp)] += 
							   (int)(0x10000 * (9   * (int)(  ((double)1 - t) *              t  *              t  * t * 255)) )
							 + (int)(0x00100 * (15  * (int)(  ((double)1 - t) * ((double)1 - t) *              t  * t * 255)) )
							 + (int)(0x00001 * (8.5 * (int)(  ((double)1 - t) * ((double)1 - t) * ((double)1 - t) * t * 255)));
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
					set->pixels[y * windowWidth + x] += 
							   (int)(0x10000 * (9   * (int)(  ((double)1 - t) *              t  *              t  * t * 255)) )
							 + (int)(0x00100 * (15  * (int)(  ((double)1 - t) * ((double)1 - t) *              t  * t * 255)) )
							 + (int)(0x00001 * (8.5 * (int)(  ((double)1 - t) * ((double)1 - t) * ((double)1 - t) * t * 255)));
					// set->pixels[y * windowWidth + x] = (uint32_t)(set->BLACK_COLOR_PIXEL + 0x00FF0000 - (0x0010000 * (n)+01000000 * 5 * n));
				}

				colored |= cmp;
			}

		}

	}

	return;
}