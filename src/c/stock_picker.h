#pragma once

#include <pebble.h>

// Screen 2 - the film-stock chooser for a new roll. A MenuLayer grouped into a
// section per brand. Selecting a stock creates the roll and returns to the
// Roll List. Back cancels without creating anything.
void stock_picker_window_push(void);
