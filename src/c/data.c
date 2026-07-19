#include "data.h"

// --- Aperture (full stops) --------------------------------------------------
static const char *const s_apertures[] = {
  "f/1.4", "f/2", "f/2.8", "f/4", "f/5.6",
  "f/8", "f/11", "f/16", "f/22", "f/32",
};

uint8_t data_aperture_count(void) {
  return ARRAY_LENGTH(s_apertures);
}

const char *data_aperture_str(uint8_t idx) {
  if (idx >= ARRAY_LENGTH(s_apertures)) {
    return "?";
  }
  return s_apertures[idx];
}

// --- Shutter speed (full stops, slow -> fast, with bulb) --------------------
static const char *const s_shutters[] = {
  "B", "1\"", "1/2", "1/4", "1/8", "1/15", "1/30",
  "1/60", "1/125", "1/250", "1/500", "1/1000", "1/2000", "1/4000",
};

uint8_t data_shutter_count(void) {
  return ARRAY_LENGTH(s_shutters);
}

const char *data_shutter_str(uint8_t idx) {
  if (idx >= ARRAY_LENGTH(s_shutters)) {
    return "?";
  }
  return s_shutters[idx];
}

// --- ISO (box speeds + common push/pull ratings) ---------------------------
static const char *const s_isos[] = {
  "25", "50", "64", "100", "125", "160", "200", "320",
  "400", "500", "640", "800", "1600", "3200", "6400",
};

uint8_t data_iso_count(void) {
  return ARRAY_LENGTH(s_isos);
}

const char *data_iso_str(uint8_t idx) {
  if (idx >= ARRAY_LENGTH(s_isos)) {
    return "?";
  }
  return s_isos[idx];
}

// --- Film stocks ------------------------------------------------------------
// iso_idx is the box speed as an index into s_isos above.
typedef struct {
  const char *name;
  Brand brand;
  uint8_t iso_idx;
} Stock;

static const Stock s_stocks[] = {
  // Kodak
  { "Portra 160",   BRAND_KODAK,     5 },   // ISO 160
  { "Portra 400",   BRAND_KODAK,     8 },   // ISO 400
  { "Portra 800",   BRAND_KODAK,     11 },  // ISO 800
  { "Gold 200",     BRAND_KODAK,     6 },   // ISO 200
  { "UltraMax 400", BRAND_KODAK,     8 },   // ISO 400
  { "Ektar 100",    BRAND_KODAK,     3 },   // ISO 100
  { "Tri-X 400",    BRAND_KODAK,     8 },   // ISO 400
  { "T-Max 100",    BRAND_KODAK,     3 },   // ISO 100
  { "T-Max 400",    BRAND_KODAK,     8 },   // ISO 400
  // Ilford
  { "HP5 Plus 400", BRAND_ILFORD,    8 },   // ISO 400
  { "FP4 Plus 125", BRAND_ILFORD,    4 },   // ISO 125
  { "Delta 100",    BRAND_ILFORD,    3 },   // ISO 100
  { "Delta 400",    BRAND_ILFORD,    8 },   // ISO 400
  { "Delta 3200",   BRAND_ILFORD,    13 },  // ISO 3200
  { "XP2 Super 400",BRAND_ILFORD,    8 },   // ISO 400
  // Fujifilm
  { "Superia 400",  BRAND_FUJI,      8 },   // ISO 400
  { "Pro 400H",     BRAND_FUJI,      8 },   // ISO 400
  { "Velvia 50",    BRAND_FUJI,      1 },   // ISO 50
  { "Provia 100F",  BRAND_FUJI,      3 },   // ISO 100
  { "Acros 100 II", BRAND_FUJI,      3 },   // ISO 100
  // CineStill
  { "800T",         BRAND_CINESTILL, 11 },  // ISO 800
  { "50D",          BRAND_CINESTILL, 1 },   // ISO 50
};

static const char *const s_brand_names[BRAND_COUNT] = {
  "Kodak", "Ilford", "Fujifilm", "CineStill",
};

const char *data_brand_name(Brand brand) {
  if (brand >= BRAND_COUNT) {
    return "?";
  }
  return s_brand_names[brand];
}

uint8_t data_stock_count(void) {
  return ARRAY_LENGTH(s_stocks);
}

const char *data_stock_name(uint8_t stock_id) {
  if (stock_id >= ARRAY_LENGTH(s_stocks)) {
    return "?";
  }
  return s_stocks[stock_id].name;
}

Brand data_stock_brand(uint8_t stock_id) {
  if (stock_id >= ARRAY_LENGTH(s_stocks)) {
    return BRAND_KODAK;
  }
  return s_stocks[stock_id].brand;
}

uint8_t data_stock_iso_idx(uint8_t stock_id) {
  if (stock_id >= ARRAY_LENGTH(s_stocks)) {
    return 3;  // ISO 100 fallback
  }
  return s_stocks[stock_id].iso_idx;
}

