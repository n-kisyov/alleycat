#include "alley.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../player.h"
#include "../sprites.h"
#include "../sound.h"
#include "title.h"
#include "fishbowl.h"
#include "kennel.h"
#include "birdcage.h"
#include "cheese.h"
#include "love.h"
#include "milk.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WINDOW_COUNT  ROOM_COUNT
#define WINDOW_W      36
#define WINDOW_H      30
#define WINDOW_Y      40
#define WINDOW_STEP   50
#define WINDOW_LEFT   14

#define LEDGE_Y      100
#define FENCE_TOP_Y  100
#define FENCE_Y      108
#define FENCE_BOT_Y  144
/* The pavement is magenta because the trashcan bottoms are drawn with a
 * magenta background; standing them on cyan leaves visible corner blocks. */
#define GROUND_Y     144
#define SURFACE_Y    156
#define CAT_Y        (SURFACE_Y - 11)
#define HUD_Y        (SCREEN_H - 16)
#define TRASHCAN_H   (12 + 8 + 11)

#define OTHER_CATS         3
#define OTHER_CAT_MIN_X    8
#define OTHER_CAT_MAX_X    (SCREEN_W - 32)
#define OTHER_CAT_TURN_PCT 1

typedef struct {
	int x, y, w, h;
	int room_type;
} window_t;

struct alley_data {
	struct player cat;
	window_t windows[WINDOW_COUNT];
	int trash_x[5];
	int timer;
	int other_cat_x[OTHER_CATS];
	int other_cat_dir[OTHER_CATS];
	int near_window;      /* index of the pipe the cat is standing at, or -1 */
	int level_banner;     /* ticks left on the "LEVEL n" banner */
	int pending_level;
};

static struct scene *create_room(int room)
{
	switch (room) {
	case ROOM_FISHBOWL: return fishbowl_create();
	case ROOM_KENNEL:   return kennel_create();
	case ROOM_BIRDCAGE: return birdcage_create();
	case ROOM_CHEESE:   return cheese_create();
	case ROOM_LOVE:     return love_create();
	case ROOM_MILK:     return milk_create();
	default:            return NULL;
	}
}

static void alley_enter(struct scene *s)
{
	struct alley_data *d = calloc(1, sizeof(*d));
	int i;

	if (!d)
		return;

	player_init(&d->cat);
	player_set_pos(&d->cat, 8, CAT_Y);
	player_set_limits(&d->cat, 0, SCREEN_W);

	for (i = 0; i < WINDOW_COUNT; i++) {
		d->windows[i].x = WINDOW_LEFT + i * WINDOW_STEP;
		d->windows[i].y = WINDOW_Y;
		d->windows[i].w = WINDOW_W;
		d->windows[i].h = WINDOW_H;
		d->windows[i].room_type = i;
	}

	for (i = 0; i < 5; i++)
		d->trash_x[i] = 40 + i * 58;

	for (i = 0; i < OTHER_CATS; i++) {
		d->other_cat_x[i] = 80 + i * 70;
		d->other_cat_dir[i] = (i % 2) ? 1 : -1;
	}

	d->near_window = -1;
	s->data = d;
}

/* The pipe under each window is what the cat climbs, so "near" is measured
 * against the pipe, not the window itself. */
static int pipe_x(const window_t *w)
{
	return w->x + w->w / 2;
}

static void alley_update(struct scene *s)
{
	struct alley_data *d = (struct alley_data *)s->data;
	struct rect cat_box;
	int dx = 0, i;

	if (!d)
		return;

	d->timer++;

	if (d->level_banner > 0 && --d->level_banner == 0) {
		game_level_complete();
		d->pending_level = 0;
	}

	/* All six windows dark: bank the level and reopen them. */
	if (!d->level_banner && !d->pending_level &&
	    g_state.rooms_done >= ROOM_COUNT) {
		char msg[FLASH_MAX];
		d->pending_level = 1;
		d->level_banner  = SECONDS(2);
		snprintf(msg, sizeof(msg), "LEVEL %d", g_state.level + 1);
		game_flash(msg);
		sound_play_tone(900, 120);
		sound_play_tone(1200, 240);
	}

	if (input_key_pressed(KEY_LEFT))
		dx -= PLAYER_SPEED;
	if (input_key_pressed(KEY_RIGHT))
		dx += PLAYER_SPEED;

	/* Jump first, then set the horizontal input.  player_set_input no
	 * longer touches vy, so the impulse survives to player_update. */
	if (input_key_just_pressed(KEY_ACTION) && d->near_window < 0)
		player_jump(&d->cat);

	player_set_input(&d->cat, dx);
	player_update(&d->cat, CAT_Y);

	cat_box = player_bounds(&d->cat);
	d->near_window = -1;
	for (i = 0; i < WINDOW_COUNT; i++) {
		int px = pipe_x(&d->windows[i]);

		if (game_room_is_done(d->windows[i].room_type))
			continue;
		if (px >= cat_box.x - 6 && px <= cat_box.x + cat_box.w + 6) {
			d->near_window = i;
			break;
		}
	}

	if (d->near_window >= 0 && !d->level_banner &&
	    (input_key_just_pressed(KEY_UP) || input_key_just_pressed(KEY_ACTION))) {
		int room = d->windows[d->near_window].room_type;
		struct scene *sc = create_room(room);

		if (sc) {
			game_start_room(room);
			game_flash(NULL);
			sound_play_tone(600, 150);
			input_clear_edges();
			/* Pushed, not replaced: the alley keeps its state so the
			 * cat comes back where it left. */
			scene_request_push(sc);
			return;
		}
	}

	for (i = 0; i < OTHER_CATS; i++) {
		d->other_cat_x[i] += d->other_cat_dir[i];
		if (d->other_cat_x[i] < OTHER_CAT_MIN_X || d->other_cat_x[i] > OTHER_CAT_MAX_X)
			d->other_cat_dir[i] = -d->other_cat_dir[i];
		else if (rand() % 100 < OTHER_CAT_TURN_PCT)
			d->other_cat_dir[i] = -d->other_cat_dir[i];
	}
}

