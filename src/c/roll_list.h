#pragma once

#include <pebble.h>

// Screen 1 - the app's home screen. Lists every roll (newest first) with a
// trailing "+ New Roll" row. Pushed at launch.
void roll_list_window_push(void);

// Redraw the list, e.g. after a settings change alters the date format.
void roll_list_refresh(void);
