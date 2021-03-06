#include "menu.hh"
#include "utils.hh"
#include "dialogue_box.hh"

const char *main_menu_messages[14] = {
        /*00*/          "Function",
        /*01*/          "^|HD/Spotify|CD|FM radio",
        /*02*/          "Browse files",
        /*04*/          "Browse playlists",
        /*05*/          " ",
        /*06*/          " ",
        /*07*/          "Settings",
        /*08*/          "^|Setup spotify (NYI!)",
        NULL
};


class MainView;
class MainMenu : public Menu
{
	friend class MainView;

public:
	MainMenu(Font *font) : Menu(font)
	{
		this->setText(main_menu_messages);
	}

	virtual void selectCallback(int which)
	{
		switch (which)
		{
		case 0:
			if (this->p_submenus[0].sel == 1) { // CD
				mpd_send_clear(Gui::gui->mpd_conn);
				mpd_send_add(Gui::gui->mpd_conn, "cdda://");
				mpd_response_finish(Gui::gui->mpd_conn);
			} else if (this->p_submenus[0].sel == 2) // Radio
				Gui::gui->status_bar->queueMessage("NYI!");
			break;

		case 2:
			Gui::gui->pushView((GuiView*)Gui::gui->fv);
			break;
		case 3:
			Gui::gui->pushView((GuiView*)Gui::gui->plv);
			break;
		}
	}

	virtual void hoverCallback(int which)
	{
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->popView();
	}

private:
};


class MainView : public GuiView
{
public:
	MainView() : GuiView()
	{
		panic_if(!Gui::gui->default_font,
				"Theme does not seem correctly loaded\n");

		this->menu = new MainMenu(Gui::gui->default_font);
	}

	virtual ~MainView()
	{
		delete this->menu;
	}

	void runLogic()
	{
		this->menu->runLogic();
	}

	void pushEvent(event_t ev)
	{
		this->menu->pushEvent(ev);
	}

	void viewPushCallback()
	{
		this->menu->selectOne(0);
	}

	void draw(SDL_Surface *where)
	{
		 SDL_Rect dst;

		 /* Blit the backgrounds */
		 dst = (SDL_Rect){10,10,300,220};
		 SDL_BlitSurface(Gui::gui->main_menu_bg, NULL, where, &dst);

		 this->menu->draw(where, 20, 20, 280, 200);
	}

protected:
	MainMenu *menu;
};
