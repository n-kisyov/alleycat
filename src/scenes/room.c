#include "room.h"
#include "gameover.h"
#include "../game.h"
#include "../render.h"
#include "../sound.h"
#include <stdio.h>

void room_succeed(int room, int points)
{
	char msg[FLASH_MAX];

	game_add_score(points);
	game_room_done(room);
	sound_play_tone(1200, 200);
	snprintf(msg, sizeof(msg), "%s CLEAR", game_room_name(room));
	game_flash(msg);
	scene_request_pop();
}

void room_fail(const char *reason)
{
	sound_play_tone(200, 400);
	if (game_lose_life()) {
		game_flash(NULL);
		scene_request_reset(gameover_create());
		return;
	}
	game_flash(reason ? reason : "TRY AGAIN");
	scene_request_pop();
}

void room_leave(void)
{
	game_start_room(-1);
	game_flash(NULL);
	scene_request_pop();
}

void room_draw_walls(uint8_t floor_color)
{
	render_fill(CGA_BLACK);
	render_fill_rect(0, ROOM_CEILING_Y, SCREEN_W, ROOM_FLOOR_Y - ROOM_CEILING_Y,
	                 floor_color);
	render_rect(0, ROOM_CEILING_Y, SCREEN_W, ROOM_FLOOR_Y - ROOM_CEILING_Y,
	            CGA_BLACK);
	render_fill_rect(0, ROOM_FLOOR_Y, SCREEN_W, ROOM_HUD_Y - ROOM_FLOOR_Y,
	                 CGA_BLACK);
}

void room_draw_hud(int room, int collected, int total, int ticks_left)
{
	int seconds = ticks_left / LOGIC_HZ;

	/* Glyph tiles are light cyan, so text only reads cleanly on a cyan
	 * band.  That is how the original gets black letters. */
	render_fill_rect(0, 0, SCREEN_W, ROOM_CEILING_Y, CGA_CYAN);
	render_text(game_room_name(room), 4, 4);

	render_number(collected, 152, 4, 1);
	render_text("-", 160, 4);
	render_number(total, 168, 4, 1);

	render_text("TIME", 216, 4);
	if (seconds < 0)
		seconds = 0;
	/* Flash the clock once it is genuinely tight. */
	if (seconds > 5 || (g_state.ticks / (LOGIC_HZ / 4)) % 2 == 0)
		render_number(seconds, 256, 4, 2);

	render_fill_rect(0, ROOM_HUD_Y, SCREEN_W, ROOM_HUD_H, CGA_CYAN);
	render_text("SCORE", 4, ROOM_HUD_Y + 4);
	render_number(g_state.score, 48, ROOM_HUD_Y + 4, 6);
	render_text("LIVES", 184, ROOM_HUD_Y + 4);
	render_number(g_state.lives, 232, ROOM_HUD_Y + 4, 2);
}
