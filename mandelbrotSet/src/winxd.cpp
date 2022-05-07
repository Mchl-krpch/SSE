// File with implimentation functions of winXd forms.
#include "winxd.h"

void setRectSettings(
	sf::RectangleShape& rect,
	const sf::Vector2f& vector,
	const    sf::Color& color,
	const int posX,
	const int posY)
{
	rect.setSize(vector);
	rect.setFillColor(color);
	rect.setPosition((float)posX, (float)posY);
}

void crossPlatformMessage(const char *title, const char *msg)
{
	#ifdef _WIN32
	 // 4-th argument - the style of the icon, button.
	MessageBox(NULL, msg, title, 0);
	#else
	fprintf(stderr, msg);
	#endif
}

// Taken from GCC code as some versions don't have __cpuidex
static __inline void __MY_ORIGINAL_cpuidex (int __cpuid_info[4], int __leaf, int __subleaf)
{
	__asm__ __volatile__ ("cpuid\n\t"							                                         \
		: "=a" (__cpuid_info[0]), "=b" (__cpuid_info[1]), "=c" (__cpuid_info[2]), "=d" (__cpuid_info[3]) \
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

void winXd::create(const char *name)
{
	 // Check if AVX512 can be used.
	if (!IsAVX512InTouch())
	{
		crossPlatformMessage("AVX512 not supported on you PC", MSG_AVX512_NOT_SUPPORTED);
	}
	else
	{
		isAVX512supported = true;
	}

	 // Try to load fonts.
	if (!bold.loadFromFile(BOLD_FONT_PTR)){
		crossPlatformMessage("Application failed to load bold font", MSG_FAILED_TO_LOAD_BOLD_FONT);
	}

	if (!reg.loadFromFile(REG_FONT_PTR))
	{
		crossPlatformMessage("Application failed to load regular font", MSG_FAILED_TO_LOAD_REGULAR_FONT);
	}

	 // Set settings to upper panel and UX instruction.
	setUpperPanel   (NameInUpperPanel, windowShortcuts);
	setControlLabel (guideString, guide);

	window.create(sf::VideoMode(WIDTH, HEIGHT),	name, sf::Style::None);

	if (!icon.loadFromFile(ICON_PTR))
	{
		crossPlatformMessage("Failed to load icon", MSG_FAILED_TO_LOAD_ICON);
	}

	window.setIcon(ICON_SIZE, ICON_SIZE, icon.getPixelsPtr());

	setTexture.create(WIDTH, HEIGHT);

	setRender.setPosition(0, 20);
	coordinates coords;

	mandelbrot set;
	set.pixels = (int *)calloc(WIDTH * HEIGHT, sizeof(int));
	set.setCheckText(bold, HEIGHT - 100);

	setFpsText(set.fpsString);
	set.setModeString(reg, HEIGHT - 120, renderMode);

	time_t timeSpend = 0;
	time_t startTime = clock() / CLOCKS_PER_SEC;
	isTesting = false;

	sf::Text testString("TESTING", bold, BIG_FONT_SIZE);
	testString.setPosition(30, 30);
	
	while (window.isOpen())
	{
		sf::Event event;
		getBehavior(window, event, &coords, &set);

		window.clear(sf::Color::Transparent);

		 // Update set with one of render-function [depends on mode].
		switch (renderMode)
		{
			case (RenderMode::NoOptimizationMode):
			{
				renderSetNoOptimization(WIDTH, HEIGHT, &set);
				break;
			}
			case (RenderMode::OptimizationSSE):
			{
				renderSetSSE(WIDTH, HEIGHT, &set);
				break;
			}
			case (RenderMode::OptimizationAVX256):
			{
				renderSetAVX(WIDTH, HEIGHT, &set);
				break;
			}
			case (RenderMode::OptimizationAVX512):
			{
				if (isAVX512supported)
				{
					renderSetAVX512f(WIDTH, HEIGHT, &set);
				}
				break;
			}
			default:
			{
				renderSetNoOptimization(WIDTH, HEIGHT, &set);
				break;
			}
		}

		 /*  SWITCH MODE
		  If the 'Testing' mode is enabled, the window rendering will
		  be slow. Between frames will pass 'TESTING_TIME_DELAY'-ms */

		if (isTesting)
		{
			timeSpend = clock() / CLOCKS_PER_SEC - startTime;
			set.renew(set.fpsString);

			 // Check timeSpend
			if (timeSpend > 3)
			{
				startTime = clock() / CLOCKS_PER_SEC;
				
				 // Main drawing functon
				updateMandelbrotWindow(
					window,
					setTexture, setRender,
					upPanel, NameInUpperPanel, windowShortcuts, guide,
					&set);

				window.draw(testString);

				window.display();
			}
		}
		else
		{
			updateMandelbrotWindow(
				window,
				setTexture, setRender,
				upPanel, NameInUpperPanel, windowShortcuts, guide,
				&set);

			window.display();
		}
	}

	free(set.pixels);
}

void winXd::setUpperPanel(sf::Text& NameInUpperPanel, sf::Text& windowShortcuts)
{
	setRectSettings(upPanel, sf::Vector2f(WIDTH, 20),
					highlightedColor, 0, 0);

	NameInUpperPanel.setString("winXd:mandelbrot");
	NameInUpperPanel.setFont(bold);
	NameInUpperPanel.setCharacterSize(STD_FONT_SIZE);
	NameInUpperPanel.setPosition(3, 1);

	windowShortcuts.setString("Esc:exit V:minimize O:FullScreen/SmallScreen");
	windowShortcuts.setFont(reg);
	windowShortcuts.setCharacterSize(STD_FONT_SIZE);
	windowShortcuts.setPosition(WIDTH - windowShortcuts.getLocalBounds().width - ELEMENTS_MARGIN, 1);
}

void winXd::setControlLabel(char *guideString, sf::Text& guide)
{
	strcpy(guideString, CONTROL_INFO_TEXT);

	guide.setString(guideString);
	guide.setFont(reg);
	guide.setCharacterSize(14);
	guide.setFillColor(paleWhite);
	guide.setPosition(ELEMENTS_MARGIN, HEIGHT - guide.getLocalBounds().height - ELEMENTS_MARGIN);
}

void winXd::setFpsText(sf::Text& fpsString)
{
	fpsString.setFont(reg);
	fpsString.setCharacterSize(STD_FONT_SIZE);
	fpsString.setPosition(NameInUpperPanel.getLocalBounds().width + 2 * ELEMENTS_MARGIN, 1);
}

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

void winXd::updateMandelbrotWindow(
	sf::RenderWindow& window,
	sf::Texture& setTexture, sf::Sprite& setRender,
	sf::RectangleShape& upPanel, sf::Text& NameInUpperPanel, sf::Text& windowShortcuts, sf::Text& guide,
	mandelbrot *set)
{
	setTexture.update((uint8_t *)set->pixels);
	setRender.setTexture(setTexture, false);
	window.draw(setRender);

	set->renew(set->fpsString);

	window.draw(upPanel);
	window.draw(NameInUpperPanel);
	window.draw(windowShortcuts);
	window.draw(guide);

	window.draw(set->DetailLevelString);
	window.draw(set->modeString);
	window.draw(set->fpsString);
}

void winXd::createFullSreenWindow(sf::RenderWindow& window, mandelbrot *set, sf::Text& windowShortcuts, sf::Text& guide)
{
	TEMP_WIDTH  = WIDTH;
	TEMP_HEIGHT = HEIGHT;

	WIDTH  = FULL_SCREEN_WIDTH;
	HEIGHT = FULL_SCREEN_HEIGHT;

	window.create(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot set", sf::Style::Fullscreen);
	window.setIcon(ICON_SIZE, ICON_SIZE, icon.getPixelsPtr());


	set->pixels = (int *)realloc(set->pixels, WIDTH * HEIGHT * sizeof(int));

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

	windowShortcuts.setPosition(WIDTH - windowShortcuts.getLocalBounds().width - ELEMENTS_MARGIN, 1);
	guide.setPosition(ELEMENTS_MARGIN, HEIGHT - guide.getLocalBounds().height - ELEMENTS_MARGIN);
	set->setModeString(reg, HEIGHT - 120, renderMode);

	set->setCheckText(bold, HEIGHT - 100);
}

void winXd::createCommonSreenWindow(sf::RenderWindow& window, mandelbrot *set, sf::Text& windowShortcuts, sf::Text& guide)
{
	WIDTH  = TEMP_WIDTH;
	HEIGHT = TEMP_HEIGHT;

	window.create(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot set", sf::Style::None);
	window.setIcon(ICON_SIZE, ICON_SIZE, icon.getPixelsPtr());

	set->pixels = (int *)realloc(set->pixels, WIDTH * HEIGHT * sizeof(int));
	
	setTexture.create(WIDTH, HEIGHT);
	setRender.setPosition(0, 0);
	setRender.setTextureRect(sf::IntRect(0, 0, WIDTH, HEIGHT));
	setRender.setTexture(setTexture, false);

	setRectSettings(
		upPanel,
		sf::Vector2f(WIDTH, 20),
		sf::Color(1, 121, 216),
		0, 0);

	windowShortcuts.setPosition(WIDTH - windowShortcuts.getLocalBounds().width - ELEMENTS_MARGIN, 1);
	guide.setPosition(ELEMENTS_MARGIN, HEIGHT - guide.getLocalBounds().height - ELEMENTS_MARGIN);
	set->setModeString(reg, HEIGHT - 120, renderMode);

	set->setCheckText(bold, HEIGHT - 100);
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
		set->setCheckText(bold, HEIGHT - 100);
	}

	if (event.key.code == sf::Keyboard::Down)
	{
		if (set->MAX_CHECK == 0)
		{
			crossPlatformMessage("you are trying to make a negative number of colors", MSG_NEGATIV_NUMBER_OF_COLORS);
		}
		else
		{
			set->MAX_CHECK--;
		}

		set->setCheckText(bold, HEIGHT - 100);
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
			createFullSreenWindow(window, set, windowShortcuts, guide);
			
			isFullScreen = true;
		}
		else
		{
			createCommonSreenWindow(window, set, windowShortcuts, guide);

			isFullScreen = false;
		}
	}
}