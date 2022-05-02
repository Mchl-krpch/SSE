#ifndef _winXd_
#define _winXd_

#include <SFML/Graphics.hpp>
#include <windows.h>
#include <stdint.h>

#include "mandelbrot.hpp"

const int BUF_LEN = 256;

const int STD_ICON_SIZE = 128;
const int STD_FONT_SIZE = 14;
const int MARGIN        = 10;

const char *BOLD_FONT_PTR = "res/bold.ttf";
const char *REG_FONT_PTR  = "res/reg.ttf";
const char *ICON_PTR      = "res/icon.png";

const int FULL_SCREEN_WIDTH  = 1920;
const int FULL_SCREEN_HEIGHT = 1080;

const int TESTING_TIME_DELAY = 2000; // time in milliseconds

class winXd
{
private:
	int renderMode = 0;

	// Special flag for test fps on server.
	bool isTesting = false;
	bool isAVX512supported = false;

	bool isFullScreen = false;
	bool isLeftMousePressed = false;
	sf::Vector2i mousePosition;

	// Constants & variables
	int WIDTH  = 1200;
	int HEIGHT =  800;

	int TEMP_WIDTH  = 0;
	int TEMP_HEIGHT = 0;

	sf::RenderWindow window;
	sf::Font bold;
	sf::Font reg;

	// Render objects.
	sf::Texture setTexture;
	sf::Sprite  setRender;

	// Devices & need
	sf::Image icon;
	sf::Mouse mouse;
	sf::RectangleShape upPanel;

	void checkMouseEvent(mandelbrot *set, sf::Event& event, coordinates *coords)
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

