#include "gameover.h"
#include "title.h"
#include "../game.h"
#include "../render.h"
#include "../input.h"
#include "../sound.h"
#include "../sprites.h"
#include <stdlib.h>

#define HOLD_TICKS SECONDS(1)

struct gameover_data {
	int timer;
};

static void gameover_enter(struct scene *s)
{
	s->data = calloc(1, sizeof(struct gameover_data));

	sound_stop();
	sound_play_tone(220, 300);
	sound_play_tone(180, 300);
	sound_play_tone(140, 600);
}

static void gameover_update(struct scene *s)
{
	struct gameover_data *d = (struct gameover_data *)s->data;

	if (!d)
		return;
	d->timer++;

	/* Ignore input briefly so the key that killed you does not skip past
	 * the screen telling you so. */
	if (d->timer < HOLD_TICKS)
		return;

	if (input_key_just_pressed(KEY_ACTION) || input_key_just_pressed(KEY_RETURN)) {
		game_new();
		input_clear_edges();
		scene_request_replace(title_create());
	}
}

static void gameover_render(struct scene *s)
{
	struct gameover_data *d = (struct gameover_data *)s->data;

	if (!d)
		return;

	render_fill(CGA_BLACK);
	render_banner("GAME OVER", 76);

	render_fill_rect(0, 108, SCREEN_W, TEXT_BAND_H, CGA_CYAN);
	render_text("SCORE", 88, 112);
	render_number(g_state.score, 136, 112, 6);

	render_fill_rect(0, 128, SCREEN_W, TEXT_BAND_H, CGA_CYAN);
	render_text("BEST", 88, 132);
	render_number(g_state.hi_score, 136, 132, 6);

	if (d->timer >= HOLD_TICKS && (d->timer / SECONDS(1)) % 2 == 0)
		render_banner("PRESS SPACE", 164);
}

static void gameover_exit(struct scene *s)
{
	free(s->data);
	s->data = NULL;
}

static void gameover_keydown(struct scene *s, SDL_Keycode key)
{
	(void)s;
	if (key == SDLK_ESCAPE)
		game_quit();
}

struct scene *gameover_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));

	if (!s)
		return NULL;
	s->enter   = gameover_enter;
	s->update  = gameover_update;
	s->render  = gameover_render;
	s->exit    = gameover_exit;
	s->keydown = gameover_keydown;
	return s;
}
