#include "menu.hh"
#include "utils.hh"
#include "dialogue_box.hh"

class Song
{
public:
	Song()
	{
		set(~0, "", "");
	}

	void set(unsigned id, const char *artist, const char *title)
	{
		m_id = id;

		snprintf(m_artist, sizeof(m_artist),
				"%s", artist);
		snprintf(m_title, sizeof(m_title),
				"   %s", title);
	}

	unsigned getId()
	{
		return m_id;
	}

	const char *getArtist()
	{
		return m_artist;
	}

	const char *getTitle()
	{
		return m_title;
	}

private:
	unsigned m_id;
	char m_artist[255];
	char m_title[255];
};


class PlayView : public GuiView, public TimeoutHandler
{
public:
	PlayView() : GuiView(), TimeoutHandler()
	{
		panic_if(!Gui::gui->default_font,
				"Theme does not seem correctly loaded\n");

		this->m_playing = false;
		this->m_current_id = ~0;
		this->m_n_songs = 4;

		for (unsigned i = 0; i < this->m_n_songs; i++)
			this->m_songs[i] = new Song();

		/* Update this directly */
		this->timeoutCallback();
	}

	virtual ~PlayView()
	{
	}

	void runLogic()
	{
	}

	void pushEvent(event_t ev)
	{
		struct mpd_status *status = mpd_status_begin();

		printf("EV: %08x\n", ev);

		switch (ev) {
		case KEY_PAGEDOWN:
		{
			if (!status)
				break;

			unsigned last_id = mpd_status_get_queue_length(status);

			/* Stop if we're at the last entry */
			if (m_current_id == last_id) {
				printf("Last entry, just stop\n");
				mpd_run_stop(Gui::gui->mpd_conn);
				break;
			}

			mpd_run_next(Gui::gui->mpd_conn);
			TimerController::controller->arm(this);
			break;
		}
		case KEY_PAGEUP:
			mpd_run_previous(Gui::gui->mpd_conn);
			TimerController::controller->arm(this);
			break;
		case KEY_SELECT:
		{
			if (!status)
				break;

			enum mpd_state state = mpd_status_get_state(status);
			if (state == MPD_STATE_STOP)
				mpd_run_play(Gui::gui->mpd_conn);
			else
				mpd_run_toggle_pause(Gui::gui->mpd_conn);

			break;
		}
		default:
			break;
		}

		if (!status)
			mpd_status_free(status);
	}

	void viewPushCallback()
	{
	}

	void draw(SDL_Surface *where)
	{
		SDL_Surface *playing = Gui::gui->pause;
		SDL_Rect dst;

		if (this->m_playing)
			playing = Gui::gui->play;

		/* Blit play/pause*/
		dst = (SDL_Rect){320-40, 0,320,40};
		SDL_BlitSurface(playing, NULL, where, &dst);

		for (unsigned i = 0; i < this->m_n_songs; i++) {
			bool bold = false;

			if (m_songs[i]->getId() == m_current_id)
				bold = true;

			Gui::gui->default_font->draw(where,
					m_songs[i]->getArtist(), 10, 40 + i * 40,
					310, 20, bold);
			Gui::gui->default_font->draw(where,
					m_songs[i]->getTitle(), 10, 60  + i * 40,
					310, 20, bold);
		}
	}

protected:
	void timeoutCallback()
	{
		struct mpd_song *song;
		struct mpd_status *status;

		/* Retrigger */
		TimerController::controller->arm(this, 700);

		status = mpd_run_status(Gui::gui->mpd_conn);
		if (!status)
			return;

		enum mpd_state state = mpd_status_get_state(status);
		unsigned last_id = mpd_status_get_queue_length(status);

		this->m_playing = (state == MPD_STATE_PLAY);
		mpd_status_free(status);

		song = mpd_run_current_song(Gui::gui->mpd_conn);
		if (!song)
			return;

		m_current_id = mpd_song_get_id(song);
		mpd_song_free(song);

		unsigned first = m_current_id - m_n_songs / 2 + 1;
		unsigned last = m_current_id + m_n_songs / 2 + 1;

		if (m_current_id < 2) {
			first = 0;
			last = 4;
		}
		if (last > last_id)
			last = last_id;

		for (unsigned i = 0; i < m_n_songs; i++)
				m_songs[i]->set(i, "", "");

		for (unsigned i = first; i < last; i++) {
			const char *artist;
			const char *title;
			struct mpd_song *cur = mpd_run_get_queue_song_id(Gui::gui->mpd_conn, i);

			if (!cur)
				break;

			artist = mpd_song_get_tag(cur, MPD_TAG_ARTIST, 0);
			title = mpd_song_get_tag(cur, MPD_TAG_TITLE, 0);

			if (artist && title)
				m_songs[i - first]->set(i, artist, title);

			mpd_song_free(cur);
		}

		mpd_response_finish(Gui::gui->mpd_conn);
	}

	bool m_playing;
	unsigned m_current_id;

	Song *m_songs[4];
	unsigned m_n_songs;
};
