#ifndef ALLEYCAT_SOUND_H
#define ALLEYCAT_SOUND_H

#include <stdint.h>

void sound_init(void);
void sound_close(void);
void sound_play_tone(int frequency, int duration_ms);
void sound_play_intro_music(void);
void sound_stop(void);
void sound_update(void);

#endif
