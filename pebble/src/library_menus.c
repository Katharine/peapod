#include "pebble_os.h"
#include "library_menus.h"
#include "common.h"

#define MENU_CACHE_COUNT 15
#define MENU_ENTRY_LENGTH 21
#define MENU_STACK_DEPTH 4 // Deepest: genres -> artists -> albums -> songs

typedef struct {
    MenuLayer layer;
    Window window;
    char menu_entries[MENU_CACHE_COUNT][MENU_ENTRY_LENGTH];
    uint16_t total_entry_count;
    uint16_t current_entry_offset;
    uint16_t last_entry;
    uint16_t current_selection;
    MPMediaGrouping grouping;
} LibraryMenu;

static LibraryMenu menu_stack[MENU_STACK_DEPTH];
static int8_t menu_stack_pointer;

static AppMessageCallbacksNode app_callbacks;

static bool send_library_request(MPMediaGrouping grouping, uint32_t offset);
static bool play_track(uint16_t index);
static void received_message(DictionaryIterator *received, void* context);
// Menu callbacks
static uint16_t get_num_rows(MenuLayer* layer, uint16_t section_index, void* context);
static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context);
static void selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context);
static void select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context);

static void window_disappeared(Window* window);

void init_library_menus() {
    app_callbacks = (AppMessageCallbacksNode){
        .callbacks = {
            .in_received = received_message
        }
    };
    app_message_register_callbacks(&app_callbacks);
    menu_stack_pointer = -1;
}

static bool build_parent_history(DictionaryIterator *iter) {
    if(menu_stack_pointer > 0) {
        // Pack up the parent stuff.
        uint8_t parents[MENU_STACK_DEPTH*3+1];
        parents[0] = menu_stack_pointer;
        for(uint8_t i = 0; i < menu_stack_pointer; ++i) {
            parents[i*3+1] = menu_stack[i].grouping;
            parents[i*3+3] = menu_stack[i].current_selection >> 8;
            parents[i*3+2] = menu_stack[i].current_selection & 0xFF;
        }
        if(dict_write_data(iter, IPOD_REQUEST_PARENT_KEY, parents, ARRAY_LENGTH(parents)) != DICT_OK) {
            return false;
        }
    }
    return true;
}

static bool send_library_request(MPMediaGrouping grouping, uint32_t offset) {
    DictionaryIterator *iter;
    if(app_message_out_get(&iter) != APP_MSG_OK) goto failed;
    dict_write_uint8(iter, IPOD_REQUEST_LIBRARY_KEY, grouping);
    dict_write_uint32(iter, IPOD_REQUEST_OFFSET_KEY, offset);
    build_parent_history(iter);
    if(app_message_out_send() != APP_MSG_OK) goto failed;
    app_message_out_release();
    return true;
failed:
    app_message_out_release();
    return false;
}

static bool play_track(uint16_t index) {
    DictionaryIterator *iter;
    LibraryMenu* menu = &menu_stack[menu_stack_pointer];
    if(app_message_out_get(&iter) != APP_MSG_OK) goto failed;
    dict_write_uint8(iter, IPOD_REQUEST_LIBRARY_KEY, menu->grouping);
    dict_write_uint16(iter, IPOD_PLAY_TRACK_KEY, menu->current_selection);
    build_parent_history(iter);
    if(app_message_out_send() != APP_MSG_OK) goto failed;
    app_message_out_release();
    return true;
failed:
    app_message_out_release();
    return false;
}

void display_library_view(MPMediaGrouping grouping) {
    if(menu_stack_pointer >= MENU_STACK_DEPTH) return;
    LibraryMenu* menu = &menu_stack[++menu_stack_pointer];
    menu->grouping = grouping;
    menu->total_entry_count = 0;
    menu->current_entry_offset = 0;
    menu->last_entry = 0;
    menu->current_selection = 0;
    memset(menu->menu_entries, 0, MENU_CACHE_COUNT * MENU_ENTRY_LENGTH);
    window_init(&menu->window, "Library window");
    menu_layer_init(&menu->layer, GRect(0, 0, 144, 152));
    menu_layer_set_click_config_onto_window(&menu->layer, &menu->window);
    menu_layer_set_callbacks(&menu->layer, menu, (MenuLayerCallbacks){
        .get_num_rows = get_num_rows,
        .draw_row = draw_row,
        .selection_changed = selection_changed,
        .select_click = select_click,
    });
    layer_add_child(window_get_root_layer(&menu->window), menu_layer_get_layer(&menu->layer));
    
    window_stack_push(&menu->window, true);
    window_set_window_handlers(&menu->window, (WindowHandlers) {
        .unload = window_disappeared,
    });
    send_library_request(grouping, 0);
}

