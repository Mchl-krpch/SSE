// My module
#ifdef _WIN32
#include <windows.h>
#endif

#include "winXd.hpp"
#include "mandelbrot.hpp"

int main()
{
	winXd frame;
	frame.create("Mandelbrot set");

	return 0;
}