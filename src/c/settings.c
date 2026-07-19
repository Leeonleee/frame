#include "settings.h"

// Settings live at a high key so they never collide with the roll data keys
// (0..61) owned by data.c, and are untouched by a roll-data reset.
#define PERSIST_KEY_SETTINGS 100

// strftime patterns offered by the phone config's date-format dropdown. The
// order must match the <option> values in src/pkjs/index.js.
static const char *const s_date_formats[] = {
  "%d %b",     // 19 Jul
  "%b %d",     // Jul 19
  "%d/%m/%y",  // 19/07/26
};

typedef struct {
  uint8_t vibrate_on_save;
  uint8_t date_format;  // index into s_date_formats
} Settings;

static Settings s_settings;

static void prv_save(void) {
  persist_write_data(PERSIST_KEY_SETTINGS, &s_settings, sizeof(s_settings));
}

void settings_load(void) {
  if (persist_exists(PERSIST_KEY_SETTINGS)) {
    persist_read_data(PERSIST_KEY_SETTINGS, &s_settings, sizeof(s_settings));
  } else {
    s_settings.vibrate_on_save = 1;
    s_settings.date_format = 0;
  }
  if (s_settings.date_format >= ARRAY_LENGTH(s_date_formats)) {
    s_settings.date_format = 0;
  }
}

bool settings_vibrate_on_save(void) {
  return s_settings.vibrate_on_save;
}

void settings_set_vibrate_on_save(bool enabled) {
  s_settings.vibrate_on_save = enabled ? 1 : 0;
  prv_save();
}

void settings_set_date_format(uint8_t index) {
  if (index >= ARRAY_LENGTH(s_date_formats)) {
    return;
  }
  s_settings.date_format = index;
  prv_save();
}

const char *settings_date_strftime(void) {
  return s_date_formats[s_settings.date_format];
}
