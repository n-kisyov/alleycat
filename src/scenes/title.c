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
	uint32_t seed;
	struct player cat;
	int cat_timer;
};

static void title_enter(struct scene *s)
{
	struct title_data *d = calloc(1, sizeof(*d));
	d->seed = 42;
	d->cat_timer = 0;
	player_init(&d->cat);
	d->cat.x = -30;
	d->cat.y = 96;
	srand(d->seed);
	s->data = d;
	sound_play_intro_music();
}

static void title_update(struct scene *s, float dt)
{
	struct title_data *d = (struct title_data *)s->data;
	(void)dt;

	d->timer++;
	if (d->timer % 16 == 0)
		d->logo_flag = !d->logo_flag;

	d->cat_timer++;
	if (d->cat.x <= 32)
		player_move(&d->cat, 2, 0);
	else if (d->cat.x >= 288)
		player_move(&d->cat, -2, 0);
	else {
		if (d->cat_timer > 18) {
			d->cat_timer = 0;
			int r = rand() % 256;
			if (r > 160) player_move(&d->cat, 0, 0);
			else if (r & 1) player_move(&d->cat, 2, 0);
			else player_move(&d->cat, -2, 0);
		}
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

	for (int i = 4160; i < 4240; i += 2) {
		render_sprite(sprite_fence_top[rand() % 4], (i - 4160) * 2 + 4, 160, 8, 8);
	}

	render_fill_rect(0, 168, SCREEN_W, 32, PAL_BROWN);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			render_sprite(sprite_fence_hole[i], j * 16 + 4, 176, 8, 5);
		}
	}

	render_sprite(title_sprite6, 260, 184, 16, 8);

	render_text("HI", 8, 164);
	render_number(0, 20, 164 + 2, 6);
	render_text("SCORE", 100, 164);
	render_number(0, 110, 164 + 2, 6);
	render_text("0", 64, 164 + 2);
	render_number(9, 68, 164 + 2, 1);

	render_sprite(title_sprite1, 20, 60, 44, 29);
	render_sprite(title_sprite2, 180, 60, 56, 22);
	render_sprite(title_sprite3, 108, 88, 12, 12);
	render_sprite(title_sprite4, 128, 60, 56, 8);

	render_sprite(sprite_copyright, 180, 140, 48, 11);

	if (d->logo_flag)
		render_sprite(sprite_synsoft, 180, 80, 40, 12);
	else
		render_sprite(sprite_ibm_corp, 180, 80, 40, 12);

	render_text("THE", 108, 100);
	render_text("ALLEY", 108, 108);
	render_text("CAT!", 108, 116);

	player_render(&d->cat);

	if (d->timer % 64 < 32)
		render_text("PRESS SPACE", 115, 180);
}

static void title_exit(struct scene *s)
{
	free(s->data);
}

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
	s->enter = title_enter;
	s->update = title_update;
	s->render = title_render;
	s->exit = title_exit;
	s->keydown = title_keydown;
	return s;
}
