#include <pebble.h>
#include "roll_list.h"

static void prv_init(void) {
  roll_list_window_push();
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
