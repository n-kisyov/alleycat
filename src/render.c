#include "render.h"
#include "sprites.h"
#include "game.h"
#include <string.h>
#include <stdlib.h>

/* The full CGA hardware palette.  Only the four entries named by cga_pal_1
 * are ever reachable on screen, but the AND blit can land on other indices
 * (11 & 13 == 9, for instance), exactly as it does on real hardware, so the
 * whole table has to be present. */
static const uint32_t cga_colors[16] = {
	0xFF000000, 0xFF0000AA, 0xFF00AA00, 0xFF00AAAA,
	0xFFAA0000, 0xFFAA00AA, 0xFFAA5500, 0xFFAAAAAA,
	0xFF555555, 0xFF5555FF, 0xFF55FF55, 0xFF55FFFF,
	0xFFFF5555, 0xFFFF55FF, 0xFFFFFF55, 0xFFFFFFFF
};

/* Mode 4 palette 1, high intensity: black, light cyan, light magenta, white. */
static const uint8_t cga_pal_1[4] = {
	CGA_BLACK, CGA_CYAN, CGA_MAGENTA, CGA_WHITE
};

#define KEY_INDEX 3   /* white -- the colour key for the cat and the items */

static SDL_Renderer *sdl_ren;
static SDL_Texture  *tex;
static uint8_t      *fb;       /* sw * sh colour indices */
static uint32_t     *pixels;   /* scratch for the texture upload */
static int           sw, sh;

int rect_overlap(struct rect a, struct rect b)
{
	return a.x < b.x + b.w && b.x < a.x + a.w &&
	       a.y < b.y + b.h && b.y < a.y + a.h;
}

int render_init(int w, int h)
{
	sw = w;
	sh = h;

	/* Nearest-neighbour, set before the renderer exists or it is ignored. */
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "0");

	sdl_ren = SDL_CreateRenderer(g_state.window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!sdl_ren)
		sdl_ren = SDL_CreateRenderer(g_state.window, -1, SDL_RENDERER_SOFTWARE);
	if (!sdl_ren)
		return 0;

	SDL_RenderSetLogicalSize(sdl_ren, w, h);
	SDL_RenderSetIntegerScale(sdl_ren, SDL_TRUE);

	tex = SDL_CreateTexture(sdl_ren, SDL_PIXELFORMAT_ARGB8888,
	                        SDL_TEXTUREACCESS_STREAMING, w, h);
	if (!tex)
		return 0;

	fb     = calloc((size_t)w * h, 1);
	pixels = calloc((size_t)w * h, sizeof(*pixels));
	return fb != NULL && pixels != NULL;
}

void render_shutdown(void)
{
	free(pixels);
	pixels = NULL;
	free(fb);
	fb = NULL;
	if (tex) {
		SDL_DestroyTexture(tex);
		tex = NULL;
	}
	if (sdl_ren) {
		SDL_DestroyRenderer(sdl_ren);
		sdl_ren = NULL;
	}
}

void render_present(void)
{
	int i, n = sw * sh;

	if (!sdl_ren || !tex || !fb || !pixels)
		return;

	for (i = 0; i < n; i++)
		pixels[i] = cga_colors[fb[i] & 15];

	SDL_UpdateTexture(tex, NULL, pixels, sw * (int)sizeof(*pixels));
	SDL_SetRenderDrawColor(sdl_ren, 0, 0, 0, 255);
	SDL_RenderClear(sdl_ren);
	SDL_RenderCopy(sdl_ren, tex, NULL, NULL);
	SDL_RenderPresent(sdl_ren);
}

/* ---- primitives -------------------------------------------------------- */

void render_fill(uint8_t color)
{
	if (fb)
		memset(fb, color, (size_t)sw * sh);
}

void render_fill_rect(int x, int y, int w, int h, uint8_t color)
{
	int row;

	if (!fb)
		return;
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if (x + w > sw) w = sw - x;
	if (y + h > sh) h = sh - y;
	if (w <= 0 || h <= 0)
		return;

	for (row = 0; row < h; row++)
		memset(fb + (size_t)(y + row) * sw + x, color, (size_t)w);
}

void render_line(int x1, int y1, int x2, int y2, uint8_t color)
{
	int dx =  abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
	int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
	int err = dx + dy;

	if (!fb)
		return;

	for (;;) {
		if ((unsigned)x1 < (unsigned)sw && (unsigned)y1 < (unsigned)sh)
			fb[(size_t)y1 * sw + x1] = color;
		if (x1 == x2 && y1 == y2)
			break;
		{
			int e2 = 2 * err;
			if (e2 >= dy) { err += dy; x1 += sx; }
			if (e2 <= dx) { err += dx; y1 += sy; }
		}
	}
}

