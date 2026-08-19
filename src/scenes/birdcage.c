#include "birdcage.h"
#include "room.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../player.h"
#include "../sprites.h"
#include "../sound.h"
#include <stdlib.h>
#include <string.h>

#define PLATFORM_COUNT 5
#define BIRD_COUNT     4
#define PLATFORM_H     4
#define CAT_GROUND     (ROOM_FLOOR_Y - 11)

struct birdcage_data {
	struct player cat;
	struct { int x, y, w; } plat[PLATFORM_COUNT];
	struct { int x, y, vx, active; } bird[BIRD_COUNT];
	int timer;
	int taken;
};

static void birdcage_enter(struct scene *s)
{
	struct birdcage_data *d = calloc(1, sizeof(*d));
	int i;

	if (!d)
		return;

	player_init(&d->cat);
	player_set_pos(&d->cat, SCREEN_W / 2, CAT_GROUND);
	player_set_limits(&d->cat, 6, SCREEN_W - 6);

	for (i = 0; i < PLATFORM_COUNT; i++) {
		d->plat[i].x = 20 + i * 58;
		d->plat[i].y = ROOM_FLOOR_Y - 24 - i * 22;
		d->plat[i].w = 44;
	}
	for (i = 0; i < BIRD_COUNT; i++) {
		d->bird[i].x = 40 + i * 60;
		d->bird[i].y = ROOM_CEILING_Y + 12 + i * 20;
		d->bird[i].vx = (i % 2) ? 1 : -1;
		d->bird[i].active = 1;
	}
	s->data = d;
}

static void birdcage_update(struct scene *s)
{
	struct birdcage_data *d = (struct birdcage_data *)s->data;
	struct rect cat_box;
	int dx = 0, i;
	int prev_bottom;

	if (!d)
		return;

	if (++d->timer > game_room_time_limit()) {
		room_fail("TIME UP");
		return;
	}

	if (input_key_pressed(KEY_LEFT))  dx -= PLAYER_SPEED;
	if (input_key_pressed(KEY_RIGHT)) dx += PLAYER_SPEED;
	if (input_key_just_pressed(KEY_ACTION)) {
		player_jump(&d->cat);
		sound_play_tone(600, 60);
	}

	cat_box = player_bounds(&d->cat);
	prev_bottom = d->cat.y + cat_box.h;

	/* Falling off the end of a platform has to be possible, so assume
	 * airborne and let the landing test below put us back down. */
	if (d->cat.on_ground && !d->cat.jumping)
		d->cat.on_ground = 0;

	player_set_input(&d->cat, dx);
	player_update(&d->cat, CAT_GROUND);

	cat_box = player_bounds(&d->cat);
	if (d->cat.vy >= 0) {
		for (i = 0; i < PLATFORM_COUNT; i++) {
			int bottom = d->cat.y + cat_box.h;

			if (cat_box.x + cat_box.w < d->plat[i].x ||
			    cat_box.x > d->plat[i].x + d->plat[i].w)
				continue;
			/* Crossed the surface this tick, rather than merely being
			 * below it. */
			if (prev_bottom <= d->plat[i].y && bottom >= d->plat[i].y) {
				player_land(&d->cat, d->plat[i].y - cat_box.h);
				break;
			}
		}
	}

	cat_box = player_bounds(&d->cat);
	for (i = 0; i < BIRD_COUNT; i++) {
		struct rect br;

		if (!d->bird[i].active)
			continue;

		d->bird[i].x += d->bird[i].vx * (1 + game_enemy_bonus_speed() / 2);
		if (d->bird[i].x < 8 || d->bird[i].x + spr_bird.w > SCREEN_W - 8)
			d->bird[i].vx = -d->bird[i].vx;

		br.x = d->bird[i].x;
		br.y = d->bird[i].y;
		br.w = spr_bird.w;
		br.h = spr_bird.h;
		if (rect_overlap(cat_box, br)) {
			d->bird[i].active = 0;
			d->taken++;
			sound_play_tone(1000 + d->taken * 120, 120);
		}
	}

	if (d->taken >= BIRD_COUNT) {
		room_succeed(ROOM_BIRDCAGE, 200 + 50 * g_state.level);
		return;
	}
}

static void birdcage_render(struct scene *s)
{
	struct birdcage_data *d = (struct birdcage_data *)s->data;
	int i;

	if (!d)
		return;

	/* Magenta interior: the cat is black, so a black room hides it. */
	room_draw_walls(CGA_MAGENTA);

	/* Cage bars. */
	for (i = 8; i < SCREEN_W - 8; i += 16)
		render_line(i, ROOM_CEILING_Y, i, ROOM_FLOOR_Y - 1, CGA_CYAN);

	for (i = 0; i < PLATFORM_COUNT; i++) {
		render_fill_rect(d->plat[i].x, d->plat[i].y, d->plat[i].w,
		                 PLATFORM_H, CGA_CYAN);
		render_rect(d->plat[i].x, d->plat[i].y, d->plat[i].w,
		            PLATFORM_H, CGA_WHITE);
	}

	for (i = 0; i < BIRD_COUNT; i++)
		if (d->bird[i].active)
			render_sprite_stencil(&spr_bird, d->bird[i].x, d->bird[i].y, CGA_WHITE);

	player_render(&d->cat);
	room_draw_hud(ROOM_BIRDCAGE, d->taken, BIRD_COUNT,
	              game_room_time_limit() - d->timer);
}

static void birdcage_exit(struct scene *s)
{
	free(s->data);
	s->data = NULL;
}

static void birdcage_keydown(struct scene *s, SDL_Keycode key)
{
	(void)s;
	if (key == SDLK_ESCAPE)
		room_leave();
}

struct scene *birdcage_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));

	if (!s)
		return NULL;
	s->enter   = birdcage_enter;
	s->update  = birdcage_update;
	s->render  = birdcage_render;
	s->exit    = birdcage_exit;
	s->keydown = birdcage_keydown;
	return s;
}
