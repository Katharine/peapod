#ifndef ipod_progress_bar_h
#define ipod_progress_bar_h

#include "pebble_os.h"

typedef struct {
    Layer layer;
    int32_t min;
    int32_t max;
    int32_t value;
    GColor bar_colour;
    GColor background_colour;
    GColor frame_colour;
} ProgressBarLayer;

void progress_bar_layer_init(ProgressBarLayer* bar, GRect frame);
void progress_bar_layer_set_range(ProgressBarLayer* bar, int32_t min, int32_t max);
void progress_bar_layer_set_value(ProgressBarLayer* bar, int32_t value);

#endif
