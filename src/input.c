#include "input.h"
#include <SDL.h>
#include <string.h>

static const uint8_t *keyboard_state;
static uint8_t current_keys;
static uint8_t previous_keys;
static int key_states[SDL_NUM_SCANCODES];
static int prev_key_states[SDL_NUM_SCANCODES];

void input_init(void)
{
	keyboard_state = SDL_GetKeyboardState(NULL);
	memset(key_states, 0, sizeof(key_states));
	memset(prev_key_states, 0, sizeof(prev_key_states));
}

void input_update(void)
{
	memcpy(prev_key_states, key_states, sizeof(key_states));
	for (int i = 0; i < SDL_NUM_SCANCODES; i++)
		key_states[i] = keyboard_state[i] ? 1 : 0;

	previous_keys = current_keys;
	current_keys = 0;

	if (keyboard_state[SDL_SCANCODE_LEFT] || keyboard_state[SDL_SCANCODE_A])
		current_keys |= KEY_LEFT;
	if (keyboard_state[SDL_SCANCODE_RIGHT] || keyboard_state[SDL_SCANCODE_D])
		current_keys |= KEY_RIGHT;
	if (keyboard_state[SDL_SCANCODE_UP] || keyboard_state[SDL_SCANCODE_W])
		current_keys |= KEY_UP;
	if (keyboard_state[SDL_SCANCODE_DOWN] || keyboard_state[SDL_SCANCODE_S])
		current_keys |= KEY_DOWN;
	if (keyboard_state[SDL_SCANCODE_SPACE] || keyboard_state[SDL_SCANCODE_LCTRL] ||
	    keyboard_state[SDL_SCANCODE_RCTRL])
		current_keys |= KEY_ACTION;
	if (keyboard_state[SDL_SCANCODE_ESCAPE])
		current_keys |= KEY_ESCAPE;
	if (keyboard_state[SDL_SCANCODE_RETURN])
		current_keys |= KEY_RETURN;
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
	return ((current_keys & key) != 0) && ((previous_keys & key) == 0);
}
