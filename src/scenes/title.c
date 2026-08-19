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

/*
 * Layout comes from the reference implementation's draw calls, whose CGA
 * memory offsets map to the screen as
 *
 *     y = 2 * (loc / 80) [+1 for the odd field], x = 4 * (loc % 80)
 *
 * so loc 1694 is (56, 42), loc 189 is (116, 4), and so on.
 */
#define WALL_X      116
#define WALL_Y        4
#define LOGO_X       56
#define LOGO_Y       42
#define EMBLEM_X    160
#define EMBLEM_Y     66
#define BAND_X      160
#define BAND_Y       80

#define FENCE_TOP_Y 104
#define FENCE_Y     112
#define FENCE_BOT_Y 168   /* trashcan bottoms carry a magenta background, so
                           * the original stands them on magenta ground; the
                           * fence has to stop above it */
#define GROUND_Y    168
#define CAT_Y        96

#define COPYRIGHT_X  56
#define COPYRIGHT_Y 188
#define LOGOS_X     160
#define LOGOS_Y     186
#define CAPTION_X   240
#define CAPTION_Y   190

#define HOLE_COUNT  9
#define UNDER_COUNT 5

struct title_data {
	int timer;
	struct player cat;
	int cat_timer;
	int next_cat_timer;
	/* Knot holes are scattered once on entry, the way alleycat_draw_fence
	 * does it, rather than sitting in an obviously even row. */
	int hole_x[HOLE_COUNT], hole_y[HOLE_COUNT], hole_i[HOLE_COUNT];
	int under_x[UNDER_COUNT];
};

static void title_enter(struct scene *s)
{
	struct title_data *d = calloc(1, sizeof(*d));
	int i;

	if (!d)
		return;
	player_init(&d->cat);
	player_set_pos(&d->cat, -30, CAT_Y);
	player_set_limits(&d->cat, -30, SCREEN_W + 30);
	d->next_cat_timer = SECONDS(1);

	for (i = 0; i < HOLE_COUNT; i++) {
		d->hole_x[i] = rand() % (SCREEN_W - 8);
		d->hole_y[i] = FENCE_Y + 20 + rand() % (FENCE_BOT_Y - FENCE_Y - 30);
		d->hole_i[i] = rand() % 4;
	}
	for (i = 0; i < UNDER_COUNT; i++)
		d->under_x[i] = rand() % (SCREEN_W - 8);

	s->data = d;

	sound_play_intro_music();
}

/* The reference walks the cat with the same rule: turn at the edges, and no
 * more than once a second, sit if the roll is above 160. */
static int decide_movement(struct title_data *d)
{
	if (d->cat.x <= 8)
		return 1;
	if (d->cat.x >= SCREEN_W - 40)
		return -1;

	if (d->cat_timer < d->next_cat_timer)
		return d->cat.input_dx ? (d->cat.input_dx > 0 ? 1 : -1) : 0;

	d->cat_timer = 0;
	d->next_cat_timer = SECONDS(1) + rand() % SECONDS(1);
	{
		int roll = rand() % 256;
		if (roll > 160)
			return 0;
		return (roll & 1) ? 1 : -1;
	}
}

static void start_game(void)
{
	sound_stop();
	input_clear_edges();
	scene_request_replace(alley_create());
}

static void title_update(struct scene *s)
{
	struct title_data *d = (struct title_data *)s->data;

	if (!d)
		return;

	d->timer++;
	d->cat_timer++;

	player_set_input(&d->cat, decide_movement(d) * 2);
	player_update(&d->cat, CAT_Y);

	/* One path into the game.  There used to be a second in title_keydown,
	 * and whichever fired first left the action key still reading as newly
	 * pressed when the alley took over. */
	if (input_key_just_pressed(KEY_ACTION) || input_key_just_pressed(KEY_RETURN))
		start_game();
}

static void draw_fence(const struct title_data *d)
{
	int i;

	for (i = 0; i * 8 < SCREEN_W; i++)
		render_sprite(&spr_fence_top[i % 4], i * 8, FENCE_TOP_Y);

	render_fill_rect(0, FENCE_Y, SCREEN_W, FENCE_BOT_Y - FENCE_Y, CGA_CYAN);
	for (i = 0; i < SCREEN_W; i += 8)
		render_line(i, FENCE_Y, i, FENCE_BOT_Y - 1, CGA_BLACK);

	for (i = 0; i < HOLE_COUNT; i++)
		render_sprite(&spr_fence_hole[d->hole_i[i]], d->hole_x[i], d->hole_y[i]);

	render_fill_rect(0, GROUND_Y, SCREEN_W, SCREEN_H - GROUND_Y, CGA_MAGENTA);
	for (i = 0; i < UNDER_COUNT; i++)
		render_sprite(&spr_fence_under, d->under_x[i], GROUND_Y - 5);
}

static void draw_trashcan(int x, int y, int middles)
{
	int i;

	render_sprite(&spr_trashcan_lid, x, y);
	y += spr_trashcan_lid.h;
	for (i = 0; i < middles; i++, y += spr_trashcan_mid.h)
		render_sprite(&spr_trashcan_mid, x + 4, y);
	render_sprite(&spr_trashcan_bot, x + 4, y);
}

static void title_render(struct scene *s)
{
	struct title_data *d = (struct title_data *)s->data;

	if (!d)
		return;

	/* The original fills the screen with 0xAA, which is light magenta in
	 * every pixel -- not the cyan this used to use. */
	render_fill(CGA_MAGENTA);

	render_sprite(&spr_title_wall,   WALL_X,   WALL_Y);
	render_sprite(&spr_title_logo,   LOGO_X,   LOGO_Y);
	render_sprite(&spr_title_emblem, EMBLEM_X, EMBLEM_Y);
	render_sprite(&spr_title_band,   BAND_X,   BAND_Y);

	draw_fence(d);

	draw_trashcan(24,  140, 2);
	draw_trashcan(96,  148, 1);
	draw_trashcan(180, 140, 2);
	draw_trashcan(264, 148, 1);

	player_render(&d->cat);

	/* Glyphs carry a light-cyan tile, so on the cyan fence only the black
	 * letterforms show -- which is how the original gets text here. */
	render_text("HI", 8, 116);
	render_number(g_state.hi_score, 32, 116, 6);
	render_text("SCORE", 176, 116);
	render_number(g_state.score, 232, 116, 6);

	if ((d->timer / SECONDS(1)) % 2 == 0)
		render_text_centered("PRESS SPACE TO START", 130);

	render_sprite(&spr_copyright, COPYRIGHT_X, COPYRIGHT_Y);
	render_sprite((d->timer / SECONDS(1)) % 2 ? &spr_synsoft : &spr_ibm_corp,
	              LOGOS_X, LOGOS_Y);
	render_sprite(&spr_title_caption, CAPTION_X, CAPTION_Y);
}

static void title_exit(struct scene *s)
{
	free(s->data);
	s->data = NULL;
}

static void title_keydown(struct scene *s, SDL_Keycode key)
{
	(void)s;
	if (key == SDLK_ESCAPE)
		game_quit();
}

struct scene *title_create(void)
{
	struct scene *s = calloc(1, sizeof(*s));

	if (!s)
		return NULL;
	s->enter   = title_enter;
	s->update  = title_update;
	s->render  = title_render;
	s->exit    = title_exit;
	s->keydown = title_keydown;
	return s;
}
