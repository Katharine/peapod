#include "now_playing.h"
#include "pebble_os.h"
#include "pebble_fonts.h"
#include "pebble_app.h"
#include "marquee_text.h"
#include "common.h"

static Window window;
static ActionBarLayer action_bar;
static MarqueeTextLayer title_layer;
static MarqueeTextLayer album_layer;
static MarqueeTextLayer artist_layer;
static BitmapLayer album_art_layer;
static GBitmap album_art_bitmap;
static uint8_t album_art_data[512];

// Action bar icons
static HeapBitmap icon_pause;
static HeapBitmap icon_play;
static HeapBitmap icon_fast_forward;
static HeapBitmap icon_rewind;
static HeapBitmap icon_volume_up;
static HeapBitmap icon_volume_down;

static AppMessageCallbacksNode app_callbacks;

static void click_config_provider(ClickConfig **config, void *context);
static void window_unload(Window* window);
static void window_load(Window* window);
static void clicked_up(ClickRecognizerRef recognizer, void *context);
static void clicked_select(ClickRecognizerRef recognizer, void *context);
static void long_clicked_select(ClickRecognizerRef recognizer, void *context);
static void clicked_down(ClickRecognizerRef recognizer, void *context);
static void request_now_playing();
static void send_state_change(int8_t change);

static void app_in_received(DictionaryIterator *received, void *context);

static char s_title[100];
static char s_album[100];
static char s_artist[100];

static bool controlling_volume = false;

void show_now_playing() {
    window_init(&window, "Now playing");
    window_set_window_handlers(&window, (WindowHandlers){
        .unload = window_unload,
        .load = window_load,
    });
    window_stack_push(&window, true);
}

