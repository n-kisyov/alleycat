#ifndef ALLEYCAT_INPUT_H
#define ALLEYCAT_INPUT_H

#include <stdint.h>

#define KEY_LEFT   0x01
#define KEY_RIGHT  0x02
#define KEY_UP     0x04
#define KEY_DOWN   0x08
#define KEY_ACTION 0x10
#define KEY_ESCAPE 0x20
#define KEY_RETURN 0x40

void    input_init(void);
void    input_update(void);

/* Called from the event loop for every key press.  A tap shorter than one
 * logic tick would otherwise be gone before the keyboard state is sampled. */
void    input_note_keydown(int scancode);
uint8_t input_get(void);
int     input_key_pressed(int key);
int     input_key_just_pressed(int key);

/* Swallow the current press edges.  Called on a scene change so the key that
 * started the game does not also make the cat jump on arrival. */
void    input_clear_edges(void);

#endif
