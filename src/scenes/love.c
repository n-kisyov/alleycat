#include "love.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../player.h"
#include "../sprites.h"
#include "../sound.h"
#include "alley.h"
#include <stdlib.h>
#include <string.h>

struct love_data {
	struct player cat;
	int timer;
	int partner_x, partner_y;
	int heart_x[5], heart_y[5], heart_active[5];
	int hearts_collected;
	int stage;
	int stage_timer;
};

static void love_enter(struct scene *s)
{
	struct love_data *d = calloc(1, sizeof(*d));
	player_init(&d->cat);
	d->cat.x = 100;
	d->cat.y = 130;
	d->partner_x = 200;
	d->partner_y = 130;

	for (int i = 0; i < 5; i++) {
		d->heart_x[i] = 50 + i * 50;
		d->heart_y[i] = 50 + rand() % 60;
		d->heart_active[i] = 1;
	}
	s->data = d;
}

static void love_update(struct scene *s, float dt)
{
	struct love_data *d = (struct love_data *)s->data;
	d->timer++;
	(void)dt;

	int dx = 0, dy = 0;
	if (input_key_pressed(KEY_LEFT))  dx = -2;
	if (input_key_pressed(KEY_RIGHT)) dx = 2;
	if (input_key_pressed(KEY_UP))    dy = -2;
	if (input_key_pressed(KEY_DOWN))  dy = 2;
	if (input_key_just_pressed(KEY_ACTION)) player_jump(&d->cat);

	if (dx == 0 && dy == 0 && d->cat.on_ground)
		d->cat.sitting = 1;
	else
		d->cat.sitting = 0;

	player_move(&d->cat, dx, dy);
	player_update(&d->cat, dt);
	if (d->cat.y > 145) { d->cat.y = 145; d->cat.on_ground = 1; d->cat.vy = 0; d->cat.jumping = 0; d->cat.falling = 0; }

	if (d->stage == 0) {
		for (int i = 0; i < 5; i++) {
			if (!d->heart_active[i]) continue;
			if (abs(d->cat.x - d->heart_x[i]) < 12 && abs(d->cat.y - d->heart_y[i]) < 12) {
				d->heart_active[i] = 0;
				d->hearts_collected++;
				sound_play_tone(500 + i * 100, 200);
			}
		}
		if (d->hearts_collected >= 5) {
			d->stage = 1;
			d->stage_timer = 0;
		}
	} else {
		d->stage_timer++;
		if (d->cat.x > d->partner_x) d->cat.x--;
		if (d->cat.x < d->partner_x) d->cat.x++;
		if (abs(d->cat.x - d->partner_x) < 15 && abs(d->cat.y - d->partner_y) < 30) {
			sound_play_tone(1200, 500);
			g_state.score += 250;
			g_state.cats_remaining--;
			game_room_done(ROOM_LOVE);
			scene_replace(alley_create());
		}
	}

	if (d->timer > 2400) {
		g_state.lives--;
		scene_replace(alley_create());
	}
}

static void love_render(struct scene *s)
{
	struct love_data *d = (struct love_data *)s->data;

	render_fill(PAL_BLACK);
	render_fill_rect(0, 150, SCREEN_W, 50, PAL_BROWN);
	render_fill_rect(10, 10, SCREEN_W - 20, 140, PAL_LMAGENTA);

	if (d->stage == 0) {
		for (int i = 0; i < 5; i++) {
			if (d->heart_active[i])
				render_sprite(sprite_heart, d->heart_x[i], d->heart_y[i], 8, 8);
		}
		render_text("COLLECT HEARTS!", 100, 180);
	} else {
		render_fill_rect(90, 50, 140, 100, PAL_RED);
		render_fill_rect(120, 50, 80, 100, PAL_LRED);
	}

	const uint8_t *partner_spr = cat_walk_left[(d->timer / 12) % CAT_WALK_FRAMES];
	render_sprite(partner_spr, d->partner_x, d->partner_y, 24, 11);

	player_render(&d->cat);

	render_text("LOVE ROOM", 4, 2);
	render_text("HEARTS:", 200, 2);
	render_number(d->hearts_collected, 250, 2, 1);

	if (d->stage == 1) {
		render_text("MEET YOUR LOVE!", 110, 185);
	}
}

static void love_exit(struct scene *s) { free(s->data); }

struct scene *love_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));
	s->enter  = love_enter;
	s->update = love_update;
	s->render = love_render;
	s->exit   = love_exit;
	return s;
}
