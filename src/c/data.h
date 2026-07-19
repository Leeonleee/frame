#pragma once

#include <pebble.h>

// Frame - a single photo's exposure settings.
// Each setting is stored as a small index into the hardcoded tables below,
// so a whole roll stays tiny (see data.c for the tables).
#define MAX_FRAMES 36
#define MAX_ROLLS 30

// Indices used as sensible defaults for the very first frame of a roll.
// f/8 and 1/125 are a middle-of-the-road daylight starting point.
#define APERTURE_DEFAULT_IDX 5  // f/8
#define SHUTTER_DEFAULT_IDX 8   // 1/125

// Packed so a full roll's frames fit in one persist value (see data.c). Each
// setting is an index into the tables below; `created` is the time_t captured
// when the frame was logged, kept for future EXIF export.
typedef struct __attribute__((__packed__)) {
  uint8_t shutter_idx;
  uint8_t aperture_idx;
  uint8_t iso_idx;
  int32_t created;
} Frame;

typedef struct {
  uint8_t stock_id;             // index into the film-stock table
  uint8_t frame_count;          // number of frames used (0..MAX_FRAMES)
  int32_t created;              // time_t captured when the roll was made
  Frame frames[MAX_FRAMES];
} Roll;

// --- Hardcoded value tables -------------------------------------------------
// Each table is a fixed list rendered from a display-string helper. Values are
// referenced everywhere by their index into these arrays.

uint8_t data_aperture_count(void);
const char *data_aperture_str(uint8_t idx);

uint8_t data_shutter_count(void);
const char *data_shutter_str(uint8_t idx);

uint8_t data_iso_count(void);
const char *data_iso_str(uint8_t idx);

// --- Film stocks ------------------------------------------------------------

typedef enum {
  BRAND_KODAK,
  BRAND_ILFORD,
  BRAND_FUJI,
  BRAND_CINESTILL,
  BRAND_COUNT,
} Brand;

const char *data_brand_name(Brand brand);

uint8_t data_stock_count(void);
const char *data_stock_name(uint8_t stock_id);
Brand data_stock_brand(uint8_t stock_id);
uint8_t data_stock_iso_idx(uint8_t stock_id);  // box speed as an ISO index

// Stocks grouped by brand, for the sectioned Stock Picker. Stocks are stored
// contiguously by brand, so these iterate the table in order.
uint8_t data_brand_stock_count(Brand brand);
int data_brand_stock_id(Brand brand, uint8_t row);  // -1 if out of range

// --- Roll store -------------------------------------------------------------
// Rolls are stored oldest-first in RAM and mirrored to persistent storage, so
// they survive the app closing. The UI presents them newest-first.

// Load all rolls from persistent storage. Call once at app launch.
void data_load(void);

uint8_t data_roll_count(void);
Roll *data_roll_get(uint8_t index);  // index is storage order (0 = oldest)

// Create a new roll for the given stock, stamped with the current time.
// Returns the new roll's storage index, or -1 if the store is full.
int data_roll_create(uint8_t stock_id);

// Append a frame to a roll. Returns the new frame's storage index (0-based),
// or -1 if the roll is full (MAX_FRAMES) or the roll index is invalid.
int data_frame_add(uint8_t roll_index, Frame frame);

// Overwrite an existing frame in place.
void data_frame_update(uint8_t roll_index, uint8_t frame_index, Frame frame);

// Build the starting values for a new frame: the roll's last frame if it has
// one (carry-over), otherwise f/8, 1/125, and the stock's box ISO.
Frame data_frame_default(uint8_t roll_index);

// Delete a whole roll, shifting later rolls down to fill the gap.
void data_roll_delete(uint8_t roll_index);

// Delete a single frame from a roll, shifting later frames down.
void data_frame_delete(uint8_t roll_index, uint8_t frame_index);
