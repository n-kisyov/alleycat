#include "player.h"
#include "sprites.h"
#include "render.h"
#include "game.h"
#include <string.h>

#define GROUND_Y 152

void player_init(struct player *p)
{
	memset(p, 0, sizeof(*p));
	p->direction = PLAYER_DIR_RIGHT;
	p->alive = 1;
	p->on_ground = 1;
	p->w = 24;
	p->h = 11;
}

void player_render(struct player *p)
{
	if (!p->alive) return;
	const uint8_t *sprite;
	int w, h;

	if (p->swimming) {
		if (p->direction == PLAYER_DIR_RIGHT) {
			sprite = p->anim_frame == 0 ? cat_swim_right1 :
			         p->anim_frame == 1 ? cat_swim_right2 : cat_swim_right3;
		} else {
			sprite = p->anim_frame == 0 ? cat_swim_left1 :
			         p->anim_frame == 1 ? cat_swim_left2 : cat_swim_left3;
		}
		w = 12; h = 12;
	} else if (p->sitting) {
		int hi = (p->sit_timer / 8) % 8;
		int bi = (p->sit_timer / 8) % 3;
		const uint8_t *head = hi < 7 ?
			(hi % 2 == 0 ? cat_head_front1 : cat_head_front2) : cat_head_back;
		const uint8_t *body = bi == 0 ? cat_body1 : bi == 1 ? cat_body2 : cat_body3;
		render_sprite(head, p->x, p->y, 16, 6);
		render_sprite(body, p->x, p->y + 6, 16, 6);
		return;
	} else if (p->jumping || p->falling) {
		sprite = cat_jump_up;
		w = 28; h = 12;
	} else if (p->direction == PLAYER_DIR_RIGHT) {
		sprite = cat_walk_right[p->anim_frame % CAT_WALK_FRAMES];
		w = 24; h = 11;
	} else {
		sprite = cat_walk_left[p->anim_frame % CAT_WALK_FRAMES];
		w = 24; h = 11;
	}
	render_sprite(sprite, p->x, p->y, w, h);
}

void player_update(struct player *p, float dt)
{
	if (!p->alive) return;
	(void)dt;

	if (!p->swimming && !p->on_ground) {
		p->vy += 1;
		p->y += p->vy;
		if (p->vy > 0) {
			p->jumping = 0;
			p->falling = 1;
		}
		if (p->y >= GROUND_Y) {
			p->y = GROUND_Y;
			p->vy = 0;
			p->jumping = 0;
			p->falling = 0;
			p->on_ground = 1;
		}
	}

	p->anim_timer++;
	if (p->anim_timer >= 4) {
		p->anim_timer = 0;
		if (p->swimming) {
			p->anim_frame = (p->anim_frame + 1) % 3;
		} else if (!p->jumping && !p->falling && !p->sitting) {
			p->anim_frame = (p->anim_frame + 1) % CAT_WALK_FRAMES;
		}
	}

	p->x += p->vx;
	if (p->x < 0) p->x = 0;
	if (p->x > SCREEN_W - p->w) p->x = SCREEN_W - p->w;
	p->vx = 0;
}

void player_move(struct player *p, int dx, int dy)
{
	p->vx = dx;
	p->vy = dy;
	if (dx > 0) p->direction = PLAYER_DIR_RIGHT;
	else if (dx < 0) p->direction = PLAYER_DIR_LEFT;
	if (!p->swimming && dx == 0 && dy == 0 && p->on_ground) {
		p->sitting = 1;
		p->sit_timer = 0;
	} else {
		p->sitting = 0;
	}
}

void player_jump(struct player *p)
{
	if (p->on_ground && !p->swimming) {
		p->vy = -5;
		p->on_ground = 0;
		p->jumping = 1;
		p->falling = 0;
		p->sitting = 0;
	}
}

void player_set_pos(struct player *p, int x, int y)
{
	p->x = x;
	p->y = y;
}

void player_set_swim(struct player *p, int swimming)
{
	p->swimming = swimming;
	p->jumping = 0;
	p->falling = 0;
	p->sitting = 0;
	if (swimming) {
		p->on_ground = 0;
		p->vy = 0;
	} else {
		p->on_ground = 1;
	}
}
