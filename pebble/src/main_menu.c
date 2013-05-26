#include "main_menu.h"
#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "common.h"
#include "library_menus.h"

static char s_now_playing[40];

static void open_now_playing(int index, void* context);
static void open_artist_list(int index, void* context);
static void open_album_list(int index, void* context);
static void open_playlist_list(int index, void* context);
static void open_composer_list(int index, void* context);
static void open_genre_list(int index, void* context);
static void open_song_list(int index, void* context);

static void received_message(DictionaryIterator *received, void *context);

static SimpleMenuItem main_menu_items[] = {
    {
        .title = "Now Playing",
        .subtitle = s_now_playing,
        .callback = open_now_playing,
    },
    {
        .title = "Playlists",
        .callback = open_playlist_list,
    },
    {
        .title = "Artists",
        .callback = open_artist_list,
    },
    {
        .title = "Albums",
        .callback = open_album_list,
    },
    {
        .title = "Composers",
        .callback = open_composer_list,
    },
    {
        .title = "Genres",
        .callback = open_genre_list,
    },
    //{
    //    .title = "All songs",
    //    .callback = open_song_list,
    //}
};

static SimpleMenuSection section = {
    .items = main_menu_items,
    .num_items = ARRAY_LENGTH(main_menu_items),
};

static SimpleMenuLayer main_menu_layer;

static AppMessageCallbacksNode app_callbacks;

void main_menu_init(Window* window) {
    memcpy(s_now_playing, "Loading...", 11);
    simple_menu_layer_init(&main_menu_layer, GRect(0, 0, 144, 152), window, &section, 1, NULL);
    layer_add_child(window_get_root_layer(window), simple_menu_layer_get_layer(&main_menu_layer));
    app_callbacks = (AppMessageCallbacksNode){
        .callbacks = {
            .in_received = received_message
        }
    };
    app_message_register_callbacks(&app_callbacks);
    // Figure out what *is* playing.
    DictionaryIterator *iter;
    app_message_out_get(&iter);
    dict_write_uint8(iter, IPOD_NOW_PLAYING_KEY, 0);
    app_message_out_send();
    app_message_out_release();
}

void main_menu_set_now_playing(char* now_playing) {
    size_t len = strlen(now_playing);
    memcpy(s_now_playing, now_playing, len < 40 ? len + 1 : 40);
}

static void received_message(DictionaryIterator *received, void* context) {
    Tuple* tuple = dict_find(received, IPOD_NOW_PLAYING_KEY);
    if(tuple) {
        uint8_t length = tuple->length > 30 ? 30 : tuple->length;
        memcpy(s_now_playing, tuple->value->cstring, length);
        layer_mark_dirty(simple_menu_layer_get_layer(&main_menu_layer));
    }
}

static void open_now_playing(int index, void* context) {}
static void open_artist_list(int index, void* context) {
    display_library_view(MPMediaGroupingAlbumArtist);
}
static void open_album_list(int index, void* context) {
    display_library_view(MPMediaGroupingAlbum);
}
static void open_playlist_list(int index, void* context) {
    display_library_view(MPMediaGroupingPlaylist);
}
static void open_genre_list(int index, void* context) {
    display_library_view(MPMediaGroupingGenre);
}
static void open_composer_list(int index, void* context) {
    display_library_view(MPMediaGroupingComposer);
}
static void open_song_list(int index, void* context) {
    display_library_view(MPMediaGroupingTitle);
}
