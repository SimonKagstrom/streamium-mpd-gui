#include "menu.hh"
#include "utils.hh"
#include "dialogue_box.hh"

const char *main_menu_messages[14] = {
        /*00*/          "Function",
        /*01*/          "^|HD|CD|FM radio",
        /*02*/          "Settings",
        /*03*/          "^|Setup spotify",
        NULL
};




class ExitListener : public DialogueListener
{
	void escapeCallback(DialogueBox *which, int selected)
	{
		delete this;
	}

	void selectCallback(DialogueBox *which, int selected)
	{
		/* Cancel? */
		if (selected != 1)
		{
			Gui::gui->exitMenu();
			exit(1);
		}

		delete this;
	}
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
		case 0: /* Insert disc */

		case 1: /* Exit */
			break;
		}
	}

	virtual void hoverCallback(int which)
	{
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->exitMenu();
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

	~MainView()
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
