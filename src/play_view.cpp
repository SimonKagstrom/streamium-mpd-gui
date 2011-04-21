#include "menu.hh"
#include "utils.hh"
#include "dialogue_box.hh"

class PlayView : public GuiView
{
public:
	PlayView() : GuiView()
	{
		panic_if(!Gui::gui->default_font,
				"Theme does not seem correctly loaded\n");

		m_duration_color = SDL_MapRGB(SDL_GetVideoInfo()->vfmt, 255,255,255);
		m_font_h = Gui::gui->default_font->getHeight("A");
	}

	virtual ~PlayView()
	{
	}

	void runLogic()
	{
	}

	void pushEvent(event_t ev)
	{
		printf("EV: %08x\n", ev);

		switch (ev) {
		case KEY_PAGEDOWN:
			MpdStatus::getInstance()->next();
			break;
		case KEY_PAGEUP:
			MpdStatus::getInstance()->prev();
			break;
		case KEY_SELECT:
			MpdStatus::getInstance()->playPause();
			break;
		default:
			break;
		}
	}

	void viewPushCallback()
	{
	}

	void draw(SDL_Surface *where)
	{
		SDL_Surface *playing = Gui::gui->pause;
		SDL_Rect dst;
		SDL_Rect duration_rect;
		MpdStatus *mpd_status = MpdStatus::getInstance();

		if (mpd_status->isPlaying())
			playing = Gui::gui->play;

		/* Blit play/pause*/
		dst = (SDL_Rect){320-40, 0,320,40};
		SDL_BlitSurface(playing, NULL, where, &dst);

		if (mpd_status->getQueueLength() == 0) {
			Gui::gui->default_font->draw(where,
					"No songs in play queue, go to", 10, 40 + (m_font_h + 5),
					310, 20, false);
			Gui::gui->default_font->draw(where,
					"menu to add or use the web browser.", 10, 40 + 2 * (m_font_h + 5),
					310, 20, false);
			return;
		}

		/* And the duration */
		int dur_w = 0;
		Song *current_song = mpd_status->getCurrentSong();
		dur_w = (mpd_status->getCurrentSongPos() * 280) / current_song->getDuration();

		duration_rect = (SDL_Rect){10, 5, dur_w, 10};
		SDL_FillRect(where, &duration_rect, m_duration_color);

		Song **queue_songs = mpd_status->getCurrentQueueSongs();
		for (int i = 0; i < mpd_status->getNumberOfQueueSongs(); i++) {
			bool bold = false;

			if (queue_songs[i] == current_song)
				bold = true;

			Gui::gui->default_font->draw(where,
					queue_songs[i]->getArtist(), 10, 40 + (i * 2) * (m_font_h + 5),
					310, 20, bold);
			Gui::gui->default_font->draw(where,
					queue_songs[i]->getTitle(), 10, 40 + (i * 2 + 1) * (m_font_h + 5),
					310, 20, bold);
		}
	}

protected:

	int m_font_h;

	uint32_t m_duration_color;
};