uint8_t data_brand_stock_count(Brand brand) {
  uint8_t count = 0;
  for (uint8_t i = 0; i < ARRAY_LENGTH(s_stocks); i++) {
    if (s_stocks[i].brand == brand) {
      count++;
    }
  }
  return count;
}

int data_brand_stock_id(Brand brand, uint8_t row) {
  uint8_t seen = 0;
  for (uint8_t i = 0; i < ARRAY_LENGTH(s_stocks); i++) {
    if (s_stocks[i].brand == brand) {
      if (seen == row) {
        return i;
      }
      seen++;
    }
  }
  return -1;
}

// --- Roll store + persistence -----------------------------------------------
// Persist layout: key 0 holds the roll count; key (index + 1) holds each roll's
// struct blob. A Roll is well under PERSIST_DATA_MAX_LENGTH (256 bytes), so it
// fits in a single value.
#define PERSIST_KEY_COUNT 0

static Roll s_rolls[MAX_ROLLS];
static uint8_t s_roll_count;

static uint32_t prv_roll_key(uint8_t index) {
  return index + 1;
}

// Write one roll's blob to its key.
static void prv_save_roll(uint8_t index) {
  persist_write_data(prv_roll_key(index), &s_rolls[index], sizeof(Roll));
}

// Persist the count and every roll blob, and clear any stale trailing keys.
// Used when the set of rolls changes (create/delete) and storage indices move.
static void prv_save_all(void) {
  persist_write_int(PERSIST_KEY_COUNT, s_roll_count);
  for (uint8_t i = 0; i < s_roll_count; i++) {
    prv_save_roll(i);
  }
  for (uint8_t i = s_roll_count; i < MAX_ROLLS; i++) {
    persist_delete(prv_roll_key(i));
  }
}

void data_load(void) {
  int count = persist_exists(PERSIST_KEY_COUNT)
                  ? persist_read_int(PERSIST_KEY_COUNT)
                  : 0;
  if (count < 0) {
    count = 0;
  }
  if (count > MAX_ROLLS) {
    count = MAX_ROLLS;
  }
  for (int i = 0; i < count; i++) {
    if (persist_exists(prv_roll_key(i))) {
      persist_read_data(prv_roll_key(i), &s_rolls[i], sizeof(Roll));
    }
  }
  s_roll_count = count;
}

uint8_t data_roll_count(void) {
  return s_roll_count;
}

Roll *data_roll_get(uint8_t index) {
  if (index >= s_roll_count) {
    return NULL;
  }
  return &s_rolls[index];
}

int data_roll_create(uint8_t stock_id) {
  if (s_roll_count >= MAX_ROLLS) {
    return -1;
  }
  Roll *roll = &s_rolls[s_roll_count];
  roll->stock_id = stock_id;
  roll->frame_count = 0;
  roll->created = time(NULL);
  int index = s_roll_count++;
  prv_save_roll(index);
  persist_write_int(PERSIST_KEY_COUNT, s_roll_count);
  return index;
}

int data_frame_add(uint8_t roll_index, Frame frame) {
  Roll *roll = data_roll_get(roll_index);
  if (!roll || roll->frame_count >= MAX_FRAMES) {
    return -1;
  }
  uint8_t idx = roll->frame_count;
  roll->frames[idx] = frame;
  roll->frame_count++;
  prv_save_roll(roll_index);
  return idx;
}

void data_frame_update(uint8_t roll_index, uint8_t frame_index, Frame frame) {
  Roll *roll = data_roll_get(roll_index);
  if (!roll || frame_index >= roll->frame_count) {
    return;
  }
  roll->frames[frame_index] = frame;
  prv_save_roll(roll_index);
}

Frame data_frame_default(uint8_t roll_index) {
  Roll *roll = data_roll_get(roll_index);
  if (roll && roll->frame_count > 0) {
    return roll->frames[roll->frame_count - 1];  // carry over last frame
  }
  Frame frame = {
    .shutter_idx = SHUTTER_DEFAULT_IDX,
    .aperture_idx = APERTURE_DEFAULT_IDX,
    .iso_idx = roll ? data_stock_iso_idx(roll->stock_id) : 3,
  };
  return frame;
}

void data_roll_delete(uint8_t roll_index) {
  if (roll_index >= s_roll_count) {
    return;
  }
  for (uint8_t i = roll_index; i + 1 < s_roll_count; i++) {
    s_rolls[i] = s_rolls[i + 1];
  }
  s_roll_count--;
  prv_save_all();  // storage indices shifted, so rewrite everything
}

void data_frame_delete(uint8_t roll_index, uint8_t frame_index) {
  Roll *roll = data_roll_get(roll_index);
  if (!roll || frame_index >= roll->frame_count) {
    return;
  }
  for (uint8_t i = frame_index; i + 1 < roll->frame_count; i++) {
    roll->frames[i] = roll->frames[i + 1];
  }
  roll->frame_count--;
  prv_save_roll(roll_index);
}
