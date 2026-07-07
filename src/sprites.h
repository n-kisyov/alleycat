#ifndef ALLEYCAT_SPRITES_H
#define ALLEYCAT_SPRITES_H

#include <stdint.h>

#define CAT_WALK_FRAMES 6
#define CAT_W 11
#define CAT_H2 8
#define CAT_HEAD_W 6
#define CAT_HEAD_H 8
#define CAT_BODY_W 6
#define CAT_BODY_H 8

extern uint8_t sprite_building[4][30][112];
extern uint8_t sprite_fence_top[4][16];
extern uint8_t sprite_fence_hole[4][10];
extern uint8_t sprite_fence_under[];
extern uint8_t sprite_digits[10][16];
extern uint8_t sprite_letters[26][16];
extern uint8_t sprite_punctuation[3][16];
extern uint8_t sprite_trashcan_lid[];
extern uint8_t sprite_trashcan_mid[];
extern uint8_t sprite_trashcan_bot[];
extern uint8_t sprite_copyright[];
extern uint8_t sprite_synsoft[];
extern uint8_t sprite_ibm_corp[];
extern uint8_t sprite_heart[];
extern uint8_t sprite_fish[];
extern uint8_t sprite_bone[];
extern uint8_t sprite_cheese_chunk[];
extern uint8_t sprite_bird[];
extern uint8_t sprite_dog[];
extern uint8_t sprite_broom[];
extern uint8_t sprite_mouse[];
extern uint8_t sprite_milk_bottle[];

extern uint8_t cat_walk_right[CAT_WALK_FRAMES][66];
extern uint8_t cat_walk_left[CAT_WALK_FRAMES][66];
extern uint8_t cat_head_front1[];
extern uint8_t cat_head_front2[];
extern uint8_t cat_head_back[];
extern uint8_t cat_body1[];
extern uint8_t cat_body2[];
extern uint8_t cat_body3[];
extern uint8_t cat_jump_up[];
extern uint8_t cat_swim_right1[];
extern uint8_t cat_swim_right2[];
extern uint8_t cat_swim_right3[];
extern uint8_t cat_swim_left1[];
extern uint8_t cat_swim_left2[];
extern uint8_t cat_swim_left3[];
extern uint8_t cat_swim_up1[];
extern uint8_t cat_swim_up2[];
extern uint8_t cat_swim_down1[];
extern uint8_t cat_swim_down2[];
extern uint8_t cat_swim_float[];

extern uint8_t cat_feet[5][10];

extern uint8_t title_sprite1[];
extern uint8_t title_sprite2[];
extern uint8_t title_sprite3[];
extern uint8_t title_sprite4[];
extern uint8_t title_sprite6[];

#endif
