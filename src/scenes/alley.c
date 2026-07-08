#include "alley.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../player.h"
#include "../sprites.h"
#include "../sound.h"
#include "fishbowl.h"
#include "kennel.h"
#include "birdcage.h"
#include "cheese.h"
#include "love.h"
#include "milk.h"
#include <stdlib.h>
#include <string.h>

#define WINDOW_COUNT 6

typedef struct {
	int x, y, w, h;
	int room_type;
	int open;
} window_t;

struct alley_data {
	struct player cat;
	window_t windows[WINDOW_COUNT];
	int trash_x[5];
	int timer;
	int other_cat_x[3];
	int other_cat_dir[3];
	int entering_room;
	int room_timer;
	int selected_room;
};

static void alley_enter(struct scene *s)
{
	struct alley_data *d = calloc(1, sizeof(*d));
	player_init(&d->cat);
	d->cat.x = 10;
	d->cat.y = 140;

	d->windows[0] = (window_t){ 40,  122, 30, 24, ROOM_FISHBOWL, 1 };
	d->windows[1] = (window_t){ 90,  122, 30, 24, ROOM_KENNEL,   1 };
	d->windows[2] = (window_t){ 140, 122, 30, 24, ROOM_BIRDCAGE, 1 };
	d->windows[3] = (window_t){ 190, 122, 30, 24, ROOM_CHEESE,   1 };
	d->windows[4] = (window_t){ 240, 122, 30, 24, ROOM_LOVE,     1 };
	d->windows[5] = (window_t){ 285, 122, 30, 24, ROOM_MILK,     1 };

	d->trash_x[0] = 10;
	d->trash_x[1] = 80;
	d->trash_x[2] = 145;
	d->trash_x[3] = 200;
	d->trash_x[4] = 255;

	for (int i = 0; i < 3; i++) {
		d->other_cat_x[i] = 100 + i * 60;
		d->other_cat_dir[i] = (i % 2) ? 1 : -1;
	}

	s->data = d;
	srand((unsigned int)SDL_GetTicks());
}

static void alley_update(struct scene *s, float dt)
{
	struct alley_data *d = (struct alley_data *)s->data;
	d->timer++;

	if (d->entering_room) {
		d->room_timer--;
		if (d->room_timer <= 0) {
			struct scene *room = NULL;
			switch (d->selected_room) {
			case ROOM_FISHBOWL: room = fishbowl_create(); break;
			case ROOM_KENNEL:   room = kennel_create(); break;
			case ROOM_BIRDCAGE: room = birdcage_create(); break;
			case ROOM_CHEESE:   room = cheese_create(); break;
			case ROOM_LOVE:     room = love_create(); break;
			case ROOM_MILK:     room = milk_create(); break;
			}
			if (room) scene_replace(room);
		}
		return;
	}

	int dx = 0;
	if (input_key_pressed(KEY_LEFT))  dx = -1;
	if (input_key_pressed(KEY_RIGHT)) dx = 1;

	if (dx == 0 && d->cat.on_ground)
		d->cat.sitting = 1;
	else
		d->cat.sitting = 0;

	if (input_key_just_pressed(KEY_ACTION))
		player_jump(&d->cat);

	player_move(&d->cat, dx, 0);
	player_update(&d->cat, dt);

	for (int i = 0; i < WINDOW_COUNT; i++) {
		window_t *w = &d->windows[i];
		if (!w->open || game_room_is_done(w->room_type)) continue;
		if (d->cat.x + 20 > w->x && d->cat.x < w->x + w->w &&
		    d->cat.y < w->y + w->h && d->cat.y + 11 > w->y) {
			if (input_key_just_pressed(KEY_UP) || input_key_just_pressed(KEY_ACTION)) {
				d->entering_room = 1;
				d->selected_room = w->room_type;
				d->room_timer = 15;
				game_start_room(w->room_type);
				sound_play_tone(600, 150);
			}
		}
	}

	for (int i = 0; i < 3; i++) {
		d->other_cat_x[i] += d->other_cat_dir[i];
		if (d->other_cat_x[i] < 10 || d->other_cat_x[i] > 300)
			d->other_cat_dir[i] = -d->other_cat_dir[i];
		if (rand() % 100 < 2)
			d->other_cat_dir[i] = -d->other_cat_dir[i];
	}
}

static void alley_render(struct scene *s)
{
	struct alley_data *d = (struct alley_data *)s->data;

	render_fill(PAL_BLACK);

	render_fill_rect(0, 0, SCREEN_W, 120, PAL_DGRAY);

	for (int i = 0; i < WINDOW_COUNT; i++) {
		window_t *w = &d->windows[i];
		uint8_t c = w->open && !game_room_is_done(w->room_type)
			? PAL_YELLOW : PAL_BROWN;
		render_fill_rect(w->x, w->y, w->w, w->h, PAL_BLACK);
		render_rect(w->x, w->y, w->w, w->h, c);
		if (game_room_is_done(w->room_type)) {
			render_line(w->x, w->y, w->x + w->w, w->y + w->h, PAL_RED);
			render_line(w->x + w->w, w->y, w->x, w->y + w->h, PAL_RED);
		}
	}

	render_fill_rect(0, 115, SCREEN_W, 8, PAL_BROWN);
	render_fill_rect(0, 148, SCREEN_W, 10, PAL_BROWN);
	render_fill_rect(0, 150, SCREEN_W, 2, PAL_LGRAY);

	for (int i = 0; i < 40; i++)
		render_sprite(sprite_fence_top[i % 4], i * 8, 110, 8, 8);

	for (int i = 0; i < 5; i++) {
		render_sprite(sprite_trashcan_lid, d->trash_x[i], 82, 40, 12);
		render_sprite(sprite_trashcan_mid, d->trash_x[i] + 4, 94, 32, 8);
		render_sprite(sprite_trashcan_bot, d->trash_x[i] + 4, 102, 32, 11);
	}

	render_fill_rect(0, SCREEN_H - 16, SCREEN_W, 16, PAL_LGRAY);
	render_text("SCORE:", 4, SCREEN_H - 10);
	render_number(g_state.score, 48, SCREEN_H - 10, 6);
	render_text("LV:", 130, SCREEN_H - 10);
	render_number(g_state.level, 150, SCREEN_H - 10, 2);
	render_text("LIVES:", 174, SCREEN_H - 10);
	render_number(g_state.lives, 220, SCREEN_H - 10, 1);

	for (int i = 0; i < 3; i++) {
		const uint8_t *cs = (d->other_cat_dir[i] > 0) ?
			cat_walk_right[(d->timer / 8) % CAT_WALK_FRAMES] :
			cat_walk_left[(d->timer / 8) % CAT_WALK_FRAMES];
		render_sprite(cs, d->other_cat_x[i], 136, 24, 11);
	}

	player_render(&d->cat);

	if (d->entering_room)
		render_text("ENTERING...", 120, 50);
}

static void alley_exit(struct scene *s) { free(s->data); }

static void alley_keydown(struct scene *s, SDL_Keycode key) { (void)s; (void)key; }

struct scene *alley_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));
	s->enter   = alley_enter;
	s->update  = alley_update;
	s->render  = alley_render;
	s->exit    = alley_exit;
	s->keydown = alley_keydown;
	return s;
}
