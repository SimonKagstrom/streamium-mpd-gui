#include <SDL.h>
#include <SDL_ttf.h>

#include <utils.hh>
#include "font.hh"

#ifndef __SDL_TTF_FONT_HH__
#define __SDL_TTF_FONT_HH__

#define N_SURFACE_CACHE 16

class Font_TTF : public Font
{
public:
	Font_TTF(TTF_Font *font,
			int r, int g, int b)
	{
		this->clr = (SDL_Color){r, g, b};
		this->font = font;
		this->m_cache_replace = 0;
	}

	virtual ~Font_TTF()
	{
		TTF_CloseFont(this->font);
	}

	int getHeight(const char *str)
	{
		int tw, th;

		TTF_SizeText(this->font, str, &tw, &th);

		return th;
	}

	int getWidth(const char *str)
	{
		int tw, th;

		TTF_SizeText(this->font, str, &tw, &th);

		return tw;
	}

	void draw(SDL_Surface *where, const char *msg,
			int x, int y, int w, int h, bool bold)
	{
		SDL_Surface *p;
		SDL_Rect dst;
		uint32_t style = TTF_STYLE_NORMAL;

		/* Nothing to print */
		if (strcmp(msg, "") == 0)
			return;

		if (bold)
				style = TTF_STYLE_BOLD | TTF_STYLE_ITALIC;

		for (int i = 0; i < N_SURFACE_CACHE; i++) {
			p = this->m_cache[i].match(msg, style);

			if (p)
				break;
		}
		if (!p) {
			TTF_SetFontStyle(this->font, style);

			p = TTF_RenderUTF8_Blended(this->font, msg, this->clr);
			if (!p)
				return;
			this->m_cache[this->m_cache_replace].replace(p, msg, style);
			this->m_cache_replace = (this->m_cache_replace + 1) & (N_SURFACE_CACHE - 1);
		}

		dst = (SDL_Rect){x, y, w, h};

		SDL_BlitSurface(p, NULL, where, &dst);
	}

protected:
	class CachedSurface
	{
	public:
		CachedSurface()
		{
			m_surface = NULL;
			strcpy(m_str, "");
			m_style = 0xffffffff;
		}

		SDL_Surface *match(const char *str, uint32_t style)
		{
			if (style != m_style)
				return NULL;
			if (strcmp(str, m_str) != 0)
				return NULL;

			return m_surface;
		}

		void replace(SDL_Surface *surf, const char *str, uint32_t style)
		{
			if (m_surface)
				SDL_FreeSurface(m_surface);

			m_surface = surf;
			strncpy(m_str, str, sizeof(m_str));
			m_style = style;
		}

		SDL_Surface *m_surface;
		char m_str[255];
		uint32_t m_style;
	};

	CachedSurface m_cache[N_SURFACE_CACHE];
	int m_cache_replace;
	TTF_Font *font;
	SDL_Color clr;
};

#endif
