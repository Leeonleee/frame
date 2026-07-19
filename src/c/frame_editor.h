#pragma once

#include <pebble.h>

// Screen 4 - the combined exposure editor. Shows shutter / aperture / ISO with
// one field highlighted. Up/Down change the highlighted value, Select cycles
// the highlight, long-press Select saves, Back discards.
//
// frame_index < 0 creates a new frame (values carried over from the last one);
// otherwise it edits the existing frame at that storage index.
void frame_editor_push(uint8_t roll_index, int frame_index);
