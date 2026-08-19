#include "input.h"
#include <SDL.h>
#include <string.h>

static const struct {
	SDL_Scancode sc;
	uint8_t      bit;
} key_map[] = {
	{ SDL_SCANCODE_LEFT,   KEY_LEFT   }, { SDL_SCANCODE_A,     KEY_LEFT   },
	{ SDL_SCANCODE_RIGHT,  KEY_RIGHT  }, { SDL_SCANCODE_D,     KEY_RIGHT  },
	{ SDL_SCANCODE_UP,     KEY_UP     }, { SDL_SCANCODE_W,     KEY_UP     },
	{ SDL_SCANCODE_DOWN,   KEY_DOWN   }, { SDL_SCANCODE_S,     KEY_DOWN   },
	{ SDL_SCANCODE_SPACE,  KEY_ACTION }, { SDL_SCANCODE_LCTRL, KEY_ACTION },
	{ SDL_SCANCODE_RCTRL,  KEY_ACTION },
	{ SDL_SCANCODE_ESCAPE, KEY_ESCAPE },
	{ SDL_SCANCODE_RETURN, KEY_RETURN }, { SDL_SCANCODE_KP_ENTER, KEY_RETURN }
};

#define KEY_MAP_LEN ((int)(sizeof(key_map) / sizeof(key_map[0])))

static const uint8_t *keyboard_state;
static uint8_t current_keys;
static uint8_t previous_keys;
static uint8_t latched_keys;

void input_init(void)
{
	keyboard_state = SDL_GetKeyboardState(NULL);
	current_keys   = 0;
	previous_keys  = 0;
	latched_keys   = 0;
}

void input_note_keydown(int scancode)
{
	int i;

	for (i = 0; i < KEY_MAP_LEN; i++)
		if (key_map[i].sc == (SDL_Scancode)scancode)
			latched_keys |= key_map[i].bit;
}

void input_update(void)
{
	uint8_t keys = 0;
	int i;

	if (keyboard_state) {
		for (i = 0; i < KEY_MAP_LEN; i++)
			if (keyboard_state[key_map[i].sc])
				keys |= key_map[i].bit;
	}

	/* A tap shorter than one logic tick is over before the keyboard state is
	 * next sampled, so keydown events latch a bit that survives to here.
	 * Without this a quick press is simply lost. */
	keys |= latched_keys;
	latched_keys = 0;

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
	latched_keys  = 0;
}
