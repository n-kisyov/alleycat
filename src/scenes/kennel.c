#include "kennel.h"
#include "room.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../player.h"
#include "../sprites.h"
#include "../sound.h"
#include <stdlib.h>
#include <string.h>

#define BONE_COUNT   3
#define CAUGHT_GRACE SECONDS(1)
#define CAT_GROUND   (ROOM_FLOOR_Y - 11)

struct kennel_data {
	struct player cat;
	struct { int x, y, active; } bone[BONE_COUNT];
	int dog_x, dog_y, dog_timer;
	int timer;
	int taken;
	int caught_timer;
};

static void kennel_enter(struct scene *s)
{
	struct kennel_data *d = calloc(1, sizeof(*d));
	int i;

	if (!d)
		return;

	player_init(&d->cat);
	player_set_pos(&d->cat, SCREEN_W / 2, CAT_GROUND);
	player_set_limits(&d->cat, 8, SCREEN_W - 8);

	d->dog_x = 24;
	d->dog_y = ROOM_FLOOR_Y - spr_dog.h;

	for (i = 0; i < BONE_COUNT; i++) {
		d->bone[i].x = 40 + i * 90 + rand() % 20;
		d->bone[i].y = ROOM_FLOOR_Y - spr_bone.h - (rand() % 40);
		d->bone[i].active = 1;
	}
	s->data = d;
}

static void kennel_update(struct scene *s)
{
	struct kennel_data *d = (struct kennel_data *)s->data;
	struct rect cat_box, dog_box;
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

	/* The dog chases along the floor, a little faster each level. */
	if (++d->dog_timer > 3 - game_enemy_bonus_speed() / 2) {
		d->dog_timer = 0;
		if (d->dog_x < d->cat.x) d->dog_x++;
		if (d->dog_x > d->cat.x) d->dog_x--;
	}

	for (i = 0; i < BONE_COUNT; i++) {
		struct rect br;

		if (!d->bone[i].active)
			continue;
		br.x = d->bone[i].x;
		br.y = d->bone[i].y;
		br.w = spr_bone.w;
		br.h = spr_bone.h;
		if (rect_overlap(cat_box, br)) {
			d->bone[i].active = 0;
			d->taken++;
			sound_play_tone(800, 100);
		}
	}

	dog_box.x = d->dog_x;
	dog_box.y = d->dog_y;
	dog_box.w = spr_dog.w;
	dog_box.h = spr_dog.h;

	/* Jumping clears the dog, which is what the jump is for. */
	if (rect_overlap(cat_box, dog_box)) {
		if (++d->caught_timer > CAUGHT_GRACE) {
			room_fail("THE DOG GOT YOU");
			return;
		}
	} else if (d->caught_timer > 0) {
		d->caught_timer--;
	}

	if (d->taken >= BONE_COUNT) {
		room_succeed(ROOM_KENNEL, 150 + 50 * g_state.level);
		return;
	}
}

static void kennel_render(struct scene *s)
{
	struct kennel_data *d = (struct kennel_data *)s->data;
	int i;

	if (!d)
		return;

	room_draw_walls(CGA_MAGENTA);

	for (i = 0; i < BONE_COUNT; i++)
		if (d->bone[i].active)
			render_sprite_stencil(&spr_bone, d->bone[i].x, d->bone[i].y, CGA_WHITE);

	render_sprite_stencil(&spr_dog, d->dog_x, d->dog_y, CGA_BLACK);
	player_render(&d->cat);

	room_draw_hud(ROOM_KENNEL, d->taken, BONE_COUNT,
	              game_room_time_limit() - d->timer);

	if (d->caught_timer > 0)
		render_banner("JUMP", 120);
}

static void kennel_exit(struct scene *s)
{
	free(s->data);
	s->data = NULL;
}

static void kennel_keydown(struct scene *s, SDL_Keycode key)
{
	(void)s;
	if (key == SDLK_ESCAPE)
		room_leave();
}

struct scene *kennel_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));

	if (!s)
		return NULL;
	s->enter   = kennel_enter;
	s->update  = kennel_update;
	s->render  = kennel_render;
	s->exit    = kennel_exit;
	s->keydown = kennel_keydown;
	return s;
}
