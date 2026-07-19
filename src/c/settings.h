#pragma once

#include <pebble.h>

// App preferences. These are configured from the settings screen in the Pebble
// phone app (see src/pkjs/index.js) and delivered to the watch over AppMessage;
// this module just stores the received values and persists them so they survive
// the app closing. Stored independently of the roll data.

// Load settings from persistent storage (or defaults). Call once at launch.
void settings_load(void);

// Vibrate briefly when a frame is saved.
bool settings_vibrate_on_save(void);
void settings_set_vibrate_on_save(bool enabled);

// How dates are formatted on the Roll List.
void settings_set_date_format(uint8_t index);
const char *settings_date_strftime(void);  // strftime pattern for the current style
