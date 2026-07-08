#include "fishbowl.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../player.h"
#include "../sprites.h"
#include "../sound.h"
#include "alley.h"
#include <stdlib.h>
#include <string.h>

typedef struct { int x, y, vy, collected; } fish_t;

struct fishbowl_data {
	struct player cat;
	fish_t fishes[4];
	int timer;
	int score;
	int bubbles[20];
	int bubble_y[20];
};

static void fishbowl_enter(struct scene *s)
{
	struct fishbowl_data *d = calloc(1, sizeof(*d));
	player_init(&d->cat);
	d->cat.x = 150;
	d->cat.y = 160;
	d->cat.swimming = 1;
	d->cat.on_ground = 0;
	player_set_swim(&d->cat, 1);

	for (int i = 0; i < 4; i++) {
		d->fishes[i].x = rand() % 280 + 20;
		d->fishes[i].y = rand() % 120 + 30;
		d->fishes[i].vy = (rand() % 3 - 1) * 2;
		d->fishes[i].collected = 0;
	}
	for (int i = 0; i < 20; i++) {
		d->bubbles[i] = rand() % SCREEN_W;
		d->bubble_y[i] = rand() % SCREEN_H;
	}
	s->data = d;
}

static void fishbowl_update(struct scene *s, float dt)
{
	struct fishbowl_data *d = (struct fishbowl_data *)s->data;
	d->timer++;

	int dx = 0, dy = 0;
	if (input_key_pressed(KEY_LEFT))  dx = -2;
	if (input_key_pressed(KEY_RIGHT)) dx = 2;
	if (input_key_pressed(KEY_UP))    dy = -2;
	if (input_key_pressed(KEY_DOWN))  dy = 2;

	d->cat.x += dx;
	d->cat.y += dy;
	if (d->cat.x < 8) d->cat.x = 8;
	if (d->cat.x > SCREEN_W - 20) d->cat.x = SCREEN_W - 20;
	if (d->cat.y < 20) d->cat.y = 20;
	if (d->cat.y > SCREEN_H - 20) d->cat.y = SCREEN_H - 20;

	d->cat.anim_timer++;
	if (d->cat.anim_timer >= 6) {
		d->cat.anim_timer = 0;
		d->cat.anim_frame = (d->cat.anim_frame + 1) % 3;
	}
	if (dx > 0) d->cat.direction = PLAYER_DIR_RIGHT;
	else if (dx < 0) d->cat.direction = PLAYER_DIR_LEFT;

	for (int i = 0; i < 4; i++) {
		fish_t *f = &d->fishes[i];
		if (f->collected) continue;
		f->y += f->vy;
		if (f->y < 30 || f->y > 150) f->vy = -f->vy;
		if (rand() % 50 == 0) f->vy = (rand() % 3 - 1) * 2;

		if (abs(d->cat.x - f->x) < 14 && abs(d->cat.y - f->y) < 14) {
			f->collected = 1;
			d->score++;
			sound_play_tone(1200 + d->score * 100, 150);
		}
	}

	for (int i = 0; i < 20; i++) {
		d->bubble_y[i] -= (rand() % 2 + 1);
		if (d->bubble_y[i] < 0) {
			d->bubble_y[i] = SCREEN_H;
			d->bubbles[i] = rand() % SCREEN_W;
		}
	}

	int all_done = 1;
	for (int i = 0; i < 4; i++) if (!d->fishes[i].collected) all_done = 0;
	if (all_done) {
		g_state.score += d->score * 50;
		game_room_done(ROOM_FISHBOWL);
		scene_replace(alley_create());
	}
	if (d->timer > 1800) {
		g_state.lives--;
		scene_replace(alley_create());
	}
}

static void fishbowl_render(struct scene *s)
{
	struct fishbowl_data *d = (struct fishbowl_data *)s->data;

	render_fill(PAL_BLUE);
	render_fill_rect(8, 20, SCREEN_W - 16, SCREEN_H - 40, PAL_CYAN);

	render_text("FISHBOWL ROOM", 8, 4);
	render_text("SCORE:", 180, 4);
	render_number(d->score, 230, 4, 1);
	render_text("/4", 240, 4);

	for (int i = 0; i < 20; i++) {
		render_fill_rect(d->bubbles[i], d->bubble_y[i], 3, 3, PAL_WHITE);
	}

	for (int i = 0; i < 4; i++) {
		if (d->fishes[i].collected) continue;
		render_fill_rect(d->fishes[i].x, d->fishes[i].y, 6, 6, PAL_YELLOW);
	}

	player_render(&d->cat);

	render_text("COLLECT THE FISH!", 120, 185);
	int remaining = 0;
	for (int i = 0; i < 4; i++) if (!d->fishes[i].collected) remaining++;
	render_number(remaining, 200, 4, 1);
}

static void fishbowl_exit(struct scene *s) { free(s->data); }

struct scene *fishbowl_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));
	s->enter  = fishbowl_enter;
	s->update = fishbowl_update;
	s->render = fishbowl_render;
	s->exit   = fishbowl_exit;
	return s;
}
