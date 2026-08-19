#include "fishbowl.h"
#include "room.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../player.h"
#include "../sprites.h"
#include "../sound.h"
#include <stdlib.h>
#include <string.h>

#define FISH_COUNT   4
#define BUBBLE_COUNT 20
#define SWIM_SPEED   2

struct fishbowl_data {
	struct player cat;
	struct { int x, y, vy, taken; } fish[FISH_COUNT];
	int bubble_x[BUBBLE_COUNT];
	int bubble_y[BUBBLE_COUNT];
	int timer;
	int taken;
};

static struct rect water(void)
{
	struct rect r;
	r.x = 8;
	r.y = ROOM_CEILING_Y + 4;
	r.w = SCREEN_W - 16;
	r.h = ROOM_FLOOR_Y - r.y;
	return r;
}

static void fishbowl_enter(struct scene *s)
{
	struct fishbowl_data *d = calloc(1, sizeof(*d));
	struct rect w = water();
	int i;

	if (!d)
		return;

	player_init(&d->cat);
	player_set_swim(&d->cat, 1);
	player_set_pos(&d->cat, SCREEN_W / 2, w.y + w.h / 2);

	for (i = 0; i < FISH_COUNT; i++) {
		d->fish[i].x  = w.x + 20 + rand() % (w.w - 60);
		d->fish[i].y  = w.y + 8 + rand() % (w.h - 24);
		d->fish[i].vy = (rand() % 3 - 1) * 2;
	}
	for (i = 0; i < BUBBLE_COUNT; i++) {
		d->bubble_x[i] = w.x + rand() % w.w;
		d->bubble_y[i] = w.y + rand() % w.h;
	}
	s->data = d;
}

static void fishbowl_update(struct scene *s)
{
	struct fishbowl_data *d = (struct fishbowl_data *)s->data;
	struct rect w = water();
	struct rect cat_box;
	int dx = 0, dy = 0, i;

	if (!d)
		return;

	if (++d->timer > game_room_time_limit()) {
		room_fail("TIME UP");
		return;
	}

	if (input_key_pressed(KEY_LEFT))  dx -= SWIM_SPEED;
	if (input_key_pressed(KEY_RIGHT)) dx += SWIM_SPEED;
	if (input_key_pressed(KEY_UP))    dy -= SWIM_SPEED;
	if (input_key_pressed(KEY_DOWN))  dy += SWIM_SPEED;

	player_swim_update(&d->cat, dx, dy, w);
	cat_box = player_bounds(&d->cat);

	for (i = 0; i < FISH_COUNT; i++) {
		struct rect fr;

		if (d->fish[i].taken)
			continue;

		d->fish[i].y += d->fish[i].vy;
		if (d->fish[i].y < w.y || d->fish[i].y + spr_fish.h > w.y + w.h)
			d->fish[i].vy = -d->fish[i].vy;
		if (rand() % 60 == 0)
			d->fish[i].vy = (rand() % 3 - 1) * 2;

		fr.x = d->fish[i].x;
		fr.y = d->fish[i].y;
		fr.w = spr_fish.w;
		fr.h = spr_fish.h;
		if (rect_overlap(cat_box, fr)) {
			d->fish[i].taken = 1;
			d->taken++;
			sound_play_tone(1200 + d->taken * 100, 120);
		}
	}

	for (i = 0; i < BUBBLE_COUNT; i++) {
		d->bubble_y[i] -= 1 + rand() % 2;
		if (d->bubble_y[i] < w.y) {
			d->bubble_y[i] = w.y + w.h;
			d->bubble_x[i] = w.x + rand() % w.w;
		}
	}

	if (d->taken >= FISH_COUNT) {
		room_succeed(ROOM_FISHBOWL, 100 + 50 * g_state.level);
		return;
	}
}

static void fishbowl_render(struct scene *s)
{
	struct fishbowl_data *d = (struct fishbowl_data *)s->data;
	struct rect w = water();
	int i;

	if (!d)
		return;

	render_fill(CGA_BLACK);
	render_fill_rect(w.x, w.y, w.w, w.h, CGA_CYAN);
	render_rect(w.x - 1, w.y - 1, w.w + 2, w.h + 2, CGA_WHITE);

	for (i = 0; i < BUBBLE_COUNT; i++)
		render_fill_rect(d->bubble_x[i], d->bubble_y[i], 2, 2, CGA_WHITE);

	for (i = 0; i < FISH_COUNT; i++)
		if (!d->fish[i].taken)
			render_sprite_stencil(&spr_fish, d->fish[i].x, d->fish[i].y, CGA_WHITE);

	player_render(&d->cat);
	room_draw_hud(ROOM_FISHBOWL, d->taken, FISH_COUNT,
	              game_room_time_limit() - d->timer);
}

static void fishbowl_exit(struct scene *s)
{
	free(s->data);
	s->data = NULL;
}

static void fishbowl_keydown(struct scene *s, SDL_Keycode key)
{
	(void)s;
	if (key == SDLK_ESCAPE)
		room_leave();
}

struct scene *fishbowl_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));

	if (!s)
		return NULL;
	s->enter   = fishbowl_enter;
	s->update  = fishbowl_update;
	s->render  = fishbowl_render;
	s->exit    = fishbowl_exit;
	s->keydown = fishbowl_keydown;
	return s;
}
