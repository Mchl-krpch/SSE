#include <SDL.h>
#undef main

#include <stdio.h>
#include <windows.h>
#include <time.h>

#include "compose.h"

int main(int argc, char **argv)
{
	 // Check errors.
	if (argc < 3)
	{
		crossPlatformMessage("undefined path to images", MSG_NOT_ENOUGH_PICTURES);
		return 1;
	}

	composePicturesFrame *frame = frameCtor(argv[1], argv[2]);

	if (frame->renderer == NULL)
	{
		crossPlatformMessage("Bad created renderer in SDL-library", MSG_BAD_RENDERER);
		return 1;
	}

	 // Write greetings.
	system("cls");
	printf(GREETINGS);

	execute(frame);

	free(frame);

	return 0;
}