static void received_message(DictionaryIterator *received, void* context) {
    if(menu_stack_pointer == -1) return;
    Tuple* tuple = dict_find(received, IPOD_LIBRARY_RESPONSE_KEY);
    LibraryMenu *menu = &menu_stack[menu_stack_pointer];
    if(tuple) {
        MPMediaGrouping grouping = tuple->value->data[0];
        if(grouping != menu->grouping) return; // Not what we wanted.
        uint16_t total_size = *(uint16_t*)&tuple->value->data[1];
        uint16_t offset = *(uint16_t*)&tuple->value->data[3];
        int8_t insert_pos = offset - menu->current_entry_offset;
        menu->total_entry_count = total_size;
        uint8_t skipping = 0;
        if(insert_pos < 0) {
            // Don't go further than 5 back.
            if(insert_pos < -5) {
                skipping = -5 - insert_pos;
            }
            memmove(&menu->menu_entries[5], &menu->menu_entries[0], (MENU_CACHE_COUNT - 5) * MENU_ENTRY_LENGTH);
            menu->last_entry += 5;
            menu->current_entry_offset = (menu->current_entry_offset < 5) ? 0 : menu->current_entry_offset - 5;
            if(menu->last_entry >= MENU_CACHE_COUNT) menu->last_entry = MENU_CACHE_COUNT - 1;
            insert_pos = 0;
        }
        for(int i = insert_pos, j = 5; i < MENU_CACHE_COUNT && j < tuple->length; ++i) {
            uint8_t len = tuple->value->data[j++];
            if(skipping) {
                --skipping;
            } else {
                memset(menu->menu_entries[i], 0, MENU_ENTRY_LENGTH);
                memcpy(menu->menu_entries[i], &tuple->value->data[j], len < MENU_ENTRY_LENGTH - 1 ? len : MENU_ENTRY_LENGTH - 1);
                if(i > menu->last_entry) {
                    menu->last_entry = i;
                }
                if(i == MENU_CACHE_COUNT - 1) {
                    // Shift back if we're out of space, unless we're too far ahead already.
                    if(menu->current_selection - menu->current_entry_offset <= 5) break;
                    memmove(&menu->menu_entries[0], &menu->menu_entries[5], (MENU_CACHE_COUNT - 5) * MENU_ENTRY_LENGTH);
                    menu->last_entry -= 5;
                    menu->current_entry_offset += 5;
                    i -= 5;
                }
            }
            j += len;
        }
        menu_layer_reload_data(&menu->layer);
    }
}

// Window callbacks
static void window_disappeared(Window* window) {
    if(menu_stack_pointer > -1)
        --menu_stack_pointer;
}

// Menu callbacks
static uint16_t get_num_rows(MenuLayer* layer, uint16_t section_index, void* context) {
    return ((LibraryMenu*)context)->total_entry_count;
}

static void draw_row(GContext *ctx, const Layer *cell_layer, MenuIndex *cell_index, void *callback_context) {
    LibraryMenu *menu = (LibraryMenu*)callback_context;
    int16_t pos = cell_index->row - menu->current_entry_offset;
    if(pos >= MENU_CACHE_COUNT || pos < 0) return;
    menu_cell_basic_draw(ctx, cell_layer, menu->menu_entries[pos], NULL, NULL);
}

static void selection_changed(struct MenuLayer *menu_layer, MenuIndex new_index, MenuIndex old_index, void *callback_context) {
    LibraryMenu *menu = (LibraryMenu*)callback_context;
    int16_t pos = new_index.row - menu->current_entry_offset;
    menu->current_selection = new_index.row;
    bool down = new_index.row > old_index.row;
    
    if(down) {
        if(pos >= menu->last_entry - 4) {
            if(menu->current_entry_offset + menu->last_entry >= menu->total_entry_count) {
                return;
            }
            send_library_request(menu->grouping, menu->last_entry + menu->current_entry_offset);
        }
    } else {
        if(pos < 5 && menu->current_entry_offset > 0) {
            send_library_request(menu->grouping, menu->current_entry_offset > 5 ? menu->current_entry_offset - 5 : 0);
        }
    }
}

static void select_click(struct MenuLayer *menu_layer, MenuIndex *cell_index, void *callback_context) {
    LibraryMenu *menu = (LibraryMenu*)callback_context;
    if(menu->grouping == MPMediaGroupingTitle) {
        play_track(cell_index->row);
    } else {
        if(menu_stack_pointer + 1 >= MENU_STACK_DEPTH) return;
        if(menu->grouping != MPMediaGroupingAlbum && menu->grouping != MPMediaGroupingPlaylist) {
            if(menu->grouping == MPMediaGroupingGenre) {
                display_library_view(MPMediaGroupingArtist);
            } else {
                display_library_view(MPMediaGroupingAlbum);
            }
        } else {
            display_library_view(MPMediaGroupingTitle);
        }
    }
}
