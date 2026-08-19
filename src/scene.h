#ifndef ALLEYCAT_SCENE_H
#define ALLEYCAT_SCENE_H

#include <SDL.h>

struct scene;

typedef void (*scene_enter_fn)(struct scene *s);
typedef void (*scene_update_fn)(struct scene *s);
typedef void (*scene_render_fn)(struct scene *s);
typedef void (*scene_exit_fn)(struct scene *s);
typedef void (*scene_keydown_fn)(struct scene *s, SDL_Keycode key);
typedef void (*scene_keyup_fn)(struct scene *s, SDL_Keycode key);

struct scene {
	scene_enter_fn   enter;
	scene_update_fn  update;
	scene_render_fn  render;
	scene_exit_fn    exit;
	scene_keydown_fn keydown;
	scene_keyup_fn   keyup;
	void            *data;
};

/*
 * Transitions are requested, not performed.  A scene that asked to be
 * replaced keeps running to the end of its update and is torn down by
 * scene_apply_pending() from the main loop, so nothing here can free a scene
 * that is still on the stack above us.
 *
 * Only the first request in a frame is honoured.  Two win conditions firing
 * on the same tick used to push two scenes and leak one.
 */
void scene_request_push(struct scene *s);
void scene_request_pop(void);
void scene_request_replace(struct scene *s);
void scene_request_reset(struct scene *s);   /* clear the stack, then push */
int  scene_transition_pending(void);
void scene_apply_pending(void);

struct scene *scene_current(void);
void scene_clear(void);
void scene_update(void);
void scene_render(void);
void scene_keydown(SDL_Keycode key);
void scene_keyup(SDL_Keycode key);

#endif
