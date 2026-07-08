#include "kennel.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../player.h"
#include "../sprites.h"
#include "../sound.h"
#include "alley.h"
#include <stdlib.h>
#include <string.h>

struct kennel_data {
	struct player cat;
	int dog_x, dog_y, dog_dir, dog_timer;
	int timer;
	int food_collected;
	int food_x[3], food_y[3];
	int food_active[3];
	int warning_timer;
};

static void kennel_enter(struct scene *s)
{
	struct kennel_data *d = calloc(1, sizeof(*d));
	player_init(&d->cat);
	d->cat.x = 150;
	d->cat.y = 130;
	d->dog_x = 50;
	d->dog_y = 100;
	d->dog_dir = 1;
	for (int i = 0; i < 3; i++) {
		d->food_x[i] = 60 + i * 100;
		d->food_y[i] = 100 + (rand() % 40);
		d->food_active[i] = 1;
	}
	s->data = d;
}

static void kennel_update(struct scene *s, float dt)
{
	struct kennel_data *d = (struct kennel_data *)s->data;
	d->timer++;

	(void)dt;
	int dx = 0;
	int dy = 0;
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

	d->dog_timer++;
	if (d->dog_timer > 3) {
		d->dog_timer = 0;
		int tx = d->cat.x;
		int ty = d->cat.y + 10;
		if (d->dog_x < tx) d->dog_x += 1;
		if (d->dog_x > tx) d->dog_x -= 1;
		if (d->dog_y < ty) d->dog_y += 1;
		if (d->dog_y > ty) d->dog_y -= 1;
	}

	for (int i = 0; i < 3; i++) {
		if (!d->food_active[i]) continue;
		if (abs(d->cat.x - d->food_x[i]) < 10 && abs(d->cat.y - d->food_y[i]) < 10) {
			d->food_active[i] = 0;
			d->food_collected++;
			sound_play_tone(800, 100);
		}
	}

	if (abs(d->cat.x - d->dog_x) < 10 && abs(d->cat.y - d->dog_y) < 12) {
		d->warning_timer++;
		if (d->warning_timer > 60) {
			g_state.lives--;
			scene_replace(alley_create());
			return;
		}
	} else {
		d->warning_timer = 0;
	}

	if (d->food_collected >= 3) {
		g_state.score += 150;
		game_room_done(ROOM_KENNEL);
		scene_replace(alley_create());
	}
	if (d->timer > 1800) {
		g_state.lives--;
		scene_replace(alley_create());
	}
}

static void kennel_render(struct scene *s)
{
	struct kennel_data *d = (struct kennel_data *)s->data;

	render_fill(PAL_BLACK);
	render_fill_rect(0, 150, SCREEN_W, 50, PAL_BROWN);
	render_fill_rect(10, 10, SCREEN_W - 20, 140, PAL_DGRAY);

	for (int i = 0; i < 3; i++) {
		if (d->food_active[i]) {
			render_fill_rect(d->food_x[i] - 3, d->food_y[i] - 3, 6, 6, PAL_WHITE);
		}
	}

	render_fill_rect(d->dog_x, d->dog_y, 12, 12, PAL_RED);
	player_render(&d->cat);

	render_text("KENNEL", 4, 2);
	render_text("FOOD:", 200, 2);
	render_number(d->food_collected, 240, 2, 1);
	render_text("/3", 250, 2);

	if (d->warning_timer > 0) {
		render_text("WATCH OUT!", 130, 180);
	}
	render_text("AVOID THE DOG!", 100, 185);
}

static void kennel_exit(struct scene *s) { free(s->data); }

struct scene *kennel_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));
	s->enter  = kennel_enter;
	s->update = kennel_update;
	s->render = kennel_render;
	s->exit   = kennel_exit;
	return s;
}
