#include "love.h"
#include "room.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../player.h"
#include "../sprites.h"
#include "../sound.h"
#include <stdlib.h>
#include <string.h>

#define HEART_COUNT 5
#define CAT_GROUND  (ROOM_FLOOR_Y - 11)

enum love_stage { STAGE_COLLECT = 0, STAGE_MEET, STAGE_DONE };

struct love_data {
	struct player cat;
	struct { int x, y, active; } heart[HEART_COUNT];
	int partner_x, partner_y;
	int timer;
	int taken;
	int stage;
	int stage_timer;
};

static void love_enter(struct scene *s)
{
	struct love_data *d = calloc(1, sizeof(*d));
	int i;

	if (!d)
		return;

	player_init(&d->cat);
	player_set_pos(&d->cat, 32, CAT_GROUND);
	player_set_limits(&d->cat, 8, SCREEN_W - 8);

	d->partner_x = SCREEN_W - 56;
	d->partner_y = CAT_GROUND;

	for (i = 0; i < HEART_COUNT; i++) {
		d->heart[i].x = 30 + i * 54;
		d->heart[i].y = ROOM_CEILING_Y + 16 + rand() % 40;
		d->heart[i].active = 1;
	}
	s->data = d;
}

static void love_update(struct scene *s)
{
	struct love_data *d = (struct love_data *)s->data;
	struct rect cat_box, partner_box;
	int dx = 0, i;

	if (!d)
		return;

	/* A little more time here: there are two stages to get through. */
	if (++d->timer > game_room_time_limit() + SECONDS(10)) {
		room_fail("TIME UP");
		return;
	}

	if (d->stage == STAGE_DONE) {
		if (++d->stage_timer > SECONDS(2)) {
			room_succeed(ROOM_LOVE, 250 + 50 * g_state.level);
			return;
		}
		player_set_input(&d->cat, 0);
		player_update(&d->cat, CAT_GROUND);
		return;
	}

	if (input_key_pressed(KEY_LEFT))  dx -= PLAYER_SPEED;
	if (input_key_pressed(KEY_RIGHT)) dx += PLAYER_SPEED;
	if (input_key_just_pressed(KEY_ACTION))
		player_jump(&d->cat);

	player_set_input(&d->cat, dx);
	player_update(&d->cat, CAT_GROUND);
	cat_box = player_bounds(&d->cat);

	if (d->stage == STAGE_COLLECT) {
		for (i = 0; i < HEART_COUNT; i++) {
			struct rect hr;

			if (!d->heart[i].active)
				continue;
			hr.x = d->heart[i].x;
			hr.y = d->heart[i].y;
			hr.w = spr_heart.w;
			hr.h = spr_heart.h;
			if (rect_overlap(cat_box, hr)) {
				d->heart[i].active = 0;
				d->taken++;
				sound_play_tone(500 + d->taken * 100, 160);
			}
		}
		if (d->taken >= HEART_COUNT) {
			d->stage = STAGE_MEET;
			d->stage_timer = 0;
			sound_play_tone(900, 200);
		}
		return;
	}

	/* Stage two: walk to the other cat under your own steam. */
	partner_box.x = d->partner_x;
	partner_box.y = d->partner_y;
	partner_box.w = spr_cat_walk_left[0].w;
	partner_box.h = spr_cat_walk_left[0].h;

	if (rect_overlap(cat_box, partner_box)) {
		d->stage = STAGE_DONE;
		d->stage_timer = 0;
		sound_play_tone(1200, 200);
		sound_play_tone(1400, 400);
	}
}

static void love_render(struct scene *s)
{
	struct love_data *d = (struct love_data *)s->data;
	int i;

	if (!d)
		return;

	room_draw_walls(CGA_MAGENTA);

	if (d->stage == STAGE_COLLECT) {
		for (i = 0; i < HEART_COUNT; i++)
			if (d->heart[i].active)
				render_sprite_stencil(&spr_heart, d->heart[i].x, d->heart[i].y, CGA_WHITE);
	} else {
		/* Hearts rise between the pair once they have met. */
		for (i = 0; i < HEART_COUNT; i++) {
			int hx = d->partner_x - 40 + i * 14;
			int hy = ROOM_FLOOR_Y - 24 - ((d->timer * 2 + i * 23) % 80);
			render_sprite_stencil(&spr_heart, hx, hy, CGA_WHITE);
		}
	}

	render_sprite_keyed(&spr_cat_walk_left[0], d->partner_x, d->partner_y);
	player_render(&d->cat);

	room_draw_hud(ROOM_LOVE, d->taken, HEART_COUNT,
	              game_room_time_limit() + SECONDS(10) - d->timer);

	if (d->stage == STAGE_MEET)
		render_banner("GO TO YOUR LOVE", 120);
	else if (d->stage == STAGE_DONE)
		render_banner("PURR", 120);
}

static void love_exit(struct scene *s)
{
	free(s->data);
	s->data = NULL;
}

static void love_keydown(struct scene *s, SDL_Keycode key)
{
	(void)s;
	if (key == SDLK_ESCAPE)
		room_leave();
}

struct scene *love_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));

	if (!s)
		return NULL;
	s->enter   = love_enter;
	s->update  = love_update;
	s->render  = love_render;
	s->exit    = love_exit;
	s->keydown = love_keydown;
	return s;
}
