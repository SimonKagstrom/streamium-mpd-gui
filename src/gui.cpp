#include <SDL_image.h>
#include <SDL_ttf.h>

#include <utils.hh>

#include "menu.hh"
#include "status_bar.hh"
#include "gui.hh"
#include "sdl_ttf_font.hh"

extern SDL_Surface *screen;

#define THEME_ROOT_PATH "themes"
#define METADATA_ROOT_PATH "metadata"
#define GAME_ROOT_PATH "images"
#define TMP_ROOT_PATH "tmp"
#define SAVE_GAME_ROOT_PATH "saves"

static const char *get_theme_path(const char *dir, const char *what)
{
	static char buf[255];

	memset(buf, 0, sizeof(buf));
	snprintf(buf, 254, "%s/%s/%s",
			THEME_ROOT_PATH, dir, what);

	return buf;
}

/* These are a bit of special cases... */
#include "main_menu.cpp"
#include "play_view.cpp"

GuiView::GuiView()
{
}

GuiView::~GuiView()
{
}

void GuiView::updateTheme()
{
}

void GuiView::viewPushCallback()
{
}

void GuiView::viewPopCallback()
{
}

Gui::Gui()
{
	this->is_active = false;

	this->focus = NULL;
	this->screenshot = NULL;

	this->bg_left = NULL;
	this->bg_middle = NULL;
	this->bg_right = NULL;
	this->bg_submenu_left = NULL;
	this->bg_submenu_middle = NULL;
	this->bg_submenu_right = NULL;
	this->background = NULL;
	this->main_menu_bg = NULL;
	this->status_bar_bg = NULL;
	this->default_font = NULL;
	this->dialogue_bg = NULL;
	this->small_font = NULL;
	this->play = NULL;
	this->pause = NULL;

	this->n_views = 0;
	this->views = NULL;

	this->theme_base_path = THEME_ROOT_PATH;
	this->tmp_path = TMP_ROOT_PATH;

	this->dlg = NULL;
	this->mv = NULL;

	this->mpd_conn = NULL;
}

Gui::~Gui()
{
}

bool Gui::setTheme(const char *path)
{
	this->bg_left = this->loadThemeImage(path, "bg_left.png");
	this->bg_middle = this->loadThemeImage(path, "bg_middle.png");
	this->bg_right = this->loadThemeImage(path, "bg_right.png");
	this->bg_submenu_left = this->loadThemeImage(path, "bg_submenu_left.png");
	this->bg_submenu_middle = this->loadThemeImage(path, "bg_submenu_middle.png");
	this->bg_submenu_right = this->loadThemeImage(path, "bg_submenu_right.png");

	this->background = this->loadThemeImage(path, "background.png");
	this->main_menu_bg = this->loadThemeImage(path, "main_menu_bg.png");
	this->status_bar_bg = this->loadThemeImage(path, "status_bar.png");
	this->dialogue_bg = this->loadThemeImage(path, "dialogue_box.png");
	this->play = this->loadThemeImage(path, "play.png");
	this->pause = this->loadThemeImage(path, "pause.png");

	this->default_font = this->loadThemeFont(path, "font.ttf", 12);
	this->small_font = this->loadThemeFont(path, "font.ttf", 10);

	this->mpd_conn = mpd_connection_new(NULL, 0, 30000);
	panic_if (this->mpd_conn == NULL,
			"Out of memory");
	panic_if (mpd_connection_get_error(this->mpd_conn) != MPD_ERROR_SUCCESS,
			"MPD conn error: %s", mpd_connection_get_error_message(this->mpd_conn));

	/* Create the views */
	this->status_bar = new StatusBar();

	this->mv = new MainView();
	this->pv = new PlayView();

	return true;
}

void Gui::runLogic(void)
{
	GuiView *cur_view = this->peekView();

	this->status_bar->runLogic();
	TimerController::controller->tick();

	if (!this->is_active || !cur_view)
		return;
	if (this->dlg)
		this->dlg->runLogic();
	else
		cur_view->runLogic();
}

