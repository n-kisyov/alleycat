#ifndef ALLEYCAT_SOUND_H
#define ALLEYCAT_SOUND_H

#include <stdint.h>

void sound_init(void);
void sound_close(void);

void sound_play_tone(int frequency, int duration_ms);

/* Starts the intro tune, looping.  Effects take the speaker while they play. */
void sound_play_intro_music(void);
void sound_stop_music(void);

/* Silences everything: music and any queued effects. */
void sound_stop(void);

#endif
