#pragma once

#include <pebble.h>

// Screen 3 - a single roll's frames. A MenuLayer titled with the stock name,
// with "+ Add Frame" at the top and frames listed newest first below it.
void roll_window_push(uint8_t roll_index);
