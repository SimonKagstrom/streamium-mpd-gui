#ifndef __GUI_HH__
#define __GUI_HH__

#include <SDL.h>

#include <mpd/client.h>
#include <mpd/status.h>
#include <mpd/song.h>
#include <mpd/player.h>
#include <mpd/entity.h>
#include <mpd/search.h>
#include <mpd/tag.h>

#include "timer.hh"
#include "menu.hh"
#include "font.hh"


class DialogueBox;
class StatusBar;
class GuiView;

class MainView;
class PlayView;

class Gui
{
public:
	Gui();

	~Gui();

	bool setTheme(const char *path);

	void activate();

	void runLogic(void);

	void pushEvent(event_t ev);

	void pushEvent(SDL_Event *ev);

	void draw(SDL_Surface *where);

	void pushView(GuiView *view);

	void pushDialogueBox(DialogueBox *dlg);

	DialogueBox *popDialogueBox();

	GuiView *popView();

	GuiView *peekView()
	{
		if (!this->views)
			return NULL;
		return this->views[this->n_views-1];
	}

	void exitMenu();

	/* These are private, keep off! */
	const char *getThemePath(const char *dir, const char *what);

	SDL_Surface *loadThemeImage(const char *dir, const char *what);

	Font *loadThemeFont(const char *dir, const char *what, int size);

	bool is_active;
	Menu *focus; /* Where the focus goes */
	Menu *main_menu;
	SDL_Surface *screenshot;

	SDL_Surface *background;
	SDL_Surface *main_menu_bg;
	SDL_Surface *status_bar_bg;
	SDL_Surface *dialogue_bg;
	SDL_Surface *play;
	SDL_Surface *pause;

	SDL_Surface *bg_left, *bg_right, *bg_middle,
		*bg_submenu_left, *bg_submenu_right, *bg_submenu_middle;

	Font *default_font;
	Font *small_font;
	TimerController *controller;

    MainView *mv;
    PlayView *pv;

    const char *theme_base_path;
    const char *tmp_path;

	/* Handled specially */
	DialogueBox *dlg;
	StatusBar *status_bar;

	GuiView **views;
	int n_views;

	bool m_needs_redraw;

	/* MPD stuff */
	struct mpd_connection *mpd_conn;

	/* Singleton */
	static void init();
	static Gui *gui;
};

#endif /* GUI_HH */
