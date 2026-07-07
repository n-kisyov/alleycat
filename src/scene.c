#include "scene.h"
#include <stdlib.h>

#define MAX_SCENE_STACK 16

static struct scene *scene_stack[MAX_SCENE_STACK];
static int scene_stack_top = -1;

void scene_push(struct scene *s)
{
	if (scene_stack_top < MAX_SCENE_STACK - 1) {
		scene_stack[++scene_stack_top] = s;
		if (s->enter) s->enter(s);
	}
}

void scene_pop(void)
{
	if (scene_stack_top >= 0) {
		struct scene *s = scene_stack[scene_stack_top--];
		if (s->exit) s->exit(s);
		free(s);
	}
}

void scene_replace(struct scene *s)
{
	scene_pop();
	scene_push(s);
}

struct scene *scene_current(void)
{
	if (scene_stack_top >= 0)
		return scene_stack[scene_stack_top];
	return NULL;
}

void scene_update(float dt)
{
	struct scene *s = scene_current();
	if (s && s->update) s->update(s, dt);
}

void scene_render(void)
{
	struct scene *s = scene_current();
	if (s && s->render) s->render(s);
}

void scene_keydown(SDL_Keycode key)
{
	struct scene *s = scene_current();
	if (s && s->keydown) s->keydown(s, key);
}

void scene_keyup(SDL_Keycode key)
{
	struct scene *s = scene_current();
	if (s && s->keyup) s->keyup(s, key);
}