static void window_load(Window* window) {
    // Load bitmaps for action bar icons.
    heap_bitmap_init(&icon_pause, RESOURCE_ID_ICON_PAUSE);
    heap_bitmap_init(&icon_play, RESOURCE_ID_ICON_PLAY);
    heap_bitmap_init(&icon_fast_forward, RESOURCE_ID_ICON_FAST_FORWARD);
    heap_bitmap_init(&icon_rewind, RESOURCE_ID_ICON_REWIND);
    heap_bitmap_init(&icon_volume_up, RESOURCE_ID_ICON_VOLUME_UP);
    heap_bitmap_init(&icon_volume_down, RESOURCE_ID_ICON_VOLUME_DOWN);
    // Action bar
    action_bar_layer_init(&action_bar);
    action_bar_layer_add_to_window(&action_bar, window);
    action_bar_layer_set_click_config_provider(&action_bar, click_config_provider);
    // Set default icon set.
    action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &icon_fast_forward.bmp);
    action_bar_layer_set_icon(&action_bar, BUTTON_ID_DOWN, &icon_rewind.bmp);
    action_bar_layer_set_icon(&action_bar, BUTTON_ID_SELECT, &icon_play.bmp);
    
    // Text labels
    marquee_text_layer_init(&title_layer, GRect(2, 2, 118, 30));
    marquee_text_layer_set_font(&title_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
    marquee_text_layer_set_text(&title_layer, "Loading...");
    marquee_text_layer_init(&album_layer, GRect(2, 130, 118, 20));
    marquee_text_layer_set_font(&album_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
    marquee_text_layer_set_text(&album_layer, "");
    marquee_text_layer_init(&artist_layer, GRect(2, 107, 118, 25));
    marquee_text_layer_set_font(&artist_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
    marquee_text_layer_set_text(&artist_layer, "");
    
    layer_add_child(window_get_root_layer(window), &title_layer.layer);
    layer_add_child(window_get_root_layer(window), &album_layer.layer);
    layer_add_child(window_get_root_layer(window), &artist_layer.layer);
    
    // Album art
    album_art_bitmap = (GBitmap) {
        .addr = album_art_data,
        .bounds = GRect(0, 0, 64, 64),
        .info_flags = 1,
        .row_size_bytes = 8,
    };
    memset(album_art_data, 0, 512);
    bitmap_layer_init(&album_art_layer, GRect(30, 40, 64, 64));
    bitmap_layer_set_bitmap(&album_art_layer, &album_art_bitmap);
    layer_add_child(window_get_root_layer(window), &album_art_layer.layer);
    
    app_callbacks = (AppMessageCallbacksNode){
        .callbacks = {
            .in_received = app_in_received,
        }
    };
    app_message_register_callbacks(&app_callbacks);
    request_now_playing();
}

static void window_unload(Window* window) {
    action_bar_layer_remove_from_window(&action_bar);
    marquee_text_layer_deinit(&title_layer);
    marquee_text_layer_deinit(&album_layer);
    marquee_text_layer_deinit(&artist_layer);
    app_message_deregister_callbacks(&app_callbacks);
    
    // deinit action bar icons
    heap_bitmap_deinit(&icon_pause);
    heap_bitmap_deinit(&icon_play);
    heap_bitmap_deinit(&icon_fast_forward);
    heap_bitmap_deinit(&icon_rewind);
    heap_bitmap_deinit(&icon_volume_up);
    heap_bitmap_deinit(&icon_volume_down);
}

static void click_config_provider(ClickConfig **config, void* context) {
    config[BUTTON_ID_DOWN]->click.handler = clicked_down;
    config[BUTTON_ID_UP]->click.handler = clicked_up;
    config[BUTTON_ID_SELECT]->click.handler = clicked_select;
    config[BUTTON_ID_SELECT]->long_click.handler = long_clicked_select;
}

static void clicked_up(ClickRecognizerRef recognizer, void *context) {
    if(!controlling_volume) {
        send_state_change(1);
    } else {
        send_state_change(64);
    }
}
static void clicked_select(ClickRecognizerRef recognizer, void *context) {
    send_state_change(0);
}
static void clicked_down(ClickRecognizerRef recognizer, void *context) {
    if(!controlling_volume) {
        send_state_change(-1);
    } else {
        send_state_change(-64);
    }
}
static void long_clicked_select(ClickRecognizerRef recognizer, void *context) {
    controlling_volume = !controlling_volume;
    if(controlling_volume) {
        action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &icon_volume_up.bmp);
        action_bar_layer_set_icon(&action_bar, BUTTON_ID_DOWN, &icon_volume_down.bmp);
    } else {
        action_bar_layer_set_icon(&action_bar, BUTTON_ID_UP, &icon_fast_forward.bmp);
        action_bar_layer_set_icon(&action_bar, BUTTON_ID_DOWN, &icon_rewind.bmp);
    }
}

static void send_state_change(int8_t state_change) {
    DictionaryIterator *iter;
    app_message_out_get(&iter);
    if(!iter) return;
    dict_write_int8(iter, IPOD_STATE_CHANGE_KEY, state_change);
    app_message_out_send();
    app_message_out_release();
}

static void request_now_playing() {
    DictionaryIterator *iter;
    app_message_out_get(&iter);
    if(!iter) return;
    dict_write_int8(iter, IPOD_NOW_PLAYING_KEY, 2);
    app_message_out_send();
    app_message_out_release();
}

static void app_in_received(DictionaryIterator *received, void* context) {
    Tuple* tuple = dict_find(received, IPOD_NOW_PLAYING_RESPONSE_TYPE_KEY);
    if(tuple) {
        NowPlayingType type = tuple->value->uint8;
        tuple = dict_find(received, IPOD_NOW_PLAYING_KEY);
        if(!tuple) return;
        char* target = NULL;
        MarqueeTextLayer *layer = NULL;
        switch(type) {
            case NowPlayingAlbum:
                target = s_album;
                layer = &album_layer;
                break;
            case NowPlayingArtist:
                target = s_artist;
                layer = &artist_layer;
                break;
            case NowPlayingTitle:
                target = s_title;
                layer = &title_layer;
                break;
            default:
                return;
        }
        uint8_t len = strlen(tuple->value->cstring);
        if(len > 99) len = 99;
        memcpy(target, tuple->value->cstring, len);
        target[len] = '\0';
        marquee_text_layer_set_text(layer, target);
        return;
    }
    tuple = dict_find(received, IPOD_ALBUM_ART_KEY);
    if(tuple) {
        size_t offset = tuple->value->data[0] * 104;
        memcpy(album_art_data + offset, tuple->value->data + 1, tuple->length - 1);
        layer_mark_dirty(&album_art_layer.layer);
    }
}