	void createFullSreenWindow(sf::RenderWindow& window, mandelbrot *set)
	{
		TEMP_WIDTH  = WIDTH;
		TEMP_HEIGHT = HEIGHT;

		WIDTH  = 1920;
		HEIGHT = 1080;

		window.create(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot set", sf::Style::Fullscreen);
		window.setIcon(STD_ICON_SIZE, STD_ICON_SIZE, icon.getPixelsPtr());

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

	void createCommonSreenWindow(sf::RenderWindow& window, mandelbrot *set)
	{
		WIDTH  = TEMP_WIDTH;
		HEIGHT = TEMP_HEIGHT;

		window.create(sf::VideoMode(WIDTH, HEIGHT), "Mandelbrot set", sf::Style::None);
		window.setIcon(STD_ICON_SIZE, STD_ICON_SIZE, icon.getPixelsPtr());

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

	void checkSetEvent(sf::RenderWindow& window, mandelbrot *set, sf::Event& event)
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
			if (renderMode == 0)
			{
				renderMode = 1;
				set->setModeString(reg, HEIGHT - 120, renderMode);

				return;
			}

			if (renderMode == 1)
			{
				renderMode = 2;
				set->setModeString(reg, HEIGHT - 120, renderMode);

				return;
			}

			if (renderMode == 2)
			{
				if (isAVX512supported)
				{
					renderMode = 3;
				}
				else
				{
					renderMode = 0;
				}

				set->setModeString(reg, HEIGHT - 120, renderMode);

				return;
			}

			if (renderMode == 3 && isAVX512supported)
			{
				renderMode = 0;
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
				MessageBox(NULL,
					"You can not make the number\n"
					"of drawn levels negative)",

					"you are trying to make a negative number of colors",

					0);
			}
			else
			{
				set->MAX_CHECK--;
			}
		}
	}

	void checkWindowEvent(sf::RenderWindow& window, mandelbrot *set, sf::Event& event)
	{
		if (event.key.code == sf::Keyboard::Escape)
		{
			window.close();
		}

		if (event.key.code == sf::Keyboard::V)
		{
			SendNotifyMessageW(
				window.getSystemHandle(),
				WM_SYSCOMMAND,
				SC_MINIMIZE,
				0);
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

	// Window navigation.
	void getBehavior(sf::RenderWindow& window, sf::Event& event, coordinates *coords, mandelbrot *set)
	{
		while(window.pollEvent(event))
		{
			// Exit the app
			if (event.type == sf::Event::KeyPressed)
			{
				// Escape, minimize, FullScreen events
				checkWindowEvent(window, set, event);

				// Changing render mode, detail level, test toggle
				checkSetEvent(window, set, event);
			}

			// Dragging window.
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
		bool AVX512_AVAILABLE = IsAVX512InTouch();
		if (!AVX512_AVAILABLE)
		{
			// 4й аргумент - стиль иконки, кнопки
			MessageBox(NULL,
				"unfortunately, there's nothing to be done, the fact is that some processors "
				"do not support avx512. In this case, two exhaustive options can be considered."
				"The first is to rent a server, the second is to buy a new computer.",

				"AVX512 not supported...",
				0);
		}

		// Try to load data.
		if (!bold.loadFromFile(BOLD_FONT_PTR))
		{
			MessageBox(NULL,
				"I think you misplaced the fonts folder,\n"
				"check if the font file should be located\n"
				"at the relative address res/bold.ttf\n"
				"relative to the executable .exe file",

				"Application failed to load bold font",

				0);
		}

		if (!reg.loadFromFile(REG_FONT_PTR))
		{
			MessageBox(NULL,
				"I think you misplaced the fonts folder,\n"
				"check if the font file should be located\n"
				"at the relative address res/reg.ttf\n"
				"relative to the executable .exe file",

				"Application failed to load regular font",

				0);
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
		fps.setPosition(label.getLocalBounds().width + 3 + MARGIN, 1);

		sf::Text info("Esc:exit V:minimize O:FullScreen/SmallScreen", reg, STD_FONT_SIZE);
		info.setPosition(WIDTH - info.getLocalBounds().width - MARGIN, 1);

		char *guideString = (char *)calloc(BUF_LEN, sizeof(char));
		strcpy(guideString,
			"Use the \'up arrow\', \'down arrow\' to adjust the precision of the set.\n"
			"Use your mouse to navigate through the set\n"
			"Also you can use \'T\' to change renderMode of rendering");

		sf::Text guide(guideString, reg, 14);
		guide.setFillColor(sf::Color(255, 255, 255, 150));
		guide.setPosition(MARGIN, HEIGHT - guide.getLocalBounds().height - MARGIN);

		window.create(
			sf::VideoMode(WIDTH, HEIGHT),
			name,
			sf::Style::None);

		if (!icon.loadFromFile(ICON_PTR))
		{
			MessageBox(NULL,
				"most likely the matter is in the location\n"
				"of the icon, do you have it at the\n"
				"relative address res/icon.png relative\n"
				"to the executable .exe file?",

				"failed to load image",

				0);
		}

		window.setIcon(STD_ICON_SIZE, STD_ICON_SIZE, icon.getPixelsPtr());

		setTexture.create(WIDTH, HEIGHT);

		setRender.setPosition(0, 0);
		coordinates coords;

		mandelbrot set;
		set.pixels = (unsigned *)calloc(WIDTH * HEIGHT, sizeof(unsigned));
		set.setCheckText(bold, HEIGHT - 100);

		set.fpsString.setFont(reg);
		set.fpsString.setCharacterSize(STD_FONT_SIZE);
		set.fpsString.setPosition(label.getLocalBounds().width + 2 * MARGIN, 1);
		set.renew(set.fpsString);

		set.setModeString(reg, HEIGHT - 120, renderMode);

		time_t timeSpend = 0;
		time_t startTime = clock();
		isTesting = false;

		sf::Text testString("TESTING", bold, 24);
		testString.setPosition(30, 30);

		while (window.isOpen())
		{
			sf::Event event;
			getBehavior(window, event, &coords, &set);

			window.clear(sf::Color::Transparent);

			// Update set.
			if (renderMode == 0)
			{
				renderSetNoOptimization(WIDTH, HEIGHT, &set);
			}
			if (renderMode == 1)
			{
				renderSetSSE(WIDTH, HEIGHT, &set);
			}
			if (renderMode == 2)
			{
				renderSetAVX(WIDTH, HEIGHT, &set);
			}
			if (renderMode == 3 && isAVX512supported)
			{
				renderSetAVX512f(WIDTH, HEIGHT, &set);
			}

			if (isTesting)
			{
				timeSpend = clock() - startTime;
				set.renew(set.fpsString);
				
				if (timeSpend > TESTING_TIME_DELAY)
				{
					printf("clock\n");
					startTime = clock();
					
					setTexture.update((uint8_t *)set.pixels);
					setRender.setTexture(setTexture, false);

					window.draw(setRender);

					window.draw(upPanel);
					window.draw(label);
					window.draw(info);
					window.draw(guide);

					set.setCheckText(bold, HEIGHT - 100);
					info.setPosition(WIDTH - info.getLocalBounds().width - MARGIN, 1);
					guide.setPosition(MARGIN, HEIGHT - guide.getLocalBounds().height - MARGIN);

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
				info.setPosition(WIDTH - info.getLocalBounds().width - MARGIN, 1);
				guide.setPosition(MARGIN, HEIGHT - guide.getLocalBounds().height - MARGIN);

				window.draw(set.checkInfo);
				window.draw(set.fpsString);
				window.draw(set.modeString);

				window.display();
			}
		}
	}
};

#endif