// main libs
#include <stdio.h>
#include <stdint.h>
#include <SFML/Graphics.hpp>

// For special mode of window
#ifdef _WIN32

#include <Dwmapi.h>
#pragma comment (lib, "Dwmapi.lib")

// For visual studio to not scream
#define snprintf sprintf_s

#endif

// For _m256
#include <immintrin.h>

typedef  __m256   sse_t;
typedef  __m512  sse_t512;
typedef  uint8_t     u8;
typedef uint32_t    u32;
typedef  int64_t    s64;

const float  windowWidth   = 1200;
const float  windowHeight  =  800;
const s64    screenWidth   = 1920;
const s64    screenHeight  = 1080;
const float  R2            =   20;

const int    mouseMovementSensivity  = 25000;

const u32    MAX_CHECK               = 32;
const u32    BLACK_COLOR_PIXEL       = 0xFF000000;
const float  MOUSE_WHEEL_SENSITIVITY = 0.05;

const sse_t FULL_COLORED = _mm256_cmp_ps(_mm256_set1_ps(0), _mm256_set1_ps(0), _CMP_EQ_OQ);
const sse_t MUL_2        = _mm256_set1_ps(2.0);
const sse_t R_NEED       = _mm256_set1_ps(100);

const uint16_t FULL_COLORED512 = 0xFFFF;
const sse_t512 MUL_512         = _mm512_set1_ps(2.0);
const sse_t512 R_NEED512       = _mm512_set1_ps(100);


struct fpsCounter
{
	sf::Clock  clock = {};
	sf::Time   time  = clock.getElapsedTime();

	float time_prev     = time.asSeconds();
	float time_now      = 0;
	float time_last_out = 0;

	const float FPS_DELAY = 0.15;

	char str[32] = "fps:";

	void Renew(sf::Text& fpsLabel)
	{
		time     = clock.getElapsedTime  ();
		time_now = time.asSeconds        ();

		if (time_now - time_last_out > FPS_DELAY)
		{
			float cur_fps = (float)(1 / (time_now - time_prev));

			snprintf(str + 4, 26, "%.2lf\n", (float)(1 / (time_now - time_prev)));
			fpsLabel.setString(str);

			printf("%s\n", str);

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

		rel_x_coef = ((float)(-1 * (float)position.x) / mouseMovementSensivity);
		rel_y_coef = ((float)(-1 * (float)position.y) / mouseMovementSensivity);
	}
};

class winXdApp
{
private:
	#ifdef _WIN32
	MARGINS margins = { margins.cxLeftWidth = -1 };
	#endif

	sf::Event        event;
	sf::Vector2i     cur_pos;
	sf::Mouse        mouse;
	sf::Font         winBold;
	sf::Font         winReg;
	sf::Image        icon;
	sf::Text         fpsString;

	sf::Texture      texture;
	sf::Sprite       set;

	sf::Color        textColor    = { 0x00, 0x00, 0x00, 0xFF          };
	sf::Color        textSubColor = { 0x00, 0x00, 0x00, 0xFF * 8 / 10 };
	sf::Color        bodyColor    = { 0xFF, 0xFF, 0xFF, 0xFF          };  // { 8, 28, 35, 255 };

	coordinates      mousePosition;

	float scale   = 0.23;
	float x_shift = -0.3;
	float y_shift =    0;
	bool  isPressed = false;

	const int boarderSize =  3;
	const int textSize    = 14;
	const int upperPanelH = 20;

