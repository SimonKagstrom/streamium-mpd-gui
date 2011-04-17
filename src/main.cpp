#include <SDL_image.h>
#include <SDL_ttf.h>

#include "gui.hh"
#include "utils.hh"
#include "timer.hh"

#define MS_PER_FRAME 100
#define TICKS_PER_MS (MS_PER_FRAME / 2)

static void run(SDL_Surface *screen)
{
	Uint32 last_frame = SDL_GetTicks();

	while(1)
	{
		SDL_Event ev;
		Uint32 now;

		if (!Gui::gui->is_active)
			break;

		while (SDL_PollEvent(&ev)) {
			if (ev.type == SDL_QUIT)
				exit(1);

			Gui::gui->pushEvent(&ev);
		}
		Gui::gui->runLogic();
		Gui::gui->draw(screen);

		SDL_Flip(screen);

		now = SDL_GetTicks();
		if ( (now - last_frame) < MS_PER_FRAME)
			SDL_Delay( MS_PER_FRAME - (now - last_frame));

		last_frame = now;
	}
}

static SDL_Surface *init(void)
{
	struct SDL_Surface *screen;
	const SDL_VideoInfo *info;

	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

	info = SDL_GetVideoInfo();
	panic_if (!info,
			"Can't do GetVideoInfo()!\n");


	screen = SDL_SetVideoMode(320, 240, info->vfmt->BitsPerPixel, SDL_DOUBLEBUF);
	panic_if(!screen, "Cannot initialize video: %s\n", SDL_GetError());
	TTF_Init();

	TimerController::init(TICKS_PER_MS);
	Gui::init();
	Gui::gui->activate();

	return screen;
}

static void fini(void)
{
	delete Gui::gui;
}


int main(int argc, char *argv[])
{
	SDL_Surface *screen;

	screen = init();
	run(screen);
	fini();

	SDL_Quit();

	return 0;
}
