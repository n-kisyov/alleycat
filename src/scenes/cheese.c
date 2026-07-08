#include "cheese.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../player.h"
#include "../sprites.h"
#include "../sound.h"
#include "alley.h"
#include <stdlib.h>
#include <string.h>

struct cheese_data {
	struct player cat;
	int timer;
	int cheese_x[6], cheese_y[6], cheese_active[6];
	int mice_x[3], mice_y[3], mice_vx[3], mice_vy[3];
	int cheese_collected;
	int warning_timer;
};

static void cheese_enter(struct scene *s)
{
	struct cheese_data *d = calloc(1, sizeof(*d));
	player_init(&d->cat);
	d->cat.x = 150;
	d->cat.y = 140;

	for (int i = 0; i < 6; i++) {
		d->cheese_x[i] = 30 + rand() % 260;
		d->cheese_y[i] = 40 + rand() % 100;
		d->cheese_active[i] = 1;
	}
	for (int i = 0; i < 3; i++) {
		d->mice_x[i] = 50 + i * 80;
		d->mice_y[i] = 100;
		d->mice_vx[i] = (rand() % 3 - 1) * 2;
		d->mice_vy[i] = (rand() % 3 - 1) * 2;
	}
	s->data = d;
}

static void cheese_update(struct scene *s, float dt)
{
	struct cheese_data *d = (struct cheese_data *)s->data;
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

	for (int i = 0; i < 6; i++) {
		if (!d->cheese_active[i]) continue;
		if (abs(d->cat.x - d->cheese_x[i]) < 10 && abs(d->cat.y - d->cheese_y[i]) < 10) {
			d->cheese_active[i] = 0;
			d->cheese_collected++;
			sound_play_tone(900, 100);
		}
	}

	for (int i = 0; i < 3; i++) {
		d->mice_x[i] += d->mice_vx[i];
		d->mice_y[i] += d->mice_vy[i];
		if (d->mice_x[i] < 10 || d->mice_x[i] > 310) d->mice_vx[i] = -d->mice_vx[i];
		if (d->mice_y[i] < 30 || d->mice_y[i] > 150) d->mice_vy[i] = -d->mice_vy[i];
		if (rand() % 60 == 0) {
			d->mice_vx[i] = (rand() % 3 - 1) * 2;
			d->mice_vy[i] = (rand() % 3 - 1) * 2;
		}

		if (abs(d->cat.x - d->mice_x[i]) < 10 && abs(d->cat.y - d->mice_y[i]) < 12) {
			d->warning_timer++;
			if (d->warning_timer > 60) {
				g_state.lives--;
				scene_replace(alley_create());
				return;
			}
		}
	}
	if (d->warning_timer > 0) d->warning_timer--;

	if (d->cheese_collected >= 6) {
		g_state.score += 300;
		game_room_done(ROOM_CHEESE);
		scene_replace(alley_create());
	}
	if (d->timer > 1800) {
		g_state.lives--;
		scene_replace(alley_create());
	}
}

static void cheese_render(struct scene *s)
{
	struct cheese_data *d = (struct cheese_data *)s->data;

	render_fill(PAL_BLACK);
	render_fill_rect(0, 150, SCREEN_W, 50, PAL_BROWN);
	render_fill_rect(10, 10, SCREEN_W - 20, 140, PAL_LGRAY);

	for (int i = 0; i < 6; i++) {
		if (d->cheese_active[i])
			render_fill_rect(d->cheese_x[i] - 2, d->cheese_y[i] - 2, 5, 5, PAL_YELLOW);
	}

	for (int i = 0; i < 3; i++) {
		render_fill_rect(d->mice_x[i] - 3, d->mice_y[i] - 3, 6, 6, PAL_LRED);
	}

	player_render(&d->cat);

	render_text("CHEESE ROOM", 4, 2);
	render_text("GOT:", 200, 2);
	render_number(d->cheese_collected, 240, 2, 1);
	render_text("/6", 250, 2);

	if (d->warning_timer > 0) render_text("WATCH THE MICE!", 110, 180);
	render_text("COLLECT ALL CHEESE!", 80, 185);
}

static void cheese_exit(struct scene *s) { free(s->data); }

struct scene *cheese_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));
	s->enter  = cheese_enter;
	s->update = cheese_update;
	s->render = cheese_render;
	s->exit   = cheese_exit;
	return s;
}
