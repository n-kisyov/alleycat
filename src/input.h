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

void input_init(void);
void input_update(void);
uint8_t input_get(void);
int input_key_pressed(int key);
int input_key_just_pressed(int key);

#endif
