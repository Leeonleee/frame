#include <pebble.h>
#include "data.h"
#include "roll_list.h"
#include "settings.h"

// Settings sent from the phone config screen arrive here.
static void prv_inbox_received(DictionaryIterator *iter, void *context) {
  bool changed = false;

  Tuple *vibrate = dict_find(iter, MESSAGE_KEY_VIBRATE_ON_SAVE);
  if (vibrate) {
    settings_set_vibrate_on_save(vibrate->value->int32 != 0);
  }

  Tuple *date_format = dict_find(iter, MESSAGE_KEY_DATE_FORMAT);
  if (date_format) {
    settings_set_date_format(date_format->value->int32);
    changed = true;  // the Roll List dates need redrawing
  }

  if (changed) {
    roll_list_refresh();
  }
}

static void prv_init(void) {
  settings_load();
  data_load();
  roll_list_window_push();

  app_message_register_inbox_received(prv_inbox_received);
  app_message_open(128, 128);
}

static void prv_deinit(void) {
  // Windows own themselves; nothing global to tear down yet.
}

int main(void) {
  prv_init();
  APP_LOG(APP_LOG_LEVEL_DEBUG, "frame started");
  app_event_loop();
  prv_deinit();
}
