// My module
#include "winXd.hpp"
#include "mandelbrot.hpp"

#include <windows.h>

int main()
{
	// To disable console we use -mwindows flat and mingw stops draw console.
	winXd frame;

	frame.create("Mandelbrot set");

	return 0;
}