 // Application file
 // Here are all the data that is needed for the external
 // shell of the application to work on SFML
#ifndef _winXd_
#define _winXd_

#ifdef _WIN32
#include <windows.h>
#endif

#include <SFML/Graphics.hpp>

#include "setCalculationFunctions.h"

 // Global constants.
const int BUF_LEN = 256;

const int ICON_SIZE = 128;
const int STD_FONT_SIZE = 14;
const int ELEMENTS_MARGIN = 10;

static const char *BOLD_FONT_PTR = "res/bold.ttf";
static const char *REG_FONT_PTR  = "res/reg.ttf";
static const char *ICON_PTR      = "res/icon.png";

const int FULL_SCREEN_WIDTH  = 1920;
const int FULL_SCREEN_HEIGHT = 1080;

/*	Time in milliseconds.
 It used in 'test-mode' of applicaton

 When you are testing the performance of AVX512 on a server, it is not
 profitable for you to render a lot of images on the server (because the
 image is very slow to transfer over the Internet compared to text or
 calculations), so when running an application on a remote server, it is
 wiser to take a long pause between rendering the image. The duration of
 the pause is the value of this constant. */

const int TESTING_TIME_DELAY = 2000;

 // Messages of errors from execution process.
static const char MSG_AVX512_NOT_SUPPORTED[] =
 "unfortunately, there's nothing to be done, the fact is that some\n"
 "processors do not support avx512. In this case, two exhaustive options\n"
 "can be considered. The first is to rent a server, the second is to buy\n"
 "a new computer.\n\n"
 
 "THERE IS REALLY NO REASON TO WORRY,\n\n"

 "yes, your computer do not support the fastest avx512 technology,\n"
 "but 3 standard modes will work for you:\n"
 " 1. without optimizations\n"
 " 2. sse optimization\n"
 " 3. avx256 optimization";

static const char MSG_FAILED_TO_LOAD_BOLD_FONT[] =
 "I think you misplaced the fonts folder,\n"
 "check if the font file should be located\n"
 "at the relative address res/bold.ttf\n"
 "relative to the executable .exe file";

static const char MSG_FAILED_TO_LOAD_REGULAR_FONT[] =
 "I think you misplaced the fonts folder,\n"
 "check if the font file should be located\n"
 "at the relative address res/reg.ttf\n"
 "relative to the executable .exe file";

void crossPlatformMessage(const char *title, const char *msg);

class winXd
{
private:// Constants.

	 // Type of render function.
	RenderMode renderMode   = RenderMode::NoOptimizationMode;

	 // Special flag for test fps on server.
	bool isTesting = false;

	 // Computer support of AVX512 technology
	bool isAVX512supported  = false;

	 // Window constants
	bool isFullScreen       = false;
	bool isLeftMousePressed = false;
	sf::Vector2i mousePosition;

	 // Start width and height of window
	int WIDTH  = 1200;
	int HEIGHT =  800;

	 // Variables that will store the dimensions of the
	 // window before Full Screen mode 
	int TEMP_WIDTH  = 0;
	int TEMP_HEIGHT = 0;

	 // Window components.
	sf::RenderWindow window;
	sf::Font bold;
	sf::Font reg;
	sf::Image icon;

	 // Window elements.
	sf::Texture setTexture;
	sf::Sprite setRender;
	sf::RectangleShape upPanel;

	 // Window controls
	sf::Mouse mouse;

private:// Functions.

	void createFullSreenWindow   (sf::RenderWindow& window, mandelbrot *set);
	void createCommonSreenWindow (sf::RenderWindow& window, mandelbrot *set);

	void checkSetEvent    (sf::RenderWindow& window, mandelbrot *set, sf::Event& event);
	void checkWindowEvent (sf::RenderWindow& window, mandelbrot *set, sf::Event& event);

	void checkMouseEvent(mandelbrot *set, sf::Event& event, coordinates *coords);

	// Application behavior.
	void getBehavior(sf::RenderWindow& window, sf::Event& event, coordinates *coords, mandelbrot *set)
	{
		while(window.pollEvent(event))
		{
			 // Keyboard events
			if (event.type == sf::Event::KeyPressed)
			{
				 // Escape, minimize, FullScreen events
				checkWindowEvent(window, set, event);

				 // Changing render mode, detail level, test toggle
				checkSetEvent(window, set, event);
			}

			 // Mouse events
			checkMouseEvent(set, event, coords);
		}
	}