void render_rect(int x, int y, int w, int h, uint8_t color)
{
	if (w <= 0 || h <= 0)
		return;
	render_line(x, y, x + w - 1, y, color);
	render_line(x, y + h - 1, x + w - 1, y + h - 1, color);
	render_line(x, y, x, y + h - 1, color);
	render_line(x + w - 1, y, x + w - 1, y + h - 1, color);
}

/* ---- sprite blits ------------------------------------------------------ */

/* mode: 0 opaque, 1 keyed on white, 2 AND into the destination,
 *       3 stencil -- every non-key pixel drawn in `tint`. */
static void blit(const struct sprite *s, int x, int y, int mode, uint8_t tint)
{
	int bytes_per_row, row;
	const uint8_t *data;

	if (!fb || !s || !s->data)
		return;

	bytes_per_row = (s->w + 3) / 4;
	data = s->data;

	for (row = 0; row < s->h; row++, data += bytes_per_row) {
		int dy = y + row;
		int col;

		if (dy < 0 || dy >= sh)
			continue;

		for (col = 0; col < s->w; col++) {
			int dx = x + col;
			uint8_t px, c;

			if ((unsigned)dx >= (unsigned)sw)
				continue;

			px = (data[col >> 2] >> (6 - (col & 3) * 2)) & 3;
			if ((mode == 1 || mode == 3) && px == KEY_INDEX)
				continue;

			c = (mode == 3) ? tint : cga_pal_1[px];
			if (mode == 2)
				fb[(size_t)dy * sw + dx] &= c;
			else
				fb[(size_t)dy * sw + dx] = c;
		}
	}
}

void render_sprite(const struct sprite *s, int x, int y)       { blit(s, x, y, 0, 0); }
void render_sprite_keyed(const struct sprite *s, int x, int y) { blit(s, x, y, 1, 0); }
void render_sprite_and(const struct sprite *s, int x, int y)   { blit(s, x, y, 2, 0); }

void render_sprite_stencil(const struct sprite *s, int x, int y, uint8_t color)
{
	blit(s, x, y, 3, color);
}

/* ---- text -------------------------------------------------------------- */

/* Glyphs are 8 pixels wide and the original steps 8 pixels between them. */
static const struct sprite *glyph(char c)
{
	if (c >= 'A' && c <= 'Z') return &spr_letters[c - 'A'];
	if (c >= 'a' && c <= 'z') return &spr_letters[c - 'a'];
	if (c >= '0' && c <= '9') return &spr_digits[c - '0'];
	if (c == '!')             return &spr_punctuation[0];
	if (c == '-')             return &spr_punctuation[1];
	if (c == '.')             return &spr_punctuation[2];
	return NULL;
}

static void draw_text(const char *text, int x, int y, int keyed)
{
	if (!text)
		return;
	for (; *text; text++, x += GLYPH_W) {
		const struct sprite *g = glyph(*text);
		if (!g)
			continue;
		if (keyed)
			render_sprite_keyed(g, x, y);
		else
			render_sprite(g, x, y);
	}
}

void render_text(const char *text, int x, int y)       { draw_text(text, x, y, 0); }
void render_text_keyed(const char *text, int x, int y) { draw_text(text, x, y, 1); }

int render_text_width(const char *text)
{
	return text ? (int)strlen(text) * GLYPH_W : 0;
}

void render_text_centered(const char *text, int y)
{
	render_text(text, (sw - render_text_width(text)) / 2, y);
}

void render_banner(const char *text, int y)
{
	render_fill_rect(0, y, sw, TEXT_BAND_H, CGA_CYAN);
	render_text_centered(text, y + (TEXT_BAND_H - GLYPH_H) / 2);
}

void render_number(int num, int x, int y, int digits)
{
	char buf[16];
	int i, limit;

	if (digits < 1)
		digits = 1;
	if (digits > (int)sizeof(buf) - 1)
		digits = (int)sizeof(buf) - 1;

	/* Clamp rather than emit characters outside '0'..'9', which the glyph
	 * lookup would silently drop and leave the field blank. */
	if (num < 0)
		num = 0;
	for (i = 0, limit = 1; i < digits && limit <= 100000000; i++)
		limit *= 10;
	if (num > limit - 1)
		num = limit - 1;

	for (i = digits - 1; i >= 0; i--) {
		buf[i] = (char)('0' + num % 10);
		num /= 10;
	}
	buf[digits] = '\0';
	render_text(buf, x, y);
}
