#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <png.h>
#include <sys/stat.h>
#include <SDL_ttf.h>
#include <SDL_image.h>

#include "font.hh"
#include "utils.hh"

TTF_Font *read_and_alloc_font(const char *path, int pt_size)
{
	TTF_Font *out;
	SDL_RWops *rw;
	Uint8 *data;
	FILE *fp = fopen(path, "r");
	size_t r;

	if (!fp) {
		fprintf(stderr, "Could not open font %s\n", path);
		return NULL;
	}
	data = (Uint8*)xmalloc(1 * 1024*1024);
	r = fread(data, 1, 1 * 1024 * 1024, fp);
	if (r == 0 || ferror(fp))
	{
		free(data);
		return NULL;
	}
	rw = SDL_RWFromMem(data, 1 * 1024 * 1024);
	if (!rw)
	{
		fprintf(stderr, "Could not create RW: %s\n", SDL_GetError());
		free(data);
		return NULL;
	}
	out = TTF_OpenFontRW(rw, 1, pt_size);
	if (!out)
	{
		fprintf(stderr, "TTF: Unable to create font %s (%s)\n",
				path, TTF_GetError());
	}
	fclose(fp);

	return out;
}




const char *construct_path(const char *base_dir, const char *file)
{
	size_t len = strlen(base_dir) + strlen(file) + 2;
	char *out = (char *)xmalloc(len);
	const char *delim = "/";

	if (strcmp(base_dir, "") == 0)
		delim = "";

	sprintf(out, "%s%s%s", base_dir, delim, file);

	return out;
}

void highlight_background(SDL_Surface *where, Font *font,
		SDL_Surface *bg_left, SDL_Surface *bg_middle, SDL_Surface *bg_right,
		int x, int y, int w, int h)
{
	SDL_Rect dst;

	/* Can't highlight without images */
	if (!bg_left ||	!bg_middle || !bg_right)
		return;

	int font_height = font->getHeight("X");
	int bg_y_start = y + font_height / 2 -
			bg_left->h / 2;
	int bg_x_start = x - bg_left->w / 3;
	int bg_x_end = x + w - (2 * bg_right->w) / 3;
	int n_mid = (bg_x_end - bg_x_start) / bg_middle->w;

	/* Left */
	dst = (SDL_Rect){bg_x_start, bg_y_start, 0,0};
	SDL_BlitSurface(bg_left, NULL, where, &dst);

	/* Middle */
	for (int i = 1; i < n_mid; i++)
	{
		dst = (SDL_Rect){bg_x_start + i * bg_middle->w, bg_y_start, 0,0};
		SDL_BlitSurface(bg_middle, NULL, where, &dst);
	}
	dst = (SDL_Rect){bg_x_end - bg_middle->w, bg_y_start, 0,0};
	SDL_BlitSurface(bg_middle, NULL, where, &dst);

	/* Right */
	dst = (SDL_Rect){bg_x_end, bg_y_start, 0,0};
	SDL_BlitSurface(bg_right, NULL,	where, &dst);
}