	void setRectSettings(
		sf::RectangleShape&  rect,
		const sf::Vector2f&  vector,
		const    sf::Color&  color,
		const int posX,
		const int posY)
	{
		rect.setSize(vector);
		rect.setFillColor(color);
		rect.setPosition((float)posX, (float)posY);
	}

public:
	void create(const char *name)
	{
		 // Check if AVX512 can be used.
		if (!IsAVX512InTouch())
		{
			crossPlatformMessage("AVX512 not supported...", MSG_AVX512_NOT_SUPPORTED);
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

		// Set settings to upper panel.
		setRectSettings(
			upPanel,
			sf::Vector2f(WIDTH, 20),
			sf::Color(1, 121, 216),
			0, 0);

		sf::Text label("winXd:mandelbrot", bold, STD_FONT_SIZE);
		label.setPosition(3, 1);

		sf::Text fps("fps:", reg, STD_FONT_SIZE);
		fps.setPosition(label.getLocalBounds().width + 3 + ELEMENTS_MARGIN, 1);

		sf::Text info("Esc:exit V:minimize O:FullScreen/SmallScreen", reg, STD_FONT_SIZE);
		info.setPosition(WIDTH - info.getLocalBounds().width - ELEMENTS_MARGIN, 1);

		char *guideString = (char *)calloc(BUF_LEN, sizeof(char));
		strcpy(guideString,
			"Use the \'up arrow\', \'down arrow\' to adjust the precision of the set.\n"
			"Use your mouse to navigate through the set\n"
			"Also you can use \'T\' to change renderMode of rendering");

		sf::Text guide(guideString, reg, 14);
		guide.setFillColor(sf::Color(255, 255, 255, 150));
		guide.setPosition(ELEMENTS_MARGIN, HEIGHT - guide.getLocalBounds().height - ELEMENTS_MARGIN);

		window.create(
			sf::VideoMode(WIDTH, HEIGHT),
			name,
			sf::Style::None);

		if (!icon.loadFromFile(ICON_PTR))
		{
			#ifdef _WIN32
			MessageBox(NULL,
				"most likely the matter is in the location\n"
				"of the icon, do you have it at the\n"
				"relative address res/icon.png relative\n"
				"to the executable .exe file?",

				"failed to load image",

				0);
			#endif
		}

		window.setIcon(ICON_SIZE, ICON_SIZE, icon.getPixelsPtr());

		setTexture.create(WIDTH, HEIGHT);

		setRender.setPosition(0, 0);
		coordinates coords;

		mandelbrot set;
		set.pixels = (unsigned *)calloc(WIDTH * HEIGHT, sizeof(unsigned));
		set.setCheckText(bold, HEIGHT - 100);

		set.fpsString.setFont(reg);
		set.fpsString.setCharacterSize(STD_FONT_SIZE);
		set.fpsString.setPosition(label.getLocalBounds().width + 2 * ELEMENTS_MARGIN, 1);
		set.renew(set.fpsString);

		set.setModeString(reg, HEIGHT - 120, renderMode);

		time_t timeSpend = 0;
		time_t startTime = clock() / CLOCKS_PER_SEC;
		isTesting = false;

		sf::Text testString("TESTING", bold, 24);
		testString.setPosition(30, 30);

		while (window.isOpen())
		{
			sf::Event event;
			getBehavior(window, event, &coords, &set);

			window.clear(sf::Color::Transparent);

			// Update set.
			if (renderMode == RenderMode::NoOptimizationMode)
			{
				renderSetNoOptimization(WIDTH, HEIGHT, &set);
			}
			if (renderMode == RenderMode::OptimizationSSE)
			{
				renderSetSSE(WIDTH, HEIGHT, &set);
			}
			if (renderMode == RenderMode::OptimizationAVX256)
			{
				renderSetAVX(WIDTH, HEIGHT, &set);
			}
			if (renderMode == RenderMode::OptimizationAVX512 && isAVX512supported)
			{
				renderSetAVX512f(WIDTH, HEIGHT, &set);
			}

			if (isTesting)
			{
				timeSpend = clock() / CLOCKS_PER_SEC - startTime;
				printf("clock %ld\n", timeSpend);
				
				set.renew(set.fpsString);

				if (timeSpend > 3)
				{
					startTime = clock() / CLOCKS_PER_SEC;
					
					setTexture.update((uint8_t *)set.pixels);
					setRender.setTexture(setTexture, false);

					window.draw(setRender);

					window.draw(upPanel);
					window.draw(label);
					window.draw(info);
					window.draw(guide);

					set.setCheckText(bold, HEIGHT - 100);
					info.setPosition(WIDTH - info.getLocalBounds().width - ELEMENTS_MARGIN, 1);
					guide.setPosition(ELEMENTS_MARGIN, HEIGHT - guide.getLocalBounds().height - ELEMENTS_MARGIN);

					window.draw(set.checkInfo);
					window.draw(set.fpsString);
					window.draw(set.modeString);

					window.draw(testString);

					window.display();
				}
			}
			else
			{
				setTexture.update((uint8_t *)set.pixels);
				setRender.setTexture(setTexture, false);
				window.draw(setRender);

				set.renew(set.fpsString);

				window.draw(upPanel);
				window.draw(label);
				window.draw(info);
				window.draw(guide);

				set.setCheckText(bold, HEIGHT - 100);
				info.setPosition(WIDTH - info.getLocalBounds().width - ELEMENTS_MARGIN, 1);
				guide.setPosition(ELEMENTS_MARGIN, HEIGHT - guide.getLocalBounds().height - ELEMENTS_MARGIN);

				window.draw(set.checkInfo);
				window.draw(set.fpsString);
				window.draw(set.modeString);

				window.display();
			}
		}
	}
};

#endif