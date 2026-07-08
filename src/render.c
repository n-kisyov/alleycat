#include "render.h"
#include "sprites.h"
#include "game.h"
#include <string.h>
#include <stdlib.h>

const uint32_t cga_colors[16] = {
	0xFF000000, 0xFF0000AA, 0xFF00AA00, 0xFF00AAAA,
	0xFFAA0000, 0xFFAA00AA, 0xFFAA5500, 0xFFAAAAAA,
	0xFF555555, 0xFF5555FF, 0xFF55FF55, 0xFF55FFFF,
	0xFFFF5555, 0xFFFF55FF, 0xFFFFFF55, 0xFFFFFFFF
};

static const int cga_pal_1[4] = { 0, 11, 13, 15 };

static SDL_Renderer *sdl_ren;
static SDL_Surface *surf;
static int sw, sh;

void render_init(int w, int h)
{
	sw = w;
	sh = h;

	sdl_ren = SDL_CreateRenderer(g_state.window, -1,
		SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!sdl_ren)
		sdl_ren = SDL_CreateRenderer(g_state.window, -1, SDL_RENDERER_SOFTWARE);
	if (sdl_ren)
		SDL_RenderSetLogicalSize(sdl_ren, w, h);

	surf = SDL_CreateRGBSurfaceWithFormat(0, w, h, 32,
		SDL_PIXELFORMAT_ARGB8888);
	if (!surf)
		surf = SDL_CreateRGBSurface(0, w, h, 32,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
}

void render_present(void)
{
	if (!sdl_ren || !surf) return;
	SDL_Texture *tex = SDL_CreateTextureFromSurface(sdl_ren, surf);
	if (tex) {
		SDL_SetRenderDrawColor(sdl_ren, 0, 0, 0, 255);
		SDL_RenderClear(sdl_ren);
		SDL_RenderCopy(sdl_ren, tex, NULL, NULL);
		SDL_DestroyTexture(tex);
	}
	SDL_RenderPresent(sdl_ren);
}

static void put32(int x, int y, uint32_t c)
{
	if (!surf || (unsigned)x >= (unsigned)sw || (unsigned)y >= (unsigned)sh)
		return;
	uint32_t *p = (uint32_t *)((uint8_t *)surf->pixels + y * surf->pitch);
	p[x] = c;
}

void render_fill(uint8_t ci)
{
	if (!surf) return;
	uint32_t c = cga_colors[ci & 15];
	uint8_t *row = (uint8_t *)surf->pixels;
	for (int y = 0; y < sh; y++) {
		uint32_t *p = (uint32_t *)row;
		for (int x = 0; x < sw; x++) p[x] = c;
		row += surf->pitch;
	}
}

void render_fill_rect(int x, int y, int w, int h, uint8_t ci)
{
	if (!surf) return;
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if (x + w > sw) w = sw - x;
	if (y + h > sh) h = sh - y;
	if (w <= 0 || h <= 0) return;
	uint32_t c = cga_colors[ci & 15];
	for (int row = 0; row < h; row++)
		for (int col = 0; col < w; col++)
			put32(x + col, y + row, c);
}

void render_sprite(const uint8_t *data, int x, int y, int w, int h)
{
	if (!surf || !data) return;
	int bytes_per_row = (w + 3) / 4;

	for (int row = 0; row < h; row++) {
		int dy = y + row;
		if (dy < 0 || dy >= sh) { data += bytes_per_row; continue; }
		int drawn = 0;
		for (int b = 0; b < bytes_per_row; b++) {
			uint8_t byte = *data++;
			int n = w - drawn;
			if (n > 4) n = 4;
			for (int p = 0; p < n; p++) {
				int dx = x + drawn + p;
				if ((unsigned)dx >= (unsigned)sw) continue;
				uint8_t px = (byte >> (6 - p * 2)) & 3;
				if (px == 0) continue;
				put32(dx, dy, cga_colors[cga_pal_1[px]]);
			}
			drawn += n;
		}
	}
}

void render_sprite_clipped(const uint8_t *data, int x, int y, int w, int h,
                           int cx, int cy, int cw, int ch)
{
	if (!surf || !data) return;
	int bytes_per_row = (w + 3) / 4;
	for (int row = 0; row < h; row++) {
		int dy = y + row;
		if (dy < cy || dy >= cy + ch) { data += bytes_per_row; continue; }
		int drawn = 0;
		for (int b = 0; b < bytes_per_row; b++) {
			uint8_t byte = *data++;
			int n = w - drawn;
			if (n > 4) n = 4;
			for (int p = 0; p < n; p++) {
				int dx = x + drawn + p;
				if (dx < cx || dx >= cx + cw || (unsigned)dx >= (unsigned)sw) continue;
				uint8_t px = (byte >> (6 - p * 2)) & 3;
				if (px == 0) continue;
				put32(dx, dy, cga_colors[cga_pal_1[px]]);
			}
			drawn += n;
		}
	}
}

void render_char(char c, int x, int y)
{
	if (c >= 'A' && c <= 'Z')       render_sprite(sprite_letters[c - 'A'], x, y, 8, 8);
	else if (c >= '0' && c <= '9')  render_sprite(sprite_digits[c - '0'], x, y, 8, 8);
	else if (c >= 'a' && c <= 'z')  render_sprite(sprite_letters[c - 'a'], x, y, 8, 8);
	else if (c == '!')              render_sprite(sprite_punctuation[0], x, y, 8, 8);
	else if (c == '-')              render_sprite(sprite_punctuation[1], x, y, 8, 8);
	else if (c == '.')              render_sprite(sprite_punctuation[2], x, y, 8, 8);
}

void render_text(const char *text, int x, int y)
{
	if (!text) return;
	while (*text) {
		render_char(*text, x, y);
		if (*text >= '0' && *text <= '9') x += 6;
		else if (*text == ' ') x += 4;
		else x += 6;
		text++;
	}
}

void render_number(int num, int x, int y, int digits)
{
	char buf[16];
	for (int i = digits - 1; i >= 0; i--) {
		buf[i] = '0' + (num % 10);
		num /= 10;
	}
	buf[digits] = 0;
	render_text(buf, x, y);
}

void render_line(int x1, int y1, int x2, int y2, uint8_t color)
{
	if (!surf) return;
	uint32_t c = cga_colors[color & 15];
	int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
	int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
	int err = dx + dy;
	while (1) {
		if ((unsigned)x1 < (unsigned)sw && (unsigned)y1 < (unsigned)sh)
			put32(x1, y1, c);
		if (x1 == x2 && y1 == y2) break;
		int e2 = 2 * err;
		if (e2 >= dy) { err += dy; x1 += sx; }
		if (e2 <= dx) { err += dx; y1 += sy; }
	}
}

void render_rect(int x, int y, int w, int h, uint8_t color)
{
	render_line(x, y, x + w - 1, y, color);
	render_line(x, y + h - 1, x + w - 1, y + h - 1, color);
	render_line(x, y, x, y + h - 1, color);
	render_line(x + w - 1, y, x + w - 1, y + h - 1, color);
}
