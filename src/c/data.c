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

// --- In-memory roll store ---------------------------------------------------
static Roll s_rolls[MAX_ROLLS];
static uint8_t s_roll_count;

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
  return s_roll_count++;
}
