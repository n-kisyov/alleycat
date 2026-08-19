#include "cheese.h"
#include "room.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../player.h"
#include "../sprites.h"
#include "../sound.h"
#include <stdlib.h>
#include <string.h>

#define CHEESE_COUNT 6
#define MICE_COUNT   3
#define CAUGHT_GRACE SECONDS(1)
#define CAT_GROUND   (ROOM_FLOOR_Y - 11)

struct cheese_data {
	struct player cat;
	struct { int x, y, active; } cheese[CHEESE_COUNT];
	struct { int x, y, vx; } mouse[MICE_COUNT];
	int timer;
	int taken;
	int caught_timer;
};

static void cheese_enter(struct scene *s)
{
	struct cheese_data *d = calloc(1, sizeof(*d));
	int i;

	if (!d)
		return;

	player_init(&d->cat);
	player_set_pos(&d->cat, SCREEN_W / 2, CAT_GROUND);
	player_set_limits(&d->cat, 8, SCREEN_W - 8);

	for (i = 0; i < CHEESE_COUNT; i++) {
		d->cheese[i].x = 20 + (i % 3) * 100 + rand() % 40;
		d->cheese[i].y = ROOM_CEILING_Y + 24 + (i / 3) * 52;
		d->cheese[i].active = 1;
	}
	for (i = 0; i < MICE_COUNT; i++) {
		d->mouse[i].x  = 40 + i * 80;
		d->mouse[i].y  = ROOM_FLOOR_Y - spr_mouse.h;
		d->mouse[i].vx = (i % 2) ? 1 : -1;
	}
	s->data = d;
}

static void cheese_update(struct scene *s)
{
	struct cheese_data *d = (struct cheese_data *)s->data;
	struct rect cat_box;
	int dx = 0, i, touching = 0;

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

	for (i = 0; i < CHEESE_COUNT; i++) {
		struct rect cr;

		if (!d->cheese[i].active)
			continue;
		cr.x = d->cheese[i].x;
		cr.y = d->cheese[i].y;
		cr.w = spr_cheese.w;
		cr.h = spr_cheese.h;
		if (rect_overlap(cat_box, cr)) {
			d->cheese[i].active = 0;
			d->taken++;
			sound_play_tone(900 + d->taken * 60, 100);
		}
	}

	for (i = 0; i < MICE_COUNT; i++) {
		struct rect mr;

		d->mouse[i].x += d->mouse[i].vx * (1 + game_enemy_bonus_speed() / 2);
		if (d->mouse[i].x < 8 || d->mouse[i].x + spr_mouse.w > SCREEN_W - 8)
			d->mouse[i].vx = -d->mouse[i].vx;
		else if (rand() % 120 == 0)
			d->mouse[i].vx = -d->mouse[i].vx;

		mr.x = d->mouse[i].x;
		mr.y = d->mouse[i].y;
		mr.w = spr_mouse.w;
		mr.h = spr_mouse.h;
		if (rect_overlap(cat_box, mr))
			touching = 1;
	}

	if (touching) {
		if (++d->caught_timer > CAUGHT_GRACE) {
			room_fail("THE MICE GOT YOU");
			return;
		}
	} else if (d->caught_timer > 0) {
		d->caught_timer--;
	}

	if (d->taken >= CHEESE_COUNT) {
		room_succeed(ROOM_CHEESE, 300 + 50 * g_state.level);
		return;
	}
}

static void cheese_render(struct scene *s)
{
	struct cheese_data *d = (struct cheese_data *)s->data;
	int i;

	if (!d)
		return;

	room_draw_walls(CGA_CYAN);

	/* Skirting board, so the floor line reads. */
	render_fill_rect(0, ROOM_FLOOR_Y - 4, SCREEN_W, 4, CGA_MAGENTA);

	for (i = 0; i < CHEESE_COUNT; i++)
		if (d->cheese[i].active)
			render_sprite_stencil(&spr_cheese, d->cheese[i].x, d->cheese[i].y, CGA_WHITE);

	for (i = 0; i < MICE_COUNT; i++)
		render_sprite_stencil(&spr_mouse, d->mouse[i].x, d->mouse[i].y, CGA_BLACK);

	player_render(&d->cat);
	room_draw_hud(ROOM_CHEESE, d->taken, CHEESE_COUNT,
	              game_room_time_limit() - d->timer);

	if (d->caught_timer > 0)
		render_banner("JUMP", 120);
}

static void cheese_exit(struct scene *s)
{
	free(s->data);
	s->data = NULL;
}

static void cheese_keydown(struct scene *s, SDL_Keycode key)
{
	(void)s;
	if (key == SDLK_ESCAPE)
		room_leave();
}

struct scene *cheese_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));

	if (!s)
		return NULL;
	s->enter   = cheese_enter;
	s->update  = cheese_update;
	s->render  = cheese_render;
	s->exit    = cheese_exit;
	s->keydown = cheese_keydown;
	return s;
}
