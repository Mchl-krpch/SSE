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

const int ICON_SIZE       = 128;
const int BIG_FONT_SIZE   = 24;
const int STD_FONT_SIZE   = 14;
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
 " 3. avx256 optimization\n";

static const char MSG_FAILED_TO_LOAD_BOLD_FONT[] =
 "I think you misplaced the fonts folder,\n"
 "check if the font file should be located\n"
 "at the relative address res/bold.ttf\n"
 "relative to the executable .exe file\n";

static const char MSG_FAILED_TO_LOAD_REGULAR_FONT[] =
 "I think you misplaced the fonts folder,\n"
 "check if the font file should be located\n"
 "at the relative address res/reg.ttf\n"
 "relative to the executable .exe file\n";

static const char MSG_FAILED_TO_LOAD_ICON[] =
 "most likely the matter is in the location\n"
 "of the icon, do you have it at the\n"
 "relative address res/icon.png relative\n"
 "to the executable .exe file?\n";

static const char MSG_NEGATIV_NUMBER_OF_COLORS[] =
 "You can not make the number\n"
 "of drawn levels negative)\n";

static const char CONTROL_INFO_TEXT[] = 
 "Use the \'up arrow\', \'down arrow\' to adjust the precision of the set.\n"
 "Use your mouse to navigate through the set\n"
 "Also you can use \'T\' to change renderMode of rendering\n";

void crossPlatformMessage(const char *title, const char *msg);

void setRectSettings(
	sf::RectangleShape& rect,
	const sf::Vector2f& vector,
	const    sf::Color& color,
	const int posX,
	const int posY);

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

	 // Upper panel elements.
	sf::Text NameInUpperPanel;
	sf::Text windowShortcuts;

	 // Color palette
	sf::Color highlightedColor {  1, 121, 216,  255};
	sf::Color paleWhite        {255, 255, 255,  150};

	 // Window controls
	sf::Mouse mouse;
	sf::Text  guide;
	char *guideString = (char *)calloc(BUF_LEN, sizeof(char));

private:// Functions.

	void createFullSreenWindow   (sf::RenderWindow& window, mandelbrot *set, sf::Text& windowShortcuts, sf::Text& guide);
	void createCommonSreenWindow (sf::RenderWindow& window, mandelbrot *set, sf::Text& windowShortcuts, sf::Text& guide);

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

	 // UI-Element constructors.
	void setUpperPanel(sf::Text& label, sf::Text& windowShortcuts);
	void setControlLabel(char *guideString, sf::Text& guide);
	void setFpsText(sf::Text& fpsString);

	void updateMandelbrotWindow(sf::RenderWindow& window,
		sf::Texture& setTexture,sf::Sprite& setRender,
		sf::RectangleShape& upPanel, sf::Text& label, sf::Text& windowShortcuts, sf::Text& guide,
		mandelbrot *set);

public:
	void create(const char *name);
};

#endif