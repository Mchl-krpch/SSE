// main libs
#include <stdio.h>
#include <stdint.h>
#include <SFML/Graphics.hpp>

// For special mode of window
#include <Dwmapi.h>
#pragma comment (lib, "Dwmapi.lib")

// For _m256
#include <immintrin.h>

typedef  __m256d  sse_t;
typedef  uint8_t     u8;
typedef uint32_t    u32;
typedef  int64_t    s64;

const float  windowWidth   = 1200;
const float  windowHeight  =  800;
const s64    screenWidth   = 1920;
const s64    screenHeight  = 1080;
const double R2            =   20;

const int    mouseMovementSensivity  = 25000;

const u32    MAX_CHECK               = 32;
const u32    BLACK_COLOR_PIXEL       = 0xFF000000;
const double MOUSE_WHEEL_SENSITIVITY = 0.05;

const __m256d FULL_COLORED = _mm256_cmp_pd(_mm256_set1_pd(0), _mm256_set1_pd(0), _CMP_EQ_OQ);
const __m256d MUL_2        = _mm256_set1_pd(2.0);
const __m256d R_NEED       = _mm256_set1_pd(100);


struct fpsCounter
{
	sf::Clock clock;
	sf::Time  time = clock.getElapsedTime();

	double time_prev     = time.asSeconds();
	double time_now      = 0;
	double time_last_out = 0;

	const double FPS_DELAY = 0.15;

	char str[32] = "fps:";

	void Renew(sf::Text& fpsLabel)
	{
		time = clock.getElapsedTime();
		time_now = time.asSeconds();

		if (time_now - time_last_out > FPS_DELAY)
		{
			float cur_fps = (float)(1 / (time_now - time_prev));

			sprintf_s(str + 4, 26, "%.2lf\n", (float)(1 / (time_now - time_prev)));
			fpsLabel.setString(str);

			time_last_out = time_now;
		}

		time_prev = time_now;
	}
};

struct coordinates
{
	float rel_x_coef = 0;
	float rel_y_coef = 0;

	void getCoordinates(sf::RenderWindow& window)
	{
		sf::Vector2f position;

		position.x = sf::Mouse::getPosition().x - window.getPosition().x - (windowWidth / 2);
		position.y = sf::Mouse::getPosition().y - window.getPosition().y - (windowHeight / 2);

		// printf("%f %f\n", position.x, position.y);

		rel_x_coef = ((float)(-1 * (int)position.x) / mouseMovementSensivity);
		rel_y_coef = ((float)(-1 * (int)position.y) / mouseMovementSensivity);
	}
};

class winXdApp
{
private:
	MARGINS          margins      = {};

	sf::Event        event;
	sf::Vector2i     cur_pos;
	sf::Mouse        mouse;
	sf::Font         winBold;
	sf::Font         winReg;
	sf::Image        icon;
	sf::Text         fpsString;

	sf::Texture      texture;
	sf::Sprite       set;

	sf::Color        textColor    = { 0x00, 0x00, 0x00, 0xFF       };
	sf::Color        textSubColor = { 0x00, 0x00, 0x00, 0xFF * 0.8 };
	sf::Color        bodyColor    = { 0xFF, 0xFF, 0xFF, 0xFF       };  // { 8, 28, 35, 255 };

	coordinates      mousePosition;

	double scale   = 0.23;
	double x_shift = -0.3;
	double y_shift =    0;
	bool isPressed = false;

	const int boarderSize =  3;
	const int textSize    = 14;
	const int upperPanelH = 20;

