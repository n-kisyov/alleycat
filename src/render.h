#ifndef ALLEYCAT_RENDER_H
#define ALLEYCAT_RENDER_H

#include <stdint.h>
#include <SDL.h>

#define PAL_BLACK    0
#define PAL_BLUE     1
#define PAL_GREEN    2
#define PAL_CYAN     3
#define PAL_RED      4
#define PAL_MAGENTA  5
#define PAL_BROWN    6
#define PAL_LGRAY    7
#define PAL_DGRAY    8
#define PAL_LBLUE    9
#define PAL_LGREEN   10
#define PAL_LCYAN    11
#define PAL_LRED     12
#define PAL_LMAGENTA 13
#define PAL_YELLOW   14
#define PAL_WHITE    15

extern const uint32_t cga_colors[16];
extern SDL_Surface *screen_surface;

void render_init(void);
void render_present(void);
void render_fill(uint8_t color_index);
void render_fill_rect(int x, int y, int w, int h, uint8_t color_index);
void render_sprite(const uint8_t *data, int x, int y, int w, int h);
void render_sprite_clipped(const uint8_t *data, int x, int y, int w, int h,
                           int clip_x, int clip_y, int clip_w, int clip_h);
void render_char(char c, int x, int y);
void render_text(const char *text, int x, int y);
void render_number(int num, int x, int y, int digits);
void render_line(int x1, int y1, int x2, int y2, uint8_t color);
void render_rect(int x, int y, int w, int h, uint8_t color);

#endif
