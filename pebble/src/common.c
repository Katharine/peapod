#include "common.h"
#include "pebble_os.h"

static uint32_t s_sequence_number = 0xFFFFFFFE;

AppMessageResult ipod_message_out_get(DictionaryIterator **iter_out) {
    AppMessageResult result = app_message_out_get(iter_out);
    if(result != APP_MSG_OK) return result;
    dict_write_int32(*iter_out, IPOD_SEQUENCE_NUMBER_KEY, ++s_sequence_number);
    if(s_sequence_number == 0xFFFFFFFF) {
        s_sequence_number = 1;
    }
    return APP_MSG_OK;
}

void reset_sequence_number() {
    DictionaryIterator *iter = NULL;
    app_message_out_get(&iter);
    if(!iter) return;
    dict_write_int32(iter, IPOD_SEQUENCE_NUMBER_KEY, 0xFFFFFFFF);
    app_message_out_send();
    app_message_out_release();
}
