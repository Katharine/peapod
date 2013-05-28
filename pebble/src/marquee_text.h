#ifndef ipod_marquee_text_h
#define ipod_marquee_text_h

#include "pebble_os.h"

typedef struct MarqueeTextLayer {
    Layer layer;
    const char* text;
    GFont font;
    GColor text_colour;
    GColor background_colour;
    int16_t offset;
    int16_t text_width;
	struct MarqueeTextLayer* previous;
	struct MarqueeTextLayer* next;
    Animation animation;
    AnimationHandlers animation_handlers;
    AnimationImplementation animation_implementation;
    bool ready;
} MarqueeTextLayer;

void marquee_text_layer_tick();
void marquee_text_layer_mark_dirty(MarqueeTextLayer *marquee);
void marquee_text_layer_init(MarqueeTextLayer *marquee, GRect frame);
void marquee_text_layer_deinit(MarqueeTextLayer *marquee);
void marquee_text_layer_set_text(MarqueeTextLayer *marquee, const char *text);
void marquee_text_layer_set_font(MarqueeTextLayer *marquee, GFont font);
void marquee_text_layer_set_text_color(MarqueeTextLayer *marquee, GColor color);
void marquee_text_layer_set_background_color(MarqueeTextLayer *marquee, GColor color);

#endif