	void renderSet(unsigned int* pixels) {

		size_t pixels_pos = 0;

		for (size_t y = 0; y < windowHeight; y++) {

			double Im_num = ((double)y / windowWidth - 0.5 * windowHeight / windowWidth) / scale + y_shift;

			for (size_t x = 0; x < windowWidth; x += 4) {

				sse_t Re = _mm256_set_pd(0, 1, 2, 3);

				Re = _mm256_add_pd(Re, _mm256_set1_pd((double)x));
				Re = _mm256_div_pd(Re, _mm256_set1_pd(windowWidth));
				Re = _mm256_sub_pd(Re, _mm256_set1_pd(0.5));
				Re = _mm256_div_pd(Re, _mm256_set1_pd(scale));
				Re = _mm256_add_pd(Re, _mm256_set1_pd(x_shift));

				sse_t Re0 = Re;

				// sse move
				sse_t Im = _mm256_set1_pd(Im_num);
				sse_t Im0 = Im;

				sse_t colored = _mm256_set1_pd(0);

				// black screen
				for (int i_pixel = 0; i_pixel < 4; i_pixel++)
					*(pixels + pixels_pos + i_pixel) = BLACK_COLOR_PIXEL;

				sse_t Re_2 = _mm256_mul_pd(Re, Re);
				sse_t Im_2 = _mm256_mul_pd(Im, Im);

				for (size_t n = 0; n < MAX_CHECK && !_mm256_testc_pd(colored, FULL_COLORED); n++) {

					Im = _mm256_fmadd_pd(MUL_2, _mm256_mul_pd(Re, Im), Im0);
					Re = _mm256_add_pd(_mm256_sub_pd(Re_2, Im_2), Re0);

					Re_2 = _mm256_mul_pd(Re, Re);
					Im_2 = _mm256_mul_pd(Im, Im);

					sse_t cmp = _mm256_cmp_pd(_mm256_add_pd(Re_2, Im_2), R_NEED, _CMP_GT_OQ);

					cmp = _mm256_andnot_pd(colored, cmp);

					for (int i_cmp = 0; i_cmp < 4; i_cmp++) {
						if (*((long long*)&cmp + i_cmp)) {
							pixels[pixels_pos + ((s64)3 - i_cmp)] =
								(u32)(BLACK_COLOR_PIXEL + 0x00FF0000 - (0x0010000 * (n)+01000000 * 5 * n));
						}
					}

					colored = _mm256_or_pd(colored, cmp);
				}

				pixels_pos += 4;
			}

		}

		return;
	}


	void getBehavior(sf::RenderWindow& window, sf::Event& event)
	{
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Escape)
				{
					window.close();
					event.type = sf::Event::Closed;

					return;
				}

				if (event.key.code == sf::Keyboard::V)
				{
					SendNotifyMessageW(
						window.getSystemHandle(),
						WM_SYSCOMMAND,
						SC_MINIMIZE,
						0);
				}
			}

			if (event.type == sf::Event::MouseWheelMoved) {
				scale *= 1 + MOUSE_WHEEL_SENSITIVITY * event.mouseWheel.delta;

				x_shift -= (double)mousePosition.rel_x_coef * event.mouseWheel.delta / scale;
				y_shift -= (double)mousePosition.rel_y_coef * event.mouseWheel.delta / scale;
			}

			if (event.type == sf::Event::MouseButtonPressed) {
				if (event.key.code == sf::Mouse::Left) {
					isPressed = true;

					cur_pos.x = mouse.getPosition().x;
					cur_pos.y = mouse.getPosition().y;
				}
			}

			if (event.type == sf::Event::MouseMoved) {
				if (isPressed) {
					window.setPosition(sf::Vector2i((window.getPosition().x + (mouse.getPosition().x - cur_pos.x)),
						window.getPosition().y + (mouse.getPosition().y - cur_pos.y)));

					cur_pos.x = mouse.getPosition().x;
					cur_pos.y = mouse.getPosition().y;
				}
			}

			if (event.type == sf::Event::MouseButtonReleased) {
				if (event.key.code == sf::Mouse::Left && isPressed) {
					isPressed = false;
				}
			}
		}
	}

	void setRectSettings(
			sf::RectangleShape& rect,
			const sf::Vector2f& vector,
			const sf::Color& color,
			const int posX, const int posY)
	{
		rect.setSize(vector);
		rect.setFillColor(color);
		rect.setPosition((float)posX, (float)posY);
	}

	void setIcon(sf::RenderWindow& window, const char* iconName)
	{
		icon.loadFromFile(iconName);
		window.setIcon(128, 128, icon.getPixelsPtr());
	}

