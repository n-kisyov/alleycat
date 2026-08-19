#ifndef ALLEYCAT_RENDER_H
#define ALLEYCAT_RENDER_H

#include <stdint.h>
#include <SDL.h>
#include "sprites.h"

/*
 * The framebuffer holds CGA colour indices, not packed RGB, so the AND blit
 * below combines exactly the way the original's video memory does.
 * Conversion to pixels happens once per frame in render_present().
 */

/* CGA mode 4, palette 1, high intensity.  These four are the whole display. */
#define CGA_BLACK   0
#define CGA_CYAN    11
#define CGA_MAGENTA 13
#define CGA_WHITE   15

#define GLYPH_W 8
#define GLYPH_H 8

struct rect { int x, y, w, h; };

int  rect_overlap(struct rect a, struct rect b);

int  render_init(int w, int h);
void render_shutdown(void);
void render_present(void);

void render_fill(uint8_t color);
void render_fill_rect(int x, int y, int w, int h, uint8_t color);
void render_rect(int x, int y, int w, int h, uint8_t color);
void render_line(int x1, int y1, int x2, int y2, uint8_t color);

/* Writes every pixel, including white.  Static art: fence, trashcans, the
 * letters and digits, the logos, the title graphics. */
void render_sprite(const struct sprite *s, int x, int y);

/* Skips colour index 3 (white).  The original keys the cat's surface on
 * white; the item sprites in this project follow the same convention. */
void render_sprite_keyed(const struct sprite *s, int x, int y);

/* ANDs the sprite into what is already there, as alleycat_draw_overlay does
 * in the reference. */
void render_sprite_and(const struct sprite *s, int x, int y);

/* Draws the sprite's shape in one colour, keying on white.  The item sprites
 * are one-bit silhouettes, and the cat is black by definition, so items need
 * to be tintable or they disappear against a dark room. */
void render_sprite_stencil(const struct sprite *s, int x, int y, uint8_t color);

void render_text(const char *text, int x, int y);
void render_text_keyed(const char *text, int x, int y);
void render_text_centered(const char *text, int y);

/* Cyan band with centred text.  Glyphs carry a light-cyan tile of their own,
 * so this is how text stays legible over black or magenta. */
#define TEXT_BAND_H 16
void render_banner(const char *text, int y);
void render_number(int num, int x, int y, int digits);
int  render_text_width(const char *text);

#endif
