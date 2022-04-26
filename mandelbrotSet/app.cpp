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

const float  WINDOW_WIDTH   =  888;
const float  WINDOW_HEIGHT  =  596;
const s64    SCREEN_WIDTH   = 1920;
const s64    SCREEN_HEIGHT  = 1080;
const double R2             =   10;

const u32    MAX_CHECK               = 64;
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
		sf::Vector2f position = (sf::Vector2f)sf::Mouse::getPosition();

		position.x = (position.x - SCREEN_WIDTH  / 2) - (window.getPosition().x - WINDOW_WIDTH  / 2 - 70);
		position.y = (position.y - SCREEN_HEIGHT / 2) - (window.getPosition().y - WINDOW_HEIGHT / 2 + 60);

		// printf("%f %f\n", position.x, position.y);

		rel_x_coef = ((float)(-1 * (int)position.x) / 16000);
		rel_y_coef = ((float)(-1 * (int)position.y) / 16000);
	}
};

class winXdApp
{
private:
	MARGINS          margins = {};

	sf::Event        event;
	sf::Vector2i     cur_pos;
	sf::Mouse        mouse;
	sf::Font         winBold;
	sf::Font         winReg;
	sf::Image        icon;
	sf::Text         fpsString;

	sf::Texture      texture;
	sf::Sprite       set;
	sf::Color        textColor    = { 0x00, 0x00, 0x00, 0xFF };
	sf::Color        textSubColor = { 0x00, 0x00, 0x00, 0xFF * 0.8 };
	sf::Color        bodyColor    = { 0xFF, 0xFF, 0xFF, 0xFF };  // { 8, 28, 35, 255 };

	coordinates mousePosition;

	double scale = 0.23;
	double x_shift = -0.3, y_shift = 0;

	bool isPressed = false;

	void renderSet(unsigned int* pixels) {

		size_t pixels_pos = 0;

		for (size_t y = 0; y < WINDOW_HEIGHT; y++) {

			double Im_num = ((double)y / WINDOW_WIDTH - 0.5 * WINDOW_HEIGHT / WINDOW_WIDTH) / scale + y_shift;

			for (size_t x = 0; x < WINDOW_WIDTH; x += 4) {

				sse_t Re = _mm256_set_pd(0, 1, 2, 3);

				Re = _mm256_add_pd(Re, _mm256_set1_pd((double)x));
				Re = _mm256_div_pd(Re, _mm256_set1_pd(WINDOW_WIDTH));
				Re = _mm256_sub_pd(Re, _mm256_set1_pd(0.5));
				Re = _mm256_div_pd(Re, _mm256_set1_pd(scale));
				Re = _mm256_add_pd(Re, _mm256_set1_pd(x_shift));

				sse_t Re0 = Re;
				// It's SSE-function move
				sse_t Im = _mm256_set1_pd(Im_num);
				sse_t Im0 = Im;

				sse_t colored = _mm256_set1_pd(0);

				// All BLACK_COLOR_PIXEL
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
		u32* pixels = (u32*)calloc((size_t)WINDOW_HEIGHT * (size_t)WINDOW_WIDTH, sizeof(u32));
		margins = {};
		margins.cxLeftWidth = -1;

		fpsCounter fps;
		fpsString.setPosition(sf::Vector2f(144, 1));
		fpsString.setFillColor(textSubColor);
		fpsString.setFont(winReg);
		fpsString.setCharacterSize(14);

		window.create(sf::VideoMode(888, 620), "winXd", sf::Style::None);
		setIcon(window, "res/icon.png");
		SetWindowLong(window.getSystemHandle(), GWL_STYLE, WS_POPUP | WS_VISIBLE);
		DwmExtendFrameIntoClientArea(window.getSystemHandle(), &margins);

		winBold.loadFromFile("res/consBold.ttf");
		winReg.loadFromFile("res/consReg.ttf");

		sf::Text name("winXd", winBold, 14);
		name.setFillColor(textColor);
		name.setPosition(sf::Vector2f(6, 1));

		sf::Text subName(":mandelbrot", winReg, 14);
		subName.setFillColor(textSubColor);
		subName.setPosition(sf::Vector2f(47, 1));

		sf::Text info("Esc:close V:minimize", winReg, 14);
		info.setFillColor(textSubColor);
		info.setPosition(sf::Vector2f(723, 1));

		// Upper panel
		sf::RectangleShape windowFrame;
		setRectSettings(windowFrame, sf::Vector2f(890, 20),	bodyColor,	0, 0);

		// Inner content
		sf::RectangleShape windowContent;
		setRectSettings(windowContent, sf::Vector2f(884, 597), sf::Color(5, 17, 21, 255), 3, 20);

		// left border
		sf::RectangleShape left;
		setRectSettings(left, sf::Vector2f(3, 600), bodyColor, 0, 20);

		// right border
		sf::RectangleShape right;
		setRectSettings(right, sf::Vector2f(3, 600), bodyColor, 885, 20);

		// bottom border
		sf::RectangleShape bottom;
		setRectSettings(bottom, sf::Vector2f(884, 3), bodyColor, 3, 617);

		texture.create((u32)WINDOW_WIDTH, (u32)WINDOW_HEIGHT);
		set.setTextureRect(sf::IntRect(0, 0, (int)WINDOW_WIDTH, (int)WINDOW_HEIGHT));
		set.setPosition(sf::Vector2f(3, 20));
		set.setTexture(texture, false);
		
		while (window.isOpen())
		{
			getBehavior(window, event);

			window.clear(sf::Color::Transparent);
			window.draw(windowFrame);
			window.draw(windowContent);

			window.draw(name);
			window.draw(subName);
			window.draw(info);

			mousePosition.getCoordinates(window);
			renderSet(pixels);
			texture.update((u8*)pixels);
			window.draw(set);

			window.draw(right);
			window.draw(left);
			window.draw(bottom);

			fps.Renew(fpsString);
			window.draw(fpsString);

			window.display();
		}
	}
};

int main()
{
	printf("winXd\n");
	winXdApp winXd;
	winXd.drawFrame(winXd.window);

	return 0;
}