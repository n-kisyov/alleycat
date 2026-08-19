#include "input.h"
#include <SDL.h>
#include <string.h>

static const uint8_t *keyboard_state;
static uint8_t current_keys;
static uint8_t previous_keys;

void input_init(void)
{
	keyboard_state = SDL_GetKeyboardState(NULL);
	current_keys   = 0;
	previous_keys  = 0;
}

void input_update(void)
{
	uint8_t keys = 0;

	if (!keyboard_state)
		return;

	if (keyboard_state[SDL_SCANCODE_LEFT] || keyboard_state[SDL_SCANCODE_A])
		keys |= KEY_LEFT;
	if (keyboard_state[SDL_SCANCODE_RIGHT] || keyboard_state[SDL_SCANCODE_D])
		keys |= KEY_RIGHT;
	if (keyboard_state[SDL_SCANCODE_UP] || keyboard_state[SDL_SCANCODE_W])
		keys |= KEY_UP;
	if (keyboard_state[SDL_SCANCODE_DOWN] || keyboard_state[SDL_SCANCODE_S])
		keys |= KEY_DOWN;
	if (keyboard_state[SDL_SCANCODE_SPACE] || keyboard_state[SDL_SCANCODE_LCTRL] ||
	    keyboard_state[SDL_SCANCODE_RCTRL])
		keys |= KEY_ACTION;
	if (keyboard_state[SDL_SCANCODE_ESCAPE])
		keys |= KEY_ESCAPE;
	if (keyboard_state[SDL_SCANCODE_RETURN])
		keys |= KEY_RETURN;

	previous_keys = current_keys;
	current_keys  = keys;
}

uint8_t input_get(void)
{
	return current_keys;
}

int input_key_pressed(int key)
{
	return (current_keys & key) != 0;
}

int input_key_just_pressed(int key)
{
	return (current_keys & key) != 0 && (previous_keys & key) == 0;
}

void input_clear_edges(void)
{
	previous_keys = current_keys;
}
