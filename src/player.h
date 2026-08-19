#ifndef ALLEYCAT_PLAYER_H
#define ALLEYCAT_PLAYER_H

#include <stdint.h>
#include "render.h"
#include "sprites.h"

#define PLAYER_DIR_RIGHT  1
#define PLAYER_DIR_LEFT  -1

#define PLAYER_SPEED  2
#define PLAYER_JUMP_VY -7
#define PLAYER_GRAVITY  1
#define PLAYER_MAX_FALL 8

struct player {
	int x, y;

	/*
	 * input_dx is horizontal intent in pixels for this tick; vy is owned by
	 * gravity and player_jump alone.  Keeping them apart is the whole point
	 * -- the old player_move() wrote vy from a movement argument that was
	 * always zero, which erased the jump impulse before it was ever applied.
	 */
	int input_dx;
	int vy;

	int direction;
	int anim_frame;
	int anim_timer;

	int on_ground;
	int jumping;
	int falling;
	int swimming;
	int sitting;
	int sit_timer;
	int sit_head;
	int sit_body;

	int alive;

	int min_x, max_x;   /* horizontal limits, inclusive of the sprite width */
};

void player_init(struct player *p);
void player_set_pos(struct player *p, int x, int y);
void player_set_limits(struct player *p, int min_x, int max_x);

void player_set_input(struct player *p, int dx);
void player_jump(struct player *p);

/* Walking physics: horizontal move, gravity, landing on ground_y. */
void player_update(struct player *p, int ground_y);

/* Free swimming, used by the fishbowl.  bounds is the water. */
void player_swim_update(struct player *p, int dx, int dy, struct rect bounds);
void player_set_swim(struct player *p, int swimming);

/* Snap onto a platform surface; used after player_update for platform rooms. */
void player_land(struct player *p, int surface_y);

const struct sprite *player_sprite(const struct player *p);
struct rect player_bounds(const struct player *p);
void player_render(const struct player *p);

#endif
