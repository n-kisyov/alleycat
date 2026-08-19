#include "player.h"
#include "sprites.h"
#include "render.h"
#include "game.h"
#include <stdlib.h>
#include <string.h>

#define ANIM_TICKS       3   /* ~20 fps, close to the original's 18.2 Hz */
#define SIT_CHANGE_TICKS 26  /* the original re-picks head and body every 8
                              * of its own frames, which lands about here */

void player_init(struct player *p)
{
	memset(p, 0, sizeof(*p));
	p->direction = PLAYER_DIR_RIGHT;
	p->alive     = 1;
	p->on_ground = 1;
	p->min_x     = 0;
	p->max_x     = SCREEN_W;
}

void player_set_pos(struct player *p, int x, int y)
{
	p->x = x;
	p->y = y;
}

void player_set_limits(struct player *p, int min_x, int max_x)
{
	p->min_x = min_x;
	p->max_x = max_x;
}

void player_set_input(struct player *p, int dx)
{
	p->input_dx = dx;
	if (dx > 0)
		p->direction = PLAYER_DIR_RIGHT;
	else if (dx < 0)
		p->direction = PLAYER_DIR_LEFT;
}

void player_jump(struct player *p)
{
	if (p->on_ground && !p->swimming) {
		p->vy        = PLAYER_JUMP_VY;
		p->on_ground = 0;
		p->jumping   = 1;
		p->falling   = 0;
		p->sitting   = 0;
	}
}

void player_land(struct player *p, int surface_y)
{
	p->y         = surface_y;
	p->vy        = 0;
	p->jumping   = 0;
	p->falling   = 0;
	p->on_ground = 1;
}

static void advance_animation(struct player *p)
{
	if (p->sitting) {
		if (++p->sit_timer >= SIT_CHANGE_TICKS) {
			p->sit_timer = 0;
			p->sit_head  = rand() % 8;
			p->sit_body  = rand() % 4;
		}
		return;
	}

	if (++p->anim_timer < ANIM_TICKS)
		return;
	p->anim_timer = 0;

	if (p->swimming)
		p->anim_frame = (p->anim_frame + 1) % 3;
	else if (!p->jumping && !p->falling)
		p->anim_frame = (p->anim_frame + 1) % CAT_WALK_FRAMES;
}

void player_update(struct player *p, int ground_y)
{
	struct rect r;

	if (!p->alive)
		return;

	/* Horizontal */
	p->x += p->input_dx;
	r = player_bounds(p);
	if (p->x < p->min_x)
		p->x = p->min_x;
	if (p->x + r.w > p->max_x)
		p->x = p->max_x - r.w;

	/* Vertical.  vy survives from player_jump because nothing else writes it. */
	if (!p->swimming) {
		if (!p->on_ground) {
			p->vy += PLAYER_GRAVITY;
			if (p->vy > PLAYER_MAX_FALL)
				p->vy = PLAYER_MAX_FALL;
			p->y += p->vy;
			if (p->vy > 0) {
				p->jumping = 0;
				p->falling = 1;
			}
		}
		if (p->y >= ground_y)
			player_land(p, ground_y);
	}

	/* The cat sits when it is idle on the ground and not mid-jump. */
	p->sitting = (!p->swimming && p->input_dx == 0 && p->on_ground);

	advance_animation(p);
}

void player_set_swim(struct player *p, int swimming)
{
	p->swimming = swimming;
	p->jumping  = 0;
	p->falling  = 0;
	p->sitting  = 0;
	p->vy       = 0;
	p->on_ground = !swimming;
}

void player_swim_update(struct player *p, int dx, int dy, struct rect bounds)
{
	struct rect r;

	if (!p->alive)
		return;

	if (dx > 0)
		p->direction = PLAYER_DIR_RIGHT;
	else if (dx < 0)
		p->direction = PLAYER_DIR_LEFT;
	p->input_dx = dx;

	p->x += dx;
	p->y += dy;

	r = player_bounds(p);
	if (p->x < bounds.x)
		p->x = bounds.x;
	if (p->x + r.w > bounds.x + bounds.w)
		p->x = bounds.x + bounds.w - r.w;
	if (p->y < bounds.y)
		p->y = bounds.y;
	if (p->y + r.h > bounds.y + bounds.h)
		p->y = bounds.y + bounds.h - r.h;

	advance_animation(p);
}

const struct sprite *player_sprite(const struct player *p)
{
	if (p->swimming) {
		const struct sprite *set = (p->direction == PLAYER_DIR_RIGHT)
			? spr_cat_swim_right : spr_cat_swim_left;
		return &set[p->anim_frame % 3];
	}
	if (p->jumping || p->falling)
		return &spr_cat_jump;
	if (p->sitting)
		return &spr_cat_head[p->sit_head % 8];   /* head only; see bounds */
	return (p->direction == PLAYER_DIR_RIGHT)
		? &spr_cat_walk_right[p->anim_frame % CAT_WALK_FRAMES]
		: &spr_cat_walk_left[p->anim_frame % CAT_WALK_FRAMES];
}

struct rect player_bounds(const struct player *p)
{
	struct rect r;
	const struct sprite *s = player_sprite(p);

	r.x = p->x;
	r.y = p->y;
	r.w = s->w;
	/* Sitting draws a head stacked on a body, so it is twice as tall as the
	 * sprite the lookup returns. */
	r.h = p->sitting ? s->h + spr_cat_body[p->sit_body % 4].h : s->h;
	return r;
}

void player_render(const struct player *p)
{
	if (!p->alive)
		return;

	if (p->sitting) {
		const struct sprite *head = &spr_cat_head[p->sit_head % 8];
		const struct sprite *body = &spr_cat_body[p->sit_body % 4];
		render_sprite_keyed(head, p->x, p->y);
		render_sprite_keyed(body, p->x, p->y + head->h);
		return;
	}

	render_sprite_keyed(player_sprite(p), p->x, p->y);
}
