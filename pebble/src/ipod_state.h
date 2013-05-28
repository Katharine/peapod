#ifndef ipod_ipod_state_h
#define ipod_ipod_state_h

#include "pebble_os.h"
#include "common.h"

typedef void(*iPodStateCallback)();

void ipod_state_init();
void ipod_state_tick();

void ipod_state_set_callback(iPodStateCallback);

uint16_t ipod_state_current_time();
uint16_t ipod_state_duration();
MPMusicPlaybackState ipod_get_playback_state();
MPMusicRepeatMode ipod_get_repeat_mode();
MPMusicShuffleMode ipod_get_shuffle_mode();

char* ipod_get_album();
char* ipod_get_artist();
char* ipod_get_title();

void ipod_set_shuffle_mode(MPMusicShuffleMode shuffle);
void ipod_set_repeat_mode(MPMusicRepeatMode repeat);

#endif
