#include "milk.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../player.h"
#include "../sprites.h"
#include "../sound.h"
#include "alley.h"
#include <stdlib.h>
#include <string.h>

#define MILK_COUNT 4

struct milk_data {
	struct player cat;
	int timer;
	int milk_x[MILK_COUNT], milk_y[MILK_COUNT], milk_active[MILK_COUNT];
	int broom_x, broom_y, broom_dir, broom_timer;
	int milk_collected;
	int warning_timer;
};

static void milk_enter(struct scene *s)
{
	struct milk_data *d = calloc(1, sizeof(*d));
	player_init(&d->cat);
	d->cat.x = 150;
	d->cat.y = 140;

	d->broom_x = 30;
	d->broom_y = 100;
	d->broom_dir = 1;
	d->broom_timer = 0;

	for (int i = 0; i < MILK_COUNT; i++) {
		d->milk_x[i] = 40 + i * 70;
		d->milk_y[i] = 120 + rand() % 25;
		d->milk_active[i] = 1;
	}
	s->data = d;
}

static void milk_update(struct scene *s, float dt)
{
	struct milk_data *d = (struct milk_data *)s->data;
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

	d->broom_timer++;
	if (d->broom_timer > 2) {
		d->broom_timer = 0;
		d->broom_x += d->broom_dir * 2;
		if (d->broom_x < 10 || d->broom_x > 300) d->broom_dir = -d->broom_dir;
	}

	for (int i = 0; i < MILK_COUNT; i++) {
		if (!d->milk_active[i]) continue;
		if (abs(d->cat.x - d->milk_x[i]) < 10 && abs(d->cat.y - d->milk_y[i]) < 12) {
			d->milk_active[i] = 0;
			d->milk_collected++;
			sound_play_tone(700 + i * 100, 120);
		}
	}

	if (abs(d->cat.x - d->broom_x) < 10 && abs(d->cat.y - d->broom_y) < 14) {
		d->warning_timer++;
		if (d->warning_timer > 60) {
			g_state.lives--;
			scene_replace(alley_create());
			return;
		}
	} else {
		d->warning_timer = 0;
	}

	if (d->milk_collected >= MILK_COUNT) {
		g_state.score += 200;
		game_room_done(ROOM_MILK);
		scene_replace(alley_create());
	}
	if (d->timer > 1800) {
		g_state.lives--;
		scene_replace(alley_create());
	}
}

static void milk_render(struct scene *s)
{
	struct milk_data *d = (struct milk_data *)s->data;

	render_fill(PAL_BLACK);
	render_fill_rect(0, 150, SCREEN_W, 50, PAL_BROWN);
	render_fill_rect(10, 10, SCREEN_W - 20, 140, PAL_LBLUE);

	for (int i = 0; i < MILK_COUNT; i++) {
		if (d->milk_active[i])
			render_fill_rect(d->milk_x[i] - 3, d->milk_y[i] - 2, 6, 8, PAL_WHITE);
	}

	render_fill_rect(d->broom_x - 2, d->broom_y - 8, 4, 16, PAL_BROWN);
	player_render(&d->cat);

	render_text("MILK ROOM", 4, 2);
	render_text("GOT:", 200, 2);
	render_number(d->milk_collected, 240, 2, 1);
	render_text("/4", 250, 2);

	if (d->warning_timer > 0) render_text("AVOID THE BROOM!", 100, 180);
	render_text("DRINK THE MILK!", 110, 185);
}

static void milk_exit(struct scene *s) { free(s->data); }

struct scene *milk_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));
	s->enter  = milk_enter;
	s->update = milk_update;
	s->render = milk_render;
	s->exit   = milk_exit;
	return s;
}