	void renderSet(unsigned int* pixels) {

		size_t pixels_pos = 0;

		for (size_t y = 0; y < windowHeight; y++) {

			float Im_num = ((float)y / windowWidth - 0.5 * windowHeight / windowWidth) / scale + y_shift;

			for (size_t x = 0; x < windowWidth; x += 8) {

				sse_t Re = _mm256_set_ps(0, 1, 2, 3, 4, 5, 6, 7);

				Re = _mm256_add_ps(Re, _mm256_set1_ps((float)x   ));
				Re = _mm256_div_ps(Re, _mm256_set1_ps(windowWidth));
				Re = _mm256_sub_ps(Re, _mm256_set1_ps(0.5        ));
				Re = _mm256_div_ps(Re, _mm256_set1_ps(scale      ));
				Re = _mm256_add_ps(Re, _mm256_set1_ps(x_shift    ));

				sse_t Re0 = Re;

				// sse move
				sse_t Im = _mm256_set1_ps(Im_num);
				sse_t Im0 = Im;

				sse_t colored = _mm256_set1_ps(0);

				// black screen
				for (int i_pixel = 0; i_pixel < 8; i_pixel++)
					*(pixels + pixels_pos + i_pixel) = BLACK_COLOR_PIXEL;

				sse_t Re_2 = _mm256_mul_ps(Re, Re);
				sse_t Im_2 = _mm256_mul_ps(Im, Im);

				for (size_t n = 0; n < MAX_CHECK && !_mm256_testc_ps(colored, FULL_COLORED); n++) {

					Im = _mm256_fmadd_ps(MUL_2, _mm256_mul_ps(Re, Im), Im0);
					Re = _mm256_add_ps(_mm256_sub_ps(Re_2, Im_2), Re0);

					Re_2 = _mm256_mul_ps(Re, Re);
					Im_2 = _mm256_mul_ps(Im, Im);

					sse_t cmp = _mm256_cmp_ps(_mm256_add_ps(Re_2, Im_2), R_NEED, _CMP_GT_OQ);

					cmp = _mm256_andnot_ps(colored, cmp);

					for (int i_cmp = 0; i_cmp < 8; i_cmp++) {
						if (*((int *)&cmp + i_cmp)) {
							pixels[pixels_pos + ((s64)7 - i_cmp)] =
								(u32)(BLACK_COLOR_PIXEL + 0x00FF0000 - (0x0010000 * (n)+01000000 * 5 * n));
						}
					}

					colored = _mm256_or_ps(colored, cmp);
				}

				pixels_pos += 8;
			}

		}

		return;
	}

