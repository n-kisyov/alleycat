#ifndef ALLEYCAT_SCENE_H
#define ALLEYCAT_SCENE_H

#include <SDL.h>

struct scene;

typedef void (*scene_enter_fn)(struct scene *s);
typedef void (*scene_update_fn)(struct scene *s, float dt);
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
	void *data;
};

void scene_push(struct scene *s);
void scene_pop(void);
void scene_replace(struct scene *s);
struct scene *scene_current(void);
void scene_update(float dt);
void scene_render(void);
void scene_keydown(SDL_Keycode key);
void scene_keyup(SDL_Keycode key);

#endif
