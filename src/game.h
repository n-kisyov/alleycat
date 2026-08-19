#ifndef ALLEYCAT_GAME_H
#define ALLEYCAT_GAME_H

#include <SDL.h>
#include <stdint.h>

#define SCREEN_W 320
#define SCREEN_H 200

/* Logic runs at a fixed rate independent of the display; every duration in
 * the game is counted in these ticks. */
#define LOGIC_HZ 60
#define SECONDS(n) ((n) * LOGIC_HZ)

#define ROOM_FISHBOWL 0
#define ROOM_KENNEL   1
#define ROOM_BIRDCAGE 2
#define ROOM_CHEESE   3
#define ROOM_LOVE     4
#define ROOM_MILK     5
#define ROOM_COUNT    6

#define STARTING_LIVES 9
#define FLASH_MAX      24

struct game_state {
	SDL_Window *window;
	int         running;
	int         score;
	int         hi_score;
	int         lives;
	int         level;
	int         rooms_done;
	int         rooms_completed[ROOM_COUNT];
	int         current_room;
	uint32_t    ticks;

	/* Short banner shown over the alley after a room ends, so leaving one
	 * is not the silent teleport it used to be. */
	char        flash[FLASH_MAX];
	int         flash_timer;
};

extern struct game_state g_state;

void game_init(void);          /* once, at startup */
void game_new(void);           /* reset for a fresh run */
void game_quit(void);

void game_start_room(int room);
void game_room_done(int room);
int  game_room_is_done(int room);
const char *game_room_name(int room);

void game_add_score(int points);

/* Returns 1 when that was the last life. */
int  game_lose_life(void);

/* All six rooms cleared: bank the bonus, bump the level, reopen the windows. */
void game_level_complete(void);

void game_flash(const char *msg);
void game_flash_update(void);

/* Difficulty derived from the level, so the rooms do not each invent their
 * own scaling. */
int  game_room_time_limit(void);   /* in logic ticks */
int  game_enemy_bonus_speed(void);

#endif
