#include "pebble_os.h"
#include "pebble_fonts.h"
#include "marquee_text.h"

static MarqueeTextLayer* head;

static void do_draw(Layer* layer, GContext* context);

void marquee_text_layer_init(MarqueeTextLayer *marquee, GRect frame) {
    layer_init(&marquee->layer, frame);
    marquee->layer.update_proc = do_draw;
    marquee->background_colour = GColorWhite;
    marquee->text_colour = GColorBlack;
    marquee->offset = 0;
	marquee->text_width = -1;
    marquee->countdown = 100;
    marquee->font = fonts_get_system_font(FONT_KEY_FONT_FALLBACK);
    marquee_text_layer_mark_dirty(marquee);
	// Update the list
	if(head)
		head->next = marquee;
	marquee->previous = head;
	marquee->next = NULL;
	head = marquee;
}

void marquee_text_layer_deinit(MarqueeTextLayer *marquee) {
	// Remove the marquee from the timer sequence.
	if(marquee == head) {
		head = marquee->previous;
	}
	if(marquee->next) {
		marquee->next->previous = marquee->previous;
	}
	if(marquee->previous) {
		marquee->previous->next = marquee->next;
	}
}

void marquee_text_layer_set_text(MarqueeTextLayer *marquee, const char *text) {
    marquee->text = text;
    marquee_text_layer_mark_dirty(marquee);
}

void marquee_text_layer_set_font(MarqueeTextLayer *marquee, GFont font) {
    marquee->font = font;
    marquee_text_layer_mark_dirty(marquee);
}

void marquee_text_layer_set_text_color(MarqueeTextLayer *marquee, GColor color) {
    marquee->text_colour = color;
    marquee_text_layer_mark_dirty(marquee);
}

void marquee_text_layer_set_background_color(MarqueeTextLayer *marquee, GColor color) {
    marquee->background_colour = color;
    marquee_text_layer_mark_dirty(marquee);
}

void marquee_text_layer_mark_dirty(MarqueeTextLayer *marquee) {
    marquee->text_width = -1;
    marquee->offset = 0;
    marquee->countdown = 100;
    layer_mark_dirty(&marquee->layer);
}

void marquee_text_layer_tick() {
	MarqueeTextLayer *marquee = head;
	while(marquee) {
		if(marquee->countdown > 0) {
			--marquee->countdown;
			goto next;
		}
		marquee->offset += 1;
		layer_mark_dirty(&marquee->layer);
	next:
		marquee = marquee->previous;
	}
}

static void do_draw(Layer* layer, GContext* context) {
    MarqueeTextLayer *marquee = (MarqueeTextLayer*)layer;
    if(marquee->text[0] == '\0') return; // empty strings are very bad.
    if(marquee->text_width == -1) {
        marquee->text_width = graphics_text_layout_get_max_used_size(context, marquee->text, marquee->font, GRect(0, 0, 1000, layer_get_frame(layer).size.h), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL).w;
	}
    graphics_context_set_fill_color(context, marquee->background_colour);
    graphics_context_set_text_color(context, marquee->text_colour);
    graphics_fill_rect(context, layer_get_bounds(&marquee->layer), 0, GCornerNone);
	if(marquee->text_width < layer_get_frame(&marquee->layer).size.w) {
		graphics_text_draw(context, marquee->text, marquee->font, layer_get_bounds(&marquee->layer), GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter, NULL);
		return;
	}
    if(marquee->offset > marquee->text_width + 30) {
        marquee->offset = 0;
		marquee->countdown = 100;
    }
    if(marquee->offset < marquee->text_width) {
        graphics_text_draw(context, marquee->text, marquee->font, GRect(-marquee->offset, 0, marquee->text_width, layer_get_frame(layer).size.h), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    }
    if(marquee->offset > marquee->text_width - layer_get_frame(layer).size.w + 30) {
        graphics_text_draw(context, marquee->text, marquee->font, GRect(-marquee->offset + marquee->text_width + 30, 0, marquee->text_width, layer_get_frame(layer).size.h), GTextOverflowModeTrailingEllipsis, GTextAlignmentLeft, NULL);
    }
}
