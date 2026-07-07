#include "game.h"
#include "scene.h"
#include "scenes/title.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct game_state g_state;

void game_init(void)
{
	memset(&g_state, 0, sizeof(g_state));
	g_state.running = 1;
	g_state.lives = 3;
	g_state.level = 1;
	g_state.cats_remaining = 8;
	g_state.current_room = -1;
	srand((unsigned int)time(NULL));
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
	if (room >= 0 && room < ROOM_COUNT) {
		if (!g_state.rooms_completed[room]) {
			g_state.rooms_completed[room] = 1;
			g_state.rooms_done++;
			g_state.score += 100;
		}
	}
	g_state.current_room = -1;
}

int game_room_is_done(int room)
{
	if (room >= 0 && room < ROOM_COUNT)
		return g_state.rooms_completed[room];
	return 0;
}

int game_room_next(void)
{
	int i, available = 0;
	for (i = 0; i < ROOM_COUNT; i++) {
		if (!g_state.rooms_completed[i])
			available++;
	}
	if (available == 0) {
		g_state.level++;
		memset(g_state.rooms_completed, 0, sizeof(g_state.rooms_completed));
		g_state.rooms_done = 0;
		g_state.cats_remaining = 8;
		return ROOM_FISHBOWL + (rand() % ROOM_COUNT);
	}
	int pick = rand() % available + 1;
	for (i = 0; i < ROOM_COUNT; i++) {
		if (!g_state.rooms_completed[i]) pick--;
		if (pick == 0) return i;
	}
	return ROOM_FISHBOWL;
}
