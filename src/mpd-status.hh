#ifndef MPD_STATUS_HH_
#define MPD_STATUS_HH_

#include "timer.hh"
#include "gui.hh"

class Song
{
public:
	Song()
	{
		set(~0, "", "", 1);
	}

	void set(unsigned id, const char *artist, const char *title, int song_duration)
	{
		m_id = id;

		snprintf(m_artist, sizeof(m_artist),
				"%s", artist);
		snprintf(m_title, sizeof(m_title),
				"   %s", title);

		if (song_duration == 0)
			song_duration = 1;

		m_song_duration = song_duration;
	}

	int getDuration()
	{
		return m_song_duration;
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

	int m_song_duration;
};


class MpdStatus : public TimeoutHandler
{
public:
	MpdStatus()
	{
		this->m_playing = false;
		this->m_current_id = ~0;
		this->m_n_songs = 4;

		m_song_duration = 0;
		m_current_elapsed_secs = 0;
		m_queue_length = 0;

		for (unsigned i = 0; i < this->m_n_songs; i++)
			m_active_songs[i] = new Song();
		m_current_song = m_active_songs[0];

		/* Update this directly */
		this->timeoutCallback();
	}

	static MpdStatus *getInstance()
	{
		static MpdStatus *g_instance;

		if (g_instance)
			return g_instance;

		g_instance = new MpdStatus();

		return g_instance;
	}



	bool isPlaying()
	{
		return m_playing;
	}

	int getCurrentSongPos()
	{
		return m_current_elapsed_secs;
	}

	Song **getCurrentQueueSongs()
	{
		return m_active_songs;
	}

	int getNumberOfQueueSongs()
	{
		return 4;
	}

	int getQueueLength()
	{
		return m_queue_length;
	}

	Song *getCurrentSong()
	{
		return m_current_song;
	}


	void next()
	{
		struct mpd_status *status = mpd_run_status(Gui::gui->mpd_conn);

		if (!status)
			return;

		unsigned last_id = mpd_status_get_queue_length(status);

		/* Stop if we're at the last entry */
		if (m_current_id == last_id)
			mpd_run_stop(Gui::gui->mpd_conn);
		else
			mpd_run_next(Gui::gui->mpd_conn);

		TimerController::controller->arm(this);
		mpd_status_free(status);
	}

	void prev()
	{
		mpd_run_previous(Gui::gui->mpd_conn);
		TimerController::controller->arm(this);
	}

	void playPause()
	{
		struct mpd_status *status = mpd_run_status(Gui::gui->mpd_conn);

		if (!status)
			return;

		enum mpd_state state = mpd_status_get_state(status);
		if (state == MPD_STATE_STOP)
			mpd_run_play(Gui::gui->mpd_conn);
		else
			mpd_run_toggle_pause(Gui::gui->mpd_conn);

		TimerController::controller->arm(this);
		mpd_status_free(status);
	}

protected:
	void timeoutCallback()
	{
		struct mpd_song *song;
		struct mpd_status *status;

		Gui::gui->m_needs_redraw = true;

		/* Retrigger */
		TimerController::controller->arm(this, 700);

		status = mpd_run_status(Gui::gui->mpd_conn);
		if (!status)
			return;

		m_queue_length = mpd_status_get_queue_length(status);
		enum mpd_state state = mpd_status_get_state(status);
		unsigned last_id = mpd_status_get_queue_length(status);

		this->m_playing = (state == MPD_STATE_PLAY);
		m_current_elapsed_secs = mpd_status_get_elapsed_ms(status) / 1000;
		mpd_status_free(status);

		song = mpd_run_current_song(Gui::gui->mpd_conn);
		if (!song)
			return;

		m_song_duration = mpd_song_get_duration(song);
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
				m_active_songs[i]->set(i, "", "", 1);

		for (unsigned i = first; i < last; i++) {
			const char *artist;
			const char *title;
			int duration;
			struct mpd_song *cur = mpd_run_get_queue_song_id(Gui::gui->mpd_conn, i);
			unsigned id = mpd_song_get_id(cur);

			if (!cur)
				break;

			artist = mpd_song_get_tag(cur, MPD_TAG_ARTIST, 0);
			title = mpd_song_get_tag(cur, MPD_TAG_TITLE, 0);
			duration = mpd_song_get_duration(song);

			if (artist && title)
				m_active_songs[i - first]->set(i, artist, title, duration);
			if (id == m_current_id)
				m_current_song = m_active_songs[i - first];

			mpd_song_free(cur);
		}

		mpd_response_finish(Gui::gui->mpd_conn);
	}

	bool m_playing;

	unsigned m_current_id;

	unsigned m_song_duration;
	unsigned m_current_elapsed_secs;

	Song *m_active_songs[4];
	Song *m_current_song;
	unsigned m_n_songs;

	unsigned m_queue_length;

};

#endif /* MPD_STATUS_HH_ */