	void renderSet512(unsigned int* pixels) {

		size_t pixels_pos = 0;

		for (size_t y = 0; y < windowHeight; y++) {

			float Im_num = ((float)y / windowWidth - 0.5 * windowHeight / windowWidth) / scale + y_shift;

			for (size_t x = 0; x < windowWidth; x += 16) {

				sse_t512 Re = _mm512_set_ps(0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15);

				Re = _mm512_add_ps(Re, _mm512_set1_ps((float)x   ));
				Re = _mm512_div_ps(Re, _mm512_set1_ps(windowWidth));
				Re = _mm512_sub_ps(Re, _mm512_set1_ps(0.5        ));
				Re = _mm512_div_ps(Re, _mm512_set1_ps(scale      ));
				Re = _mm512_add_ps(Re, _mm512_set1_ps(x_shift    ));

				sse_t512 Re0 = Re;

				// sse move
				sse_t512 Im = _mm512_set1_ps(Im_num);
				sse_t512 Im0 = Im;

				uint16_t colored = 0;

				// black screen
				for (int i_pixel = 0; i_pixel < 16; i_pixel++)
					*(pixels + pixels_pos + i_pixel) = BLACK_COLOR_PIXEL;

				sse_t512 Re_2 = _mm512_mul_ps(Re, Re);
				sse_t512 Im_2 = _mm512_mul_ps(Im, Im);

				for (size_t n = 0; n < MAX_CHECK && colored != FULL_COLORED512; n++) {

					Im = _mm512_fmadd_ps(MUL_512, _mm512_mul_ps(Re, Im), Im0);
					Re = _mm512_add_ps(_mm512_sub_ps(Re_2, Im_2), Re0);

					Re_2 = _mm512_mul_ps(Re, Re);
					Im_2 = _mm512_mul_ps(Im, Im);

					uint16_t cmp = (uint16_t)_mm512_cmp_ps_mask(_mm512_add_ps(Re_2, Im_2), R_NEED512, _CMP_GT_OQ);


					cmp = (~colored) & cmp;
					// cmp = _mm512_andnot_ps(colored, cmp);

					for (int i_cmp = 0; i_cmp < 16; i_cmp++) {
						if (1 & (cmp >> (i_cmp))) {
							pixels[pixels_pos + ((s64)15 - i_cmp)] =
								(u32)(BLACK_COLOR_PIXEL + 0x00FF0000 - (0x0010000 * (n)+01000000 * 5 * n));
						}
					}

					colored |= cmp;
				}

				pixels_pos += 16;
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

				#ifdef _WIN32
				if (event.key.code == sf::Keyboard::V)
				{
					SendNotifyMessageW(
						window.getSystemHandle(),
						WM_SYSCOMMAND,
						SC_MINIMIZE,
						0);
				}
				#endif
			}

			if (event.type == sf::Event::MouseWheelMoved) {
				scale *= 1 + MOUSE_WHEEL_SENSITIVITY * event.mouseWheel.delta;

				x_shift -= (float)mousePosition.rel_x_coef * event.mouseWheel.delta / scale;
				y_shift -= (float)mousePosition.rel_y_coef * event.mouseWheel.delta / scale;
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

	bool setIcon(sf::RenderWindow& window, const char* iconName)
	{
		icon.loadFromFile(iconName);
		window.setIcon(128, 128, icon.getPixelsPtr());
	}

public:
	sf::RenderWindow window;

	void drawFrame(sf::RenderWindow& window)
	{

		fpsCounter fps;
		fpsString.setPosition       (sf::Vector2f(144, 1));
		fpsString.setFillColor      (textSubColor);
		fpsString.setFont           (winReg);
		fpsString.setCharacterSize  (textSize);

		window.create(
			sf::VideoMode(windowWidth + 2 * boarderSize, upperPanelH + windowHeight + boarderSize),
			"winXd",
			sf::Style::None);

		// TODO: check for errors
		// if (setIcon(window, "res/icon.png"))

		// Make window transparent in some places
		#ifdef _WIN32
		SetWindowLong(window.getSystemHandle(), GWL_STYLE, WS_POPUP | WS_VISIBLE);
		DwmExtendFrameIntoClientArea(window.getSystemHandle(), &margins);
		#endif

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
		setRectSettings(
			windowFrame,
			sf::Vector2f(windowWidth + 2 * boarderSize, upperPanelH),
			bodyColor,
			0, 0);

		// Inner content
		sf::RectangleShape windowContent;
		setRectSettings(
			windowContent,
			sf::Vector2f(windowWidth, windowHeight),
			sf::Color(5, 17, 21, 255),
			boarderSize, upperPanelH);

		// left border
		sf::RectangleShape left;
		setRectSettings(
			left,
			sf::Vector2f(boarderSize, boarderSize + windowHeight),
			bodyColor,
			0, upperPanelH);

		// right border
		sf::RectangleShape right;
		setRectSettings(
			right,
			sf::Vector2f(boarderSize, boarderSize + windowHeight),
			bodyColor,
			windowWidth + boarderSize, upperPanelH);

		// bottom border
		sf::RectangleShape bottom;
		setRectSettings(
			bottom,
			sf::Vector2f(windowWidth, boarderSize),
			bodyColor,
			boarderSize, upperPanelH + windowHeight);

		// Prepare set.
		u32* pixels = (u32*)calloc((size_t)windowHeight * (size_t)windowWidth, sizeof(u32));

		texture.create     ((u32)windowWidth, (u32)windowHeight);
		set.setTextureRect (sf::IntRect(0, 0, (int)windowWidth, (int)windowHeight));
		set.setPosition    (sf::Vector2f(boarderSize, upperPanelH));
		set.setTexture     (texture, false);
		
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
			renderSet512  (pixels);
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
