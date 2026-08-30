#include "scene.h"
#include <stdlib.h>

#define MAX_SCENE_STACK 16

enum transition { TRANS_NONE = 0, TRANS_PUSH, TRANS_POP, TRANS_REPLACE, TRANS_RESET };

static struct scene *scene_stack[MAX_SCENE_STACK];
static int scene_stack_top = -1;

static enum transition pending_kind;
static struct scene   *pending_scene;

static void do_push(struct scene *s)
{
	if (!s)
		return;
	if (scene_stack_top >= MAX_SCENE_STACK - 1) {
		/* Nothing sane to do with it, and leaking would be worse. */
		free(s);
		return;
	}
	scene_stack[++scene_stack_top] = s;
	if (s->enter)
		s->enter(s);
}

static void do_pop(void)
{
	struct scene *s;

	if (scene_stack_top < 0)
		return;
	s = scene_stack[scene_stack_top--];
	if (s->exit)
		s->exit(s);
	free(s);
}

static void request(enum transition kind, struct scene *s)
{
	if (pending_kind != TRANS_NONE) {
		/* Already committed this frame; drop the loser so it does not leak. */
		if (s) {
			free(s);
		}
		return;
	}
	pending_kind  = kind;
	pending_scene = s;
}

void scene_request_push(struct scene *s)    { request(TRANS_PUSH, s); }
void scene_request_pop(void)                { request(TRANS_POP, NULL); }
void scene_request_replace(struct scene *s) { request(TRANS_REPLACE, s); }
void scene_request_reset(struct scene *s)   { request(TRANS_RESET, s); }

int scene_transition_pending(void)
{
	return pending_kind != TRANS_NONE;
}

void scene_apply_pending(void)
{
	enum transition kind = pending_kind;
	struct scene *s = pending_scene;

	pending_kind  = TRANS_NONE;
	pending_scene = NULL;

	switch (kind) {
	case TRANS_NONE:
		break;
	case TRANS_PUSH:
		do_push(s);
		break;
	case TRANS_POP:
		do_pop();
		break;
	case TRANS_REPLACE:
		do_pop();
		do_push(s);
		break;
	case TRANS_RESET:
		while (scene_stack_top >= 0)
			do_pop();
		do_push(s);
		break;
	}
}

void scene_clear(void)
{
	while (scene_stack_top >= 0)
		do_pop();
	if (pending_scene) {
		if (pending_scene->exit)
			pending_scene->exit(pending_scene);
		free(pending_scene);
		pending_scene = NULL;
	}
	pending_kind = TRANS_NONE;
}

struct scene *scene_current(void)
{
	return scene_stack_top >= 0 ? scene_stack[scene_stack_top] : NULL;
}

void scene_update(void)
{
	struct scene *s = scene_current();
	if (s && s->update)
		s->update(s);
}

void scene_render(void)
{
	struct scene *s = scene_current();
	if (s && s->render)
		s->render(s);
}

void scene_keydown(SDL_Keycode key)
{
	struct scene *s = scene_current();
	if (s && s->keydown)
		s->keydown(s, key);
}

void scene_keyup(SDL_Keycode key)
{
	struct scene *s = scene_current();
	if (s && s->keyup)
		s->keyup(s, key);
}