static void draw_window(const window_t *w, int done, int highlight)
{
	int px = w->x + w->w / 2;

	/* Drain pipe from the sill down to the alley floor: it is the visual
	 * cue for where the cat has to stand. */
	render_line(px - 1, w->y + w->h, px - 1, GROUND_Y - 1,
	            done ? CGA_BLACK : CGA_CYAN);
	render_line(px, w->y + w->h, px, GROUND_Y - 1,
	            done ? CGA_BLACK : CGA_CYAN);

	if (done) {
		render_fill_rect(w->x, w->y, w->w, w->h, CGA_BLACK);
		render_rect(w->x, w->y, w->w, w->h, CGA_CYAN);
		render_line(w->x, w->y, w->x + w->w - 1, w->y + w->h - 1, CGA_CYAN);
		render_line(w->x + w->w - 1, w->y, w->x, w->y + w->h - 1, CGA_CYAN);
		return;
	}

	render_fill_rect(w->x, w->y, w->w, w->h, CGA_MAGENTA);
	render_rect(w->x, w->y, w->w, w->h, CGA_WHITE);
	/* Sash bars */
	render_line(w->x + w->w / 2, w->y, w->x + w->w / 2, w->y + w->h - 1, CGA_BLACK);
	render_line(w->x, w->y + w->h / 2, w->x + w->w - 1, w->y + w->h / 2, CGA_BLACK);

	if (highlight)
		render_rect(w->x - 2, w->y - 2, w->w + 4, w->h + 4, CGA_WHITE);
}

static void draw_trashcan(int x, int y)
{
	render_sprite(&spr_trashcan_lid, x, y);
	render_sprite(&spr_trashcan_mid, x + 4, y + spr_trashcan_lid.h);
	render_sprite(&spr_trashcan_bot, x + 4,
	              y + spr_trashcan_lid.h + spr_trashcan_mid.h);
}

static void alley_render(struct scene *s)
{
	struct alley_data *d = (struct alley_data *)s->data;
	int i;

	if (!d)
		return;

	render_fill(CGA_BLACK);

	for (i = 0; i < WINDOW_COUNT; i++)
		draw_window(&d->windows[i], game_room_is_done(d->windows[i].room_type),
		            d->near_window == i);

	/* Ledge, fence, alley floor. */
	render_fill_rect(0, LEDGE_Y - 4, SCREEN_W, 4, CGA_CYAN);
	for (i = 0; i * 8 < SCREEN_W; i++)
		render_sprite(&spr_fence_top[i % 4], i * 8, FENCE_TOP_Y);
	render_fill_rect(0, FENCE_Y, SCREEN_W, FENCE_BOT_Y - FENCE_Y, CGA_CYAN);
	for (i = 0; i < SCREEN_W; i += 8)
		render_line(i, FENCE_Y, i, FENCE_BOT_Y - 1, CGA_BLACK);
	for (i = 0; i < 7; i++)
		render_sprite(&spr_fence_hole[i % 4], 20 + i * 44, FENCE_Y + 16);

	render_fill_rect(0, GROUND_Y, SCREEN_W, HUD_Y - GROUND_Y, CGA_MAGENTA);
	render_line(0, SURFACE_Y, SCREEN_W - 1, SURFACE_Y, CGA_BLACK);

	for (i = 0; i < 5; i++)
		draw_trashcan(d->trash_x[i], SURFACE_Y - TRASHCAN_H);

	for (i = 0; i < OTHER_CATS; i++) {
		const struct sprite *cs = (d->other_cat_dir[i] > 0)
			? &spr_cat_walk_right[(d->timer / 6) % CAT_WALK_FRAMES]
			: &spr_cat_walk_left[(d->timer / 6) % CAT_WALK_FRAMES];
		render_sprite_keyed(cs, d->other_cat_x[i], FENCE_Y - 11);
	}

	player_render(&d->cat);

	/* HUD */
	render_fill_rect(0, HUD_Y, SCREEN_W, SCREEN_H - HUD_Y, CGA_CYAN);
	render_text("SCORE", 4, HUD_Y + 4);
	render_number(g_state.score, 48, HUD_Y + 4, 6);
	render_text("LV", 152, HUD_Y + 4);
	render_number(g_state.level, 176, HUD_Y + 4, 2);
	render_text("LIVES", 216, HUD_Y + 4);
	render_number(g_state.lives, 264, HUD_Y + 4, 2);

	if (g_state.flash_timer > 0)
		render_banner(g_state.flash, 118);
	else if (d->near_window >= 0)
		render_banner("PRESS UP TO CLIMB", 118);
}

static void alley_exit(struct scene *s)
{
	free(s->data);
	s->data = NULL;
}

static void alley_keydown(struct scene *s, SDL_Keycode key)
{
	(void)s;
	if (key == SDLK_ESCAPE) {
		game_flash(NULL);
		scene_request_replace(title_create());
	}
}

struct scene *alley_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));

	if (!s)
		return NULL;
	s->enter   = alley_enter;
	s->update  = alley_update;
	s->render  = alley_render;
	s->exit    = alley_exit;
	s->keydown = alley_keydown;
	return s;
}
