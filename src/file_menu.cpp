#include <unistd.h> /* unlink */

#include "utils.hh"
#include "menu.hh"

class FileMenu;

class FileView : public GuiView
{
public:
	FileView(bool playlist);

	virtual ~FileView();

	void pushEvent(event_t ev);

	void viewPushCallback();

	void pushPath(const char *path);

	void addSong(const char *fileName);

	int addRecursively(const char *path);

	void updateList();

	const char **addEntry(const char **list, bool **is_dir, int *n_entries, const char *entry, bool dir);

	/* Inherited */
	void runLogic();

	void draw(SDL_Surface *where);


	bool m_playlist;
	FileMenu *m_menu;
	char m_path[1024];
};


class FileMenu : public Menu
{
	friend class FileView;

public:
	FileMenu(FileView *view, Font *font) :
		Menu(font)
	{
		m_view = view;
		m_is_dir = NULL;
	}

	virtual ~FileMenu()
	{
	}

	virtual void hoverCallback(int which)
	{
	}

	int selectNext(int dx, int dy)
	{
		this->cur_sel = this->getNextEntry(dy);

		if (dx > 0 && m_is_dir[this->cur_sel] && !m_view->m_playlist) {
			printf("Go down into %s\n", this->pp_msgs[this->cur_sel]);
			m_view->pushPath(this->pp_msgs[this->cur_sel]);
		} else if (dx < 0) {
			Gui::gui->popView();
			return 0;
		}

		return this->cur_sel;
	}

	virtual void selectCallback(int which)
	{
		const char *fileName = this->pp_msgs[this->cur_sel];

		/* Nothing to add */
		if (strcmp(fileName, "None") == 0)
			return;

		if (m_view->m_playlist) {
			mpd_run_load(Gui::gui->mpd_conn, fileName);
			Gui::gui->status_bar->queueMessage("Queued playlist %s",
					fileName);
		} else {
			int n = m_view->addRecursively(fileName);

			Gui::gui->status_bar->queueMessage("Queued %d song%s",
					n, n == 1 ? "" : "s");
		}
	}

	virtual void escapeCallback(int which)
	{
		Gui::gui->popView();
	}

	void setDirectoryText(const char **text, bool *is_dir, int n_entries)
	{
		setText(text);
		free(m_is_dir);

		m_is_dir = (bool *)xmalloc(n_entries * sizeof(bool));
		for (int i = 0; i < n_entries; i++)
			m_is_dir[i] = is_dir[i];
	}

	bool *m_is_dir;
	FileView *m_view;
};


FileView::FileView(bool playlist) : GuiView()
{
	m_playlist = playlist;
	m_menu = new FileMenu(this, Gui::gui->default_font);
	strcpy(m_path, "");
}

FileView::~FileView()
{
	delete m_menu;
}

const char **FileView::addEntry(const char **list, bool **is_dir_list, int *n_entries, const char *name, bool is_dir)
{
	const char **out = (const char **)xrealloc(list, sizeof(const char *) * (*n_entries + 2));
	bool *out_dir_list = (bool *)xrealloc(*is_dir_list, sizeof(bool *) * (*n_entries + 2));

	out[*n_entries] = xstrdup(name);
	out_dir_list[*n_entries] = is_dir;
	out[*n_entries + 1] = NULL;

	(*n_entries)++;
	*is_dir_list = out_dir_list;

	return out;
}

void FileView::pushPath(const char *path)
{
	snprintf(m_path, sizeof(m_path), "%s", path);

	updateList();
}

void FileView::addSong(const char *fileName)
{
	mpd_run_add(Gui::gui->mpd_conn, fileName);
}

