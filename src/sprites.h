#ifndef ALLEYCAT_SPRITES_H
#define ALLEYCAT_SPRITES_H

#include <stdint.h>

/*
 * Sprite data is CGA mode 4 packed pixels: 4 pixels per byte, 2 bits each,
 * leftmost pixel in the high bits.  Each 2-bit value indexes cga_pal_1 in
 * render.c -- 0 black, 1 light cyan, 2 light magenta, 3 white.
 *
 * Two conventions live in this file and they are NOT interchangeable:
 *
 *   - Art lifted from the original game (the cat, the fence, the trashcans,
 *     the letters and digits, the logos, the title art) is drawn opaquely.
 *     The cat is the exception: it is drawn with index 3 (white) as the
 *     colour key, which is what the original does.
 *
 *   - The item sprites were authored for this project.  They use index 0 for
 *     the artwork and index 3 as the key, so the keyed blit serves them too.
 */

struct sprite {
	const uint8_t *data;
	int            w;
	int            h;
};

/* Bytes a sprite of this size must occupy.  4 pixels to the byte, rows are
 * padded up to a whole byte. */
#define SPRITE_BYTES(w, h) ((((w) + 3) / 4) * (h))

/* Catch a width/height that does not match the data at compile time -- the
 * mismatch that used to garble the swim frames reads past the array instead. */
#define SPRITE_CHECK(arr, w, h) \
	_Static_assert(SPRITE_BYTES(w, h) == sizeof(arr), \
	               #arr " dimensions do not match its data")

#define CAT_WALK_FRAMES 6

/* ---- raw data ---------------------------------------------------------- */

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

/* ---- descriptors ------------------------------------------------------- */

/* Original game art, drawn opaquely. */
extern const struct sprite spr_fence_top[4];
extern const struct sprite spr_fence_hole[4];
extern const struct sprite spr_fence_under;
extern const struct sprite spr_digits[10];
extern const struct sprite spr_letters[26];
extern const struct sprite spr_punctuation[3];
extern const struct sprite spr_trashcan_lid;
extern const struct sprite spr_trashcan_mid;
extern const struct sprite spr_trashcan_bot;
extern const struct sprite spr_copyright;
extern const struct sprite spr_synsoft;
extern const struct sprite spr_ibm_corp;

/* Title screen artwork.  Sizes recovered from the draw calls in the reference
 * implementation, where the width argument counts bytes rather than pixels. */
extern const struct sprite spr_title_wall;    /* title_sprite1 */
extern const struct sprite spr_title_logo;    /* title_sprite2 */
extern const struct sprite spr_title_emblem;  /* title_sprite3 */
extern const struct sprite spr_title_band;    /* title_sprite4 */
extern const struct sprite spr_title_caption; /* title_sprite6 */

/* The cat, drawn with white as the colour key. */
extern const struct sprite spr_cat_walk_right[CAT_WALK_FRAMES];
extern const struct sprite spr_cat_walk_left[CAT_WALK_FRAMES];
extern const struct sprite spr_cat_head[8];
extern const struct sprite spr_cat_body[4];
extern const struct sprite spr_cat_jump;
extern const struct sprite spr_cat_swim_right[3];
extern const struct sprite spr_cat_swim_left[3];
extern const struct sprite spr_cat_swim_up[2];
extern const struct sprite spr_cat_swim_down[2];
extern const struct sprite spr_cat_swim_float;

/* Room items, also keyed on white. */
extern const struct sprite spr_heart;
extern const struct sprite spr_fish;
extern const struct sprite spr_bone;
extern const struct sprite spr_cheese;
extern const struct sprite spr_bird;
extern const struct sprite spr_dog;
extern const struct sprite spr_broom;
extern const struct sprite spr_mouse;
extern const struct sprite spr_milk_bottle;

#endif
