#include "milk.h"
#include "room.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../player.h"
#include "../sprites.h"
#include "../sound.h"
#include <stdlib.h>
#include <string.h>

#define MILK_COUNT   4
#define CAUGHT_GRACE SECONDS(1)
#define CAT_GROUND   (ROOM_FLOOR_Y - 11)

struct milk_data {
	struct player cat;
	struct { int x, y, active; } milk[MILK_COUNT];
	int broom_x, broom_y, broom_dir, broom_timer;
	int timer;
	int taken;
	int caught_timer;
};

static void milk_enter(struct scene *s)
{
	struct milk_data *d = calloc(1, sizeof(*d));
	int i;

	if (!d)
		return;

	player_init(&d->cat);
	player_set_pos(&d->cat, SCREEN_W / 2, CAT_GROUND);
	player_set_limits(&d->cat, 8, SCREEN_W - 8);

	d->broom_x   = 20;
	d->broom_y   = ROOM_FLOOR_Y - spr_broom.h;
	d->broom_dir = 1;

	for (i = 0; i < MILK_COUNT; i++) {
		d->milk[i].x = 30 + i * 70 + rand() % 20;
		d->milk[i].y = ROOM_FLOOR_Y - spr_milk_bottle.h - (rand() % 30);
		d->milk[i].active = 1;
	}
	s->data = d;
}

static void milk_update(struct scene *s)
{
	struct milk_data *d = (struct milk_data *)s->data;
	struct rect cat_box, broom_box;
	int dx = 0, i;

	if (!d)
		return;

	if (++d->timer > game_room_time_limit()) {
		room_fail("TIME UP");
		return;
	}

	if (input_key_pressed(KEY_LEFT))  dx -= PLAYER_SPEED;
	if (input_key_pressed(KEY_RIGHT)) dx += PLAYER_SPEED;
	if (input_key_just_pressed(KEY_ACTION))
		player_jump(&d->cat);

	player_set_input(&d->cat, dx);
	player_update(&d->cat, CAT_GROUND);
	cat_box = player_bounds(&d->cat);

	if (++d->broom_timer > 2) {
		d->broom_timer = 0;
		d->broom_x += d->broom_dir * (2 + game_enemy_bonus_speed());
		if (d->broom_x < 8 || d->broom_x + spr_broom.w > SCREEN_W - 8)
			d->broom_dir = -d->broom_dir;
	}

	for (i = 0; i < MILK_COUNT; i++) {
		struct rect mr;

		if (!d->milk[i].active)
			continue;
		mr.x = d->milk[i].x;
		mr.y = d->milk[i].y;
		mr.w = spr_milk_bottle.w;
		mr.h = spr_milk_bottle.h;
		if (rect_overlap(cat_box, mr)) {
			d->milk[i].active = 0;
			d->taken++;
			sound_play_tone(700 + i * 100, 110);
		}
	}

	broom_box.x = d->broom_x;
	broom_box.y = d->broom_y;
	broom_box.w = spr_broom.w;
	broom_box.h = spr_broom.h;

	if (rect_overlap(cat_box, broom_box)) {
		if (++d->caught_timer > CAUGHT_GRACE) {
			room_fail("SWEPT OUT");
			return;
		}
	} else if (d->caught_timer > 0) {
		d->caught_timer--;
	}

	if (d->taken >= MILK_COUNT) {
		room_succeed(ROOM_MILK, 200 + 50 * g_state.level);
		return;
	}
}

static void milk_render(struct scene *s)
{
	struct milk_data *d = (struct milk_data *)s->data;
	int i;

	if (!d)
		return;

	room_draw_walls(CGA_CYAN);

	/* Counter top the bottles stand on. */
	render_fill_rect(0, ROOM_FLOOR_Y - 3, SCREEN_W, 3, CGA_WHITE);

	for (i = 0; i < MILK_COUNT; i++)
		if (d->milk[i].active)
			render_sprite_stencil(&spr_milk_bottle, d->milk[i].x, d->milk[i].y, CGA_WHITE);

	render_sprite_stencil(&spr_broom, d->broom_x, d->broom_y, CGA_BLACK);
	player_render(&d->cat);

	room_draw_hud(ROOM_MILK, d->taken, MILK_COUNT,
	              game_room_time_limit() - d->timer);

	if (d->caught_timer > 0)
		render_banner("JUMP", 120);
}

static void milk_exit(struct scene *s)
{
	free(s->data);
	s->data = NULL;
}

static void milk_keydown(struct scene *s, SDL_Keycode key)
{
	(void)s;
	if (key == SDLK_ESCAPE)
		room_leave();
}

struct scene *milk_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));

	if (!s)
		return NULL;
	s->enter   = milk_enter;
	s->update  = milk_update;
	s->render  = milk_render;
	s->exit    = milk_exit;
	s->keydown = milk_keydown;
	return s;
}