int FileView::addRecursively(const char *path)
{
	const char **paths;
	const char **songs = NULL;
	int n_paths = 1;
	int n_songs = 0;
	int i = 0;

	paths = (const char **)xmalloc(sizeof(const char *));
	paths[0] = xstrdup(path);

	while (i < n_paths) {
		bool res = mpd_send_list_meta(Gui::gui->mpd_conn, paths[i]);

		printf("VOBB add: %s: %d\n", paths[i], res);

		if (!res)
			break;

		struct mpd_entity *entity;

		while ( (entity = mpd_recv_entity(Gui::gui->mpd_conn)) ) {
			const struct mpd_song *song = NULL;
			const struct mpd_directory *dir = NULL;
			const struct mpd_playlist *pl = NULL;

			printf("XXX\n");
			switch (mpd_entity_get_type(entity)) {
			case MPD_ENTITY_TYPE_UNKNOWN:
				break;

			case MPD_ENTITY_TYPE_SONG:
				song = mpd_entity_get_song(entity);
				if (song) {
					n_songs++;
					songs = (const char **)xrealloc(songs, sizeof(char *) * n_songs);
					songs[n_songs - 1] = xstrdup(mpd_song_get_uri(song));
				}
				break;

			case MPD_ENTITY_TYPE_DIRECTORY:
				dir = mpd_entity_get_directory(entity);
				if (dir && !m_playlist) {
					int cur = n_paths;

					n_paths++;
					paths = (const char **)xrealloc(paths, sizeof(char *) * (n_paths + 1));
					paths[cur] = xstrdup(mpd_directory_get_path(dir));
				}

				break;

			case MPD_ENTITY_TYPE_PLAYLIST:
				pl = mpd_entity_get_playlist(entity);
				if (pl && m_playlist) {
					int cur = n_paths;

					n_paths++;
					paths = (const char **)xrealloc(paths, sizeof(char *) * (n_paths + 1));
					paths[cur] = xstrdup(mpd_playlist_get_path(pl));
					printf("XXX: %s\n", paths[cur]);
				}

				break;
			}

			mpd_entity_free(entity);
		}

		i++;
	}
	for (int j = 0; j < i; j++)
		free((void *)paths[j]);
	free(paths);

	for (int j = 0; j < n_songs; j++) {
		addSong(songs[j]);
		free((void *)songs[j]);
	}
	free(songs);

	return n_songs;
}


void FileView::updateList()
{
	bool res = mpd_send_list_meta(Gui::gui->mpd_conn, m_path);

	if (!res)
		return;

	struct mpd_entity *entity;
	int n = 0;
	const char **text = NULL;
	bool *is_dir = NULL;

	while ( (entity = mpd_recv_entity(Gui::gui->mpd_conn)) ) {
		const struct mpd_song *song = NULL;
		const struct mpd_directory *dir = NULL;
		const struct mpd_playlist *pl = NULL;

		switch (mpd_entity_get_type(entity)) {
		case MPD_ENTITY_TYPE_UNKNOWN:
			break;

		case MPD_ENTITY_TYPE_SONG:
			song = mpd_entity_get_song(entity);
			text = addEntry(text, &is_dir, &n, mpd_song_get_uri(song), false);
			break;

		case MPD_ENTITY_TYPE_DIRECTORY:
			if (!m_playlist) {
				dir = mpd_entity_get_directory(entity);
				text = addEntry(text, &is_dir, &n, mpd_directory_get_path(dir), true);
			}
			break;

		case MPD_ENTITY_TYPE_PLAYLIST:
			if (m_playlist) {
				pl = mpd_entity_get_playlist(entity);
				text = addEntry(text, &is_dir, &n, mpd_playlist_get_path(pl), true);
			}
			break;
		}

		mpd_entity_free(entity);
	}

	if (n == 0) {
		const char *str[2];
		bool dir = false;

		str[0] = "None";
		str[1] = NULL;

		m_menu->setDirectoryText(str, &dir, 1);
	}
	else
		m_menu->setDirectoryText((const char **)text, is_dir, n);

	for (int i = 0; i < n; i++)
		free((void *)text[i]);
	free(text);
}

void FileView::viewPushCallback()
{
	strcpy(m_path, "");
	updateList();
}

void FileView::runLogic()
{
	m_menu->runLogic();
}

void FileView::pushEvent(event_t ev)
{
	m_menu->pushEvent(ev);
}

void FileView::draw(SDL_Surface *where)
{
	SDL_Rect dst;

	/* Blit the backgrounds */
	dst = (SDL_Rect){10,10,300,220};
	SDL_BlitSurface(Gui::gui->main_menu_bg, NULL, where, &dst);

	m_menu->draw(where, 20, 20, 280, 200);
}

