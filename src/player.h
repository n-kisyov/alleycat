#ifndef ALLEYCAT_PLAYER_H
#define ALLEYCAT_PLAYER_H

#include <stdint.h>

#define PLAYER_DIR_RIGHT 1
#define PLAYER_DIR_LEFT  -1

struct player {
	int x, y;
	int vx, vy;
	int w, h;
	int direction;
	int anim_frame;
	int anim_timer;
	int frame_count;
	int on_ground;
	int jumping;
	int falling;
	int swimming;
	int sitting;
	int sit_timer;
	int alive;
	int timer;
};

void player_init(struct player *p);
void player_update(struct player *p, float dt);
void player_render(struct player *p);
void player_move(struct player *p, int dx, int dy);
void player_jump(struct player *p);
void player_set_pos(struct player *p, int x, int y);
void player_set_swim(struct player *p, int swimming);

#endif