void Gui::pushView(GuiView *view)
{
	int cur = this->n_views;

	this->n_views++;
	this->views = (GuiView**)xrealloc(this->views,
			sizeof(GuiView*) * this->n_views);
	this->views[cur] = view;
	view->viewPushCallback();
}

void Gui::pushDialogueBox(DialogueBox *dlg)
{
	this->dlg = dlg;
}

DialogueBox *Gui::popDialogueBox()
{
	DialogueBox *out = this->dlg;
	this->dlg = NULL;

	return out;
}

GuiView *Gui::popView()
{
	GuiView *cur = this->peekView();

	if (!cur)
		return NULL;
	cur->viewPopCallback();

	this->n_views--;
	if (this->n_views <= 0)
	{
		free(this->views);
		this->views = NULL;
		this->n_views = 0;
		/* Deactivate when no views are left */
		this->is_active = false;

		return NULL;
	}

	this->views = (GuiView**)xrealloc(this->views,
			sizeof(GuiView*) * this->n_views);
	if (this->peekView())
		this->peekView()->viewPushCallback();

	return cur;
}

void Gui::exitMenu()
{
	/* Pop all views */
	while (this->popView())
		;
}


void Gui::pushEvent(event_t ev)
{
	GuiView *cur_view = this->peekView();

	if (ev == KEY_ENTER_MENU)
	{
		this->activate();
		return;
	}

	if (!this->is_active || !cur_view)
		return;

	if (this->dlg)
		this->dlg->pushEvent(ev);
	else
		cur_view->pushEvent(ev);
}

void Gui::pushEvent(SDL_Event *ev)
{
	switch(ev->type)
	{
	case SDL_KEYDOWN:
		switch (ev->key.keysym.sym)
		{
		case SDLK_UP:
			this->pushEvent(KEY_UP);
			break;
		case SDLK_DOWN:
			this->pushEvent(KEY_DOWN);
			break;
		case SDLK_LEFT:
			this->pushEvent(KEY_LEFT);
			break;
		case SDLK_RIGHT:
			this->pushEvent(KEY_RIGHT);
			break;
		case SDLK_PAGEDOWN:
			this->pushEvent(KEY_PAGEDOWN);
			break;
		case SDLK_PAGEUP:
			this->pushEvent(KEY_PAGEUP);
			break;
		case SDLK_RETURN:
		case SDLK_SPACE:
			this->pushEvent(KEY_SELECT);
			break;
		case SDLK_HOME:
		case SDLK_ESCAPE:
			this->pushEvent(KEY_ESCAPE);
			break;
		default:
			break;
		}
		default:
			break;
	case SDL_QUIT:
		exit(1);
		break;
	}
}

void Gui::draw(SDL_Surface *where)
{
	GuiView *cur_view = this->peekView();

	if (!this->is_active || !cur_view)
	{
		this->status_bar->draw(where);
		return;
	}

	 SDL_BlitSurface(this->background, NULL, where, NULL);
	 cur_view->draw(where);
	 if (this->dlg)
		 this->dlg->draw(where);
	 this->status_bar->draw(where);
}

void Gui::activate()
{
	this->is_active = true;

	this->pushView(this->pv);
}

SDL_Surface *Gui::loadThemeImage(const char *dir, const char *what)
{
	SDL_Surface *img = IMG_Load(get_theme_path(dir, what));
	SDL_Surface *out;

	if (!img)
		return NULL;
	out = SDL_DisplayFormatAlpha(img);
	SDL_FreeSurface(img);

	return out;
}

Font *Gui::loadThemeFont(const char *dir, const char *what, int size)
{
	TTF_Font *fnt;

	fnt = read_and_alloc_font(get_theme_path(dir, what), size);
	if (!fnt)
		return NULL;

	return new Font_TTF(fnt, 255,255,255);
}



/* The singleton/factory stuff */
Gui *Gui::gui;
void Gui::init()
{
	Gui::gui = new Gui();

	if (!Gui::gui->setTheme("default"))
	{
		/* Set the default theme */
		panic_if (!Gui::gui->setTheme("default"),
				"Setting default theme failed\n");
	}

	Gui::gui->status_bar->queueMessage("Press Menu key for the menu!");
}
