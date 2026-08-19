#include "game.h"
#include "scene.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct game_state g_state;

static const char *room_names[ROOM_COUNT] = {
	"FISHBOWL", "KENNEL", "BIRDCAGE", "CHEESE", "LOVE", "MILK"
};

void game_init(void)
{
	SDL_Window *window = g_state.window;

	memset(&g_state, 0, sizeof(g_state));
	g_state.window = window;
	g_state.running = 1;

	/* Seeded once.  The scenes used to reseed from SDL_GetTicks() on entry,
	 * which is near zero at startup and made every run look identical. */
	srand((unsigned int)time(NULL));

	game_new();
}

void game_new(void)
{
	g_state.score = 0;
	g_state.lives = STARTING_LIVES;
	g_state.level = 1;
	g_state.rooms_done = 0;
	g_state.current_room = -1;
	g_state.flash[0] = '\0';
	g_state.flash_timer = 0;
	memset(g_state.rooms_completed, 0, sizeof(g_state.rooms_completed));
}

void game_quit(void)
{
	g_state.running = 0;
}

void game_start_room(int room)
{
	g_state.current_room = room;
}

void game_room_done(int room)
{
	if (room >= 0 && room < ROOM_COUNT && !g_state.rooms_completed[room]) {
		g_state.rooms_completed[room] = 1;
		g_state.rooms_done++;
	}
	g_state.current_room = -1;
}

int game_room_is_done(int room)
{
	if (room >= 0 && room < ROOM_COUNT)
		return g_state.rooms_completed[room];
	return 0;
}

const char *game_room_name(int room)
{
	return (room >= 0 && room < ROOM_COUNT) ? room_names[room] : "";
}

void game_add_score(int points)
{
	g_state.score += points;
	if (g_state.score < 0)
		g_state.score = 0;
	if (g_state.score > g_state.hi_score)
		g_state.hi_score = g_state.score;
}

int game_lose_life(void)
{
	g_state.current_room = -1;
	if (g_state.lives > 0)
		g_state.lives--;
	return g_state.lives <= 0;
}

void game_level_complete(void)
{
	game_add_score(500 * g_state.level);
	g_state.level++;
	g_state.rooms_done = 0;
	memset(g_state.rooms_completed, 0, sizeof(g_state.rooms_completed));
}

void game_flash(const char *msg)
{
	if (!msg) {
		g_state.flash[0] = '\0';
		g_state.flash_timer = 0;
		return;
	}
	snprintf(g_state.flash, sizeof(g_state.flash), "%s", msg);
	g_state.flash_timer = SECONDS(2);
}

void game_flash_update(void)
{
	if (g_state.flash_timer > 0 && --g_state.flash_timer == 0)
		g_state.flash[0] = '\0';
}

int game_room_time_limit(void)
{
	/* 30 seconds, shrinking by 2.5 per level, never below 15. */
	int limit = SECONDS(30) - (g_state.level - 1) * SECONDS(5) / 2;
	return limit < SECONDS(15) ? SECONDS(15) : limit;
}

int game_enemy_bonus_speed(void)
{
	int bonus = (g_state.level - 1) / 2;
	return bonus > 3 ? 3 : bonus;
}
