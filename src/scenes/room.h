#ifndef ALLEYCAT_SCENES_ROOM_H
#define ALLEYCAT_SCENES_ROOM_H

#include "../scene.h"
#include "../render.h"

/*
 * Every room ended with the same block of copy-pasted bookkeeping, and each
 * copy carried the same use-after-free: scene_replace() freed the scene's data
 * and the next statement read it again.  Rooms now call one of these and
 * return; the transition itself is deferred to the end of the frame.
 */

#define ROOM_HUD_H     16
#define ROOM_HUD_Y     (SCREEN_H - ROOM_HUD_H)
#define ROOM_FLOOR_Y   (ROOM_HUD_Y - 8)
#define ROOM_CEILING_Y 20

/* Cleared: bank the points, mark the window shut, drop back to the alley. */
void room_succeed(int room, int points);

/* Ran out of time, or got caught.  Costs a life, and ends the run if that
 * was the last one. */
void room_fail(const char *reason);

/* Player bailed out with Escape: no reward, no penalty. */
void room_leave(void);

/* Standard chrome: title, progress, and the countdown that used to be
 * invisible right up until it cost you a life. */
void room_draw_hud(int room, int collected, int total, int ticks_left);

/* Walls and floor, in the four colours the display actually has. */
void room_draw_walls(uint8_t floor_color);

#endif
