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

static SDL_Surface *back = NULL;
static int bw, bh;

void render_init(int w, int h)
{
	bw = w;
	bh = h;

	SDL_Surface *win = SDL_GetWindowSurface(g_state.window);

	back = SDL_CreateRGBSurface(0, w, h, 32,
		win ? win->format->Rmask : 0x00FF0000,
		win ? win->format->Gmask : 0x0000FF00,
		win ? win->format->Bmask : 0x000000FF,
		win ? win->format->Amask : 0xFF000000);
}

void render_present(void)
{
	if (!back || !g_state.window) return;

	SDL_Surface *win = SDL_GetWindowSurface(g_state.window);
	if (!win) return;

	SDL_Rect dst = { 0, 0, win->w, win->h };
	SDL_BlitScaled(back, NULL, win, &dst);
	SDL_UpdateWindowSurface(g_state.window);
}

static void put_raw(int x, int y, uint32_t color)
{
	if (!back || x < 0 || x >= bw || y < 0 || y >= bh) return;
	uint32_t *pixels = (uint32_t *)back->pixels;
	int pitch_px = back->pitch / 4;
	pixels[y * pitch_px + x] = color;
}

void render_fill(uint8_t ci)
{
	if (!back) return;
	uint32_t c = cga_colors[ci & 15];
	uint32_t *pixels = (uint32_t *)back->pixels;
	int pitch_px = back->pitch / 4;
	for (int y = 0; y < bh; y++)
		for (int x = 0; x < bw; x++)
			pixels[y * pitch_px + x] = c;
}

void render_fill_rect(int x, int y, int w, int h, uint8_t ci)
{
	if (!back) return;
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if (x + w > bw) w = bw - x;
	if (y + h > bh) h = bh - y;
	if (w <= 0 || h <= 0) return;
	uint32_t c = cga_colors[ci & 15];
	for (int row = 0; row < h; row++)
		for (int col = 0; col < w; col++)
			put_raw(x + col, y + row, c);
}

void render_sprite(const uint8_t *data, int x, int y, int w, int h)
{
	if (!back || !data) return;
	int bytes_per_row = (w + 3) / 4;

	for (int row = 0; row < h; row++) {
		int draw_y = y + row;
		if (draw_y < 0 || draw_y >= bh) {
			data += bytes_per_row;
			continue;
		}
		int drawn = 0;
		for (int b = 0; b < bytes_per_row; b++) {
			uint8_t byte = *data++;
			int remaining = w - drawn;
			int to_draw = remaining < 4 ? remaining : 4;
			for (int p = 0; p < to_draw; p++) {
				int draw_x = x + drawn + p;
				if (draw_x < 0 || draw_x >= bw) continue;
				uint8_t pix = (byte >> (6 - p * 2)) & 0x3;
				put_raw(draw_x, draw_y, cga_colors[cga_pal_1[pix & 3]]);
			}
			drawn += to_draw;
		}
	}
}

void render_sprite_clipped(const uint8_t *data, int x, int y, int w, int h,
                           int clip_x, int clip_y, int clip_w, int clip_h)
{
	if (!back || !data) return;
	int bytes_per_row = (w + 3) / 4;

	for (int row = 0; row < h; row++) {
		int draw_y = y + row;
		if (draw_y < clip_y || draw_y >= clip_y + clip_h) {
			data += bytes_per_row;
			continue;
		}
		int drawn = 0;
		for (int b = 0; b < bytes_per_row; b++) {
			uint8_t byte = *data++;
			int remaining = w - drawn;
			int to_draw = remaining < 4 ? remaining : 4;
			for (int p = 0; p < to_draw; p++) {
				int draw_x = x + drawn + p;
				if (draw_x < clip_x || draw_x >= clip_x + clip_w) continue;
				uint8_t pix = (byte >> (6 - p * 2)) & 0x3;
				put_raw(draw_x, draw_y, cga_colors[cga_pal_1[pix & 3]]);
			}
			drawn += to_draw;
		}
	}
}

void render_char(char c, int x, int y)
{
	if (c >= 'A' && c <= 'Z')
		render_sprite(sprite_letters[c - 'A'], x, y, 8, 8);
	else if (c >= '0' && c <= '9')
		render_sprite(sprite_digits[c - '0'], x, y, 8, 8);
	else if (c >= 'a' && c <= 'z')
		render_sprite(sprite_letters[c - 'a'], x, y, 8, 8);
	else {
		switch (c) {
		case '!': render_sprite(sprite_punctuation[0], x, y, 8, 8); break;
		case '-': render_sprite(sprite_punctuation[1], x, y, 8, 8); break;
		case '.': render_sprite(sprite_punctuation[2], x, y, 8, 8); break;
		}
	}
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
	int i;
	buf[digits] = '\0';
	for (i = digits - 1; i >= 0; i--) {
		buf[i] = '0' + (num % 10);
		num /= 10;
	}
	render_text(buf, x, y);
}

void render_line(int x1, int y1, int x2, int y2, uint8_t color)
{
	if (!back) return;
	uint32_t c = cga_colors[color & 15];
	int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
	int dy = -abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
	int err = dx + dy;

	while (1) {
		put_raw(x1, y1, c);
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
