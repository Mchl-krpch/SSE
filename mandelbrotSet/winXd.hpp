#ifndef _winXd_
#define _winXd_

#include "mandelbrot.hpp"

#include <SFML/Graphics.hpp>
#include <windows.h>
#include <stdint.h>

#define BUF_LEN 256

const int STD_ICON_SIZE = 128;
const int STD_FONT_SIZE = 14;
const int MARGIN        = 10;

class winXd
{
private:
	int mode = 0;

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

	// Window navigation.
	void getBehavior(sf::RenderWindow& window, sf::Event& event, coordinates *coords, mandelbrot *set)
	{
		while(window.pollEvent(event))
		{
			// Exit the app
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
				{
					window.close();
				}

				if (event.key.code == sf::Keyboard::T)
				{
					if (mode == 0)
					{
						mode = 1;
						set->setModeString(reg, HEIGHT - 120, mode);

						break;
					}

					if (mode == 1)
					{
						mode = 2;
						set->setModeString(reg, HEIGHT - 120, mode);

						break;
					}

					if (mode == 2)
					{
						if (isAVX512supported)
						{
							mode = 3;
						}
						else
						{
							mode = 0;
						}

						set->setModeString(reg, HEIGHT - 120, mode);

						break;
					}
					if (mode == 3 && isAVX512supported)
					{
						mode = 0;
						set->setModeString(reg, HEIGHT - 120, mode);
					}
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
						set->setModeString(reg, HEIGHT - 120, mode);

						setRectSettings(
							upPanel,
							sf::Vector2f(WIDTH, 20),
							sf::Color(1, 121, 216),
							0, 0);
						
						isFullScreen = true;
					}
					else
					{
						WIDTH = TEMP_WIDTH;
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

						isFullScreen = false;
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

			// Dragging window.
			if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Left) {
					isLeftMousePressed = true;

					mousePosition.x = mouse.getPosition().x;
					mousePosition.y = mouse.getPosition().y;
				}
			}

			if (event.type == sf::Event::MouseMoved) {
				if (isLeftMousePressed) {
					window.setPosition(sf::Vector2i((window.getPosition().x + (mouse.getPosition().x - mousePosition.x)),
						window.getPosition().y + (mouse.getPosition().y - mousePosition.y)));

					mousePosition.x = mouse.getPosition().x;
					mousePosition.y = mouse.getPosition().y;
				}
			}

			if (event.type == sf::Event::MouseButtonReleased) {
				if (event.mouseButton.button == sf::Mouse::Left && isLeftMousePressed) {
					isLeftMousePressed = false;
				}
			}


			if (event.type == sf::Event::MouseWheelMoved) {
				coords->getCoordinates(window, set, WIDTH, HEIGHT);

				set->scale *= 1 + set->MOUSE_WHEEL_SENSITIVITY * event.mouseWheel.delta;

				set->x_shift -= (float)coords->rel_x_coef * event.mouseWheel.delta / set->scale;
				set->y_shift -= (float)coords->rel_y_coef * event.mouseWheel.delta / set->scale;
			}
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
		else
		{
			isAVX512supported = true;
		}

		// Try to load data.
		if (!bold.loadFromFile("res/bold.ttf"))
		{
			MessageBox(NULL,
				"I think you misplaced the fonts folder,\n"
				"check if the font file should be located\n"
				"at the relative address res/bold.ttf\n"
				"relative to the executable .exe file",

				"Application failed to load bold font",

				0);
		}

		if (!reg.loadFromFile("res/reg.ttf"))
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
			"Also you can use \'T\' to change mode of rendering");

		sf::Text guide(guideString, reg, 14);
		guide.setFillColor(sf::Color(255, 255, 255, 150));
		guide.setPosition(MARGIN, HEIGHT - guide.getLocalBounds().height - MARGIN);

		window.create(
			sf::VideoMode(WIDTH, HEIGHT),
			name,
			sf::Style::None);

		if (!icon.loadFromFile("res/icon.png"))
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
		setRender.setTextureRect(sf::IntRect(0, 0, WIDTH, HEIGHT));
		setRender.setTexture(setTexture, false);

		coordinates coords;

		mandelbrot set;
		set.pixels = (unsigned *)calloc(WIDTH * HEIGHT, sizeof(unsigned));
		set.setCheckText(bold, HEIGHT - 100);

		set.fpsString.setFont(reg);
		set.fpsString.setCharacterSize(STD_FONT_SIZE);
		set.fpsString.setPosition(label.getLocalBounds().width + 2 * MARGIN, 1);
		set.renew(set.fpsString);

		set.setModeString(reg, HEIGHT - 120, mode);

		while (window.isOpen())
		{
			sf::Event event;
			getBehavior(window, event, &coords, &set);

			// Render different parts of the window.
			window.clear(sf::Color::Transparent);

			// Update set.
			if (mode == 0)
			{
				renderSetNoOptimization(WIDTH, HEIGHT, &set);
			}
			if (mode == 1)
			{
				renderSetSSE128(WIDTH, HEIGHT, &set);
			}
			if (mode == 2)
			{
				renderSetAVX256(WIDTH, HEIGHT, &set);
			}
			if (mode == 3 && isAVX512supported)
			{
				renderSetAVX512(WIDTH, HEIGHT, &set);
			}

			setTexture.update((uint8_t *)set.pixels);
			window.draw(setRender);

			// Update fps.
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
};

#endif