public:
	sf::RenderWindow window;

	void drawFrame(sf::RenderWindow& window)
	{
		margins.cxLeftWidth = -1;

		fpsCounter fps;
		fpsString.setPosition       (sf::Vector2f(144, 1));
		fpsString.setFillColor      (textSubColor);
		fpsString.setFont           (winReg);
		fpsString.setCharacterSize  (textSize);

		window.create(
			sf::VideoMode(windowWidth + 2 * boarderSize, upperPanelH + windowHeight + boarderSize),
			"winXd",
			sf::Style::None);

		setIcon(window, "res/icon.png");

		// Make window transparent in some places
		SetWindowLong(window.getSystemHandle(), GWL_STYLE, WS_POPUP | WS_VISIBLE);
		DwmExtendFrameIntoClientArea(window.getSystemHandle(), &margins);

		winBold.loadFromFile ("res/consBold.ttf");
		winReg.loadFromFile  ("res/consReg.ttf");

		sf::Text name        ("winXd", winBold, textSize);
		name.setFillColor    (textColor);
		name.setPosition     (sf::Vector2f(6, 1));

		sf::Text subName     (":mandelbrot", winReg, textSize);
		subName.setFillColor (textSubColor);
		subName.setPosition  (sf::Vector2f(47, 1));

		sf::Text info        ("Esc:close V:minimize", winReg, textSize);
		info.setFillColor    (textSubColor);
		info.setPosition     (sf::Vector2f(windowWidth + 2 * boarderSize - 165, 1));

		// Upper panel
		sf::RectangleShape windowFrame;
		setRectSettings(windowFrame, sf::Vector2f(windowWidth + 2 * boarderSize, upperPanelH), bodyColor, 0, 0);

		// Inner content
		sf::RectangleShape windowContent;
		setRectSettings(windowContent, sf::Vector2f(windowWidth, windowHeight), sf::Color(5, 17, 21, 255), boarderSize, upperPanelH);

		// left border
		sf::RectangleShape left;
		setRectSettings(left, sf::Vector2f(boarderSize, boarderSize + windowHeight), bodyColor, 0, upperPanelH);

		// right border
		sf::RectangleShape right;
		setRectSettings(right, sf::Vector2f(boarderSize, boarderSize + windowHeight), bodyColor, windowWidth + boarderSize, upperPanelH);

		// bottom border
		sf::RectangleShape bottom;
		setRectSettings(bottom, sf::Vector2f(windowWidth, boarderSize), bodyColor, boarderSize, upperPanelH + windowHeight);

		// Prepare set.
		u32* pixels = (u32*)calloc((size_t)windowHeight * (size_t)windowWidth, sizeof(u32));

		texture.create((u32)windowWidth, (u32)windowHeight);
		set.setTextureRect(sf::IntRect(0, 0, (int)windowWidth, (int)windowHeight));
		set.setPosition(sf::Vector2f(boarderSize, upperPanelH));
		set.setTexture(texture, false);
		
		while (window.isOpen())
		{
			getBehavior(window, event);

			// Draw window
			window.clear  (sf::Color::Transparent);
			window.draw   (windowFrame);
			window.draw   (windowContent);

			window.draw   (name);
			window.draw   (subName);
			window.draw   (info);

			mousePosition.getCoordinates(window);

			// Draw set
			renderSet     (pixels);
			texture.update((u8*)pixels);
			window.draw   (set);

			// Draw borders of window
			window.draw   (right);
			window.draw   (left);
			window.draw   (bottom);

			// Add fps
			fps.Renew     (fpsString);
			window.draw   (fpsString);

			window.display();
		}
	}
};

int main()
{
	winXdApp winXd;
	winXd.drawFrame(winXd.window);

	return 0;
}