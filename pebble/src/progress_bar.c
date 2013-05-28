#include "progress_bar.h"

static void update_proc(Layer* layer, GContext *context);

void progress_bar_layer_init(ProgressBarLayer* bar, GRect frame) {
    layer_init(&bar->layer, frame);
    bar->layer.update_proc = update_proc;
    bar->min = 0;
    bar->max = 255;
    bar->value = 0;
    bar->background_colour = GColorWhite;
    bar->frame_colour = GColorBlack;
    bar->bar_colour = GColorBlack;
}

void progress_bar_layer_set_range(ProgressBarLayer* bar, int32_t min, int32_t max) {
    bar->max = max;
    bar->min = min;
    layer_mark_dirty(&bar->layer);
}

void progress_bar_layer_set_value(ProgressBarLayer* bar, int32_t value) {
    bar->value = value;
    layer_mark_dirty(&bar->layer);
}

static void update_proc(Layer* layer, GContext *context) {
    ProgressBarLayer* bar = (ProgressBarLayer*)layer;
    GRect bounds = layer_get_bounds(layer);
    // graphics_draw_rect doesn't actually exist, so we use this hack instead.
    graphics_context_set_fill_color(context, bar->frame_colour);
    graphics_fill_rect(context, bounds, 3, GCornersAll);
    bounds.origin.x += 1;
    bounds.origin.y += 1;
    bounds.size.w -= 2;
    bounds.size.h -= 2;
    graphics_context_set_fill_color(context, bar->background_colour);
    graphics_fill_rect(context, bounds, 3, GCornersAll);
    bounds.size.w = ((bar->value - bar->min) * bounds.size.w) / (bar->max - bar->min);
    GCornerMask corners = GCornersLeft;
    if(bounds.size.w >= layer_get_bounds(layer).size.w - 4) {
        corners |= GCornersRight;
    }
    graphics_context_set_fill_color(context, bar->bar_colour);
    graphics_fill_rect(context, bounds, 3, corners);
}
