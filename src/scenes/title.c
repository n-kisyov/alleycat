#include "title.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../sound.h"
#include "../player.h"
#include "../sprites.h"
#include "alley.h"
#include <stdlib.h>
#include <string.h>

struct title_data {
	int timer;
	int logo_flag;
	struct player cat;
	int cat_timer;
	int next_cat_timer;
	int frame;
};

static void title_enter(struct scene *s)
{
	struct title_data *d = calloc(1, sizeof(*d));
	player_init(&d->cat);
	d->cat.x = -30;
	d->cat.y = 150;
	srand((unsigned int)SDL_GetTicks());
	s->data = d;
}

static void title_update(struct scene *s, float dt)
{
	struct title_data *d = (struct title_data *)s->data;
	d->timer++;
	d->frame++;
	if (d->frame % 16 == 0)
		d->logo_flag = !d->logo_flag;

	d->cat_timer++;
		if (d->cat.x <= 8)
			player_move(&d->cat, 1, 0);
		else if (d->cat.x >= 280)
			player_move(&d->cat, -1, 0);
		else if (d->cat_timer >= d->next_cat_timer) {
			d->cat_timer = 0;
			d->next_cat_timer = 25 + (rand() % 40);
			int r = rand() % 256;
			if (r > 160) player_move(&d->cat, 0, 0);
			else if (r & 1) player_move(&d->cat, 1, 0);
			else player_move(&d->cat, -1, 0);
		}
	player_update(&d->cat, dt);

	if (input_key_just_pressed(KEY_ACTION) || input_key_just_pressed(KEY_RETURN)) {
		sound_stop();
		scene_replace(alley_create());
	}
}

static void title_render(struct scene *s)
{
	struct title_data *d = (struct title_data *)s->data;

	render_fill(PAL_CYAN);

	for (int i = 0; i < 40; i++)
		render_sprite(sprite_fence_top[i % 4], i * 8, 150, 8, 8);

	render_fill_rect(0, 157, 320, 43, PAL_BROWN);
	render_fill_rect(0, 182, 320, 18, PAL_DGRAY);

	render_text("HI",                6, 186);
	render_text("000000",           22, 186);
	render_text("SCORE",            98, 186);
	render_text("000000",          130, 186);

	render_text("THE",             130, 60);
	render_text("ALLEY",           124, 70);
	render_text("CAT!",            122, 80);

	if (d->logo_flag)
		render_sprite(sprite_synsoft, 100, 96, 80, 12);
	else
		render_sprite(sprite_ibm_corp, 100, 96, 80, 12);

	render_sprite(sprite_copyright, 112, 114, 96, 11);

	for (int i = 0; i < 3; i++) {
		int tx = 30 + i * 100;
		render_sprite(sprite_trashcan_lid, tx, 108, 40, 12);
		render_sprite(sprite_trashcan_mid, tx + 4, 120, 32, 8);
		render_sprite(sprite_trashcan_bot, tx + 4, 128, 32, 11);
	}

	player_render(&d->cat);

	if (d->timer % 64 < 32)
		render_text("PRESS SPACE TO START", 80, 172);
}

static void title_exit(struct scene *s) { free(s->data); }

static void title_keydown(struct scene *s, SDL_Keycode key)
{
	(void)s;
	if (key == SDLK_SPACE || key == SDLK_RETURN) {
		sound_stop();
		scene_replace(alley_create());
	}
}

struct scene *title_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));
	s->enter   = title_enter;
	s->update  = title_update;
	s->render  = title_render;
	s->exit    = title_exit;
	s->keydown = title_keydown;
	return s;
}
