#include "birdcage.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../player.h"
#include "../sprites.h"
#include "../sound.h"
#include "alley.h"
#include <stdlib.h>
#include <string.h>

#define PLATFORM_COUNT 5

struct birdcage_data {
	struct player cat;
	int timer;
	int score;
	int plat_x[PLATFORM_COUNT];
	int plat_y[PLATFORM_COUNT];
	int plat_w[PLATFORM_COUNT];
	int birds[4];
	int bird_x[4], bird_y[4], bird_vx[4];
	int birds_collected;
};

static void birdcage_enter(struct scene *s)
{
	struct birdcage_data *d = calloc(1, sizeof(*d));
	player_init(&d->cat);
	d->cat.x = 150;
	d->cat.y = 140;
	d->cat.on_ground = 1;

	for (int i = 0; i < PLATFORM_COUNT; i++) {
		d->plat_x[i] = 20 + i * 55;
		d->plat_y[i] = 160 - i * 25;
		d->plat_w[i] = 40;
	}
	for (int i = 0; i < 4; i++) {
		d->birds[i] = 1;
		d->bird_x[i] = 50 + i * 60;
		d->bird_y[i] = 40 + i * 15;
		d->bird_vx[i] = (i % 2) ? 1 : -1;
	}
	s->data = d;
}

static void birdcage_update(struct scene *s, float dt)
{
	struct birdcage_data *d = (struct birdcage_data *)s->data;
	d->timer++;
	(void)dt;

	int dx = 0;
	if (input_key_pressed(KEY_LEFT))  dx = -2;
	if (input_key_pressed(KEY_RIGHT)) dx = 2;
	if (input_key_just_pressed(KEY_ACTION)) {
		player_jump(&d->cat);
		sound_play_tone(600, 80);
	}

	if (dx == 0 && d->cat.on_ground)
		d->cat.sitting = 1;
	else
		d->cat.sitting = 0;

	player_move(&d->cat, dx, 0);
	d->cat.on_ground = 0;
	for (int i = 0; i < PLATFORM_COUNT; i++) {
		if (d->cat.x + 8 > d->plat_x[i] && d->cat.x < d->plat_x[i] + d->plat_w[i] &&
		    d->cat.y + 20 >= d->plat_y[i] && d->cat.y + 20 <= d->plat_y[i] + 6 &&
		    d->cat.vy >= 0) {
			d->cat.y = d->plat_y[i] - 20;
			d->cat.vy = 0;
			d->cat.jumping = 0;
			d->cat.falling = 0;
			d->cat.on_ground = 1;
		}
	}
	if (d->cat.y > 150) { d->cat.y = 150; d->cat.vy = 0; d->cat.jumping = 0; d->cat.falling = 0; d->cat.on_ground = 1; }
	player_update(&d->cat, dt);

	for (int i = 0; i < 4; i++) {
		if (!d->birds[i]) continue;
		d->bird_x[i] += d->bird_vx[i];
		if (d->bird_x[i] < 10 || d->bird_x[i] > 300) d->bird_vx[i] = -d->bird_vx[i];

		if (abs(d->cat.x - d->bird_x[i]) < 12 && abs(d->cat.y - d->bird_y[i]) < 12) {
			d->birds[i] = 0;
			d->birds_collected++;
			sound_play_tone(1000, 150);
		}
	}

	if (d->birds_collected >= 4) {
		g_state.score += 200;
		game_room_done(ROOM_BIRDCAGE);
		scene_replace(alley_create());
	}
	if (d->timer > 1800) {
		g_state.lives--;
		scene_replace(alley_create());
	}
}

static void birdcage_render(struct scene *s)
{
	struct birdcage_data *d = (struct birdcage_data *)s->data;

	render_fill(PAL_BLACK);
	for (int i = 10; i < SCREEN_H; i += 2) {
		render_fill_rect(0, i, 4, 1, PAL_LGRAY);
		render_fill_rect(SCREEN_W - 4, i, 4, 1, PAL_LGRAY);
	}
	for (int i = 0; i < PLATFORM_COUNT; i++) {
		render_fill_rect(d->plat_x[i], d->plat_y[i], d->plat_w[i], 4, PAL_GREEN);
	}

	for (int i = 0; i < 4; i++) {
		if (d->birds[i])
			render_fill_rect(d->bird_x[i], d->bird_y[i], 6, 6, PAL_YELLOW);
	}

	player_render(&d->cat);

	render_text("BIRDCAGE", 4, 2);
	render_text("BIRDS:", 200, 2);
	render_number(d->birds_collected, 250, 2, 1);
	render_text("/4", 260, 2);
	render_text("JUMP ON PLATFORMS!", 90, 185);
}

static void birdcage_exit(struct scene *s) { free(s->data); }

struct scene *birdcage_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));
	s->enter  = birdcage_enter;
	s->update = birdcage_update;
	s->render = birdcage_render;
	s->exit   = birdcage_exit;
	return s;
}
