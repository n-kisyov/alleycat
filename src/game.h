#ifndef ALLEYCAT_GAME_H
#define ALLEYCAT_GAME_H

#include <SDL.h>
#include <stdint.h>

#define SCREEN_W 320
#define SCREEN_H 200
#define SCALE 3

#define ROOM_FISHBOWL 0
#define ROOM_KENNEL   1
#define ROOM_BIRDCAGE 2
#define ROOM_CHEESE   3
#define ROOM_LOVE     4
#define ROOM_MILK     5
#define ROOM_COUNT    6

struct game_state {
	SDL_Window   *window;
	SDL_Renderer *renderer;
	SDL_Texture  *framebuffer;
	uint32_t     *pixels;
	int           running;
	int           score;
	int           lives;
	int           level;
	int           rooms_done;
	int           rooms_completed[ROOM_COUNT];
	int           current_room;
	uint32_t      ticks;
	int           cats_remaining;
};

extern struct game_state g_state;

void game_init(void);
void game_quit(void);
void game_start_room(int room);
void game_room_done(int room);
int  game_room_is_done(int room);
int  game_room_next(void);

#endif
