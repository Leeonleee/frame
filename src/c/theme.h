#pragma once

#include <pebble.h>

// Central palette for the app. On the colour Pebbles (emery, gabbro, ...) this
// gives a warm, filmic look; on the 1-bit platforms (aplite, diorite) the
// PBL_COLOR fallbacks collapse to the classic black-on-white style.
//
//   CHROME  - the top status bar and section-header bands (warm brown)
//   SELECT  - the highlighted menu row / active editor field (amber)
//   DANGER  - destructive actions (delete)
//   ACCENT  - small pops of colour (hints, markers)

#ifdef PBL_COLOR
  #define THEME_CHROME_BG    GColorTiffanyBlue
  #define THEME_CHROME_TEXT  GColorWhite
  #define THEME_SELECT_BG    GColorOrange
  #define THEME_SELECT_TEXT  GColorBlack
  #define THEME_ACCENT       GColorTiffanyBlue
  #define THEME_DANGER_BG    GColorRed
  #define THEME_DANGER_TEXT  GColorWhite
#else
  #define THEME_CHROME_BG    GColorBlack
  #define THEME_CHROME_TEXT  GColorWhite
  #define THEME_SELECT_BG    GColorBlack
  #define THEME_SELECT_TEXT  GColorWhite
  #define THEME_ACCENT       GColorBlack
  #define THEME_DANGER_BG    GColorBlack
  #define THEME_DANGER_TEXT  GColorWhite
#endif

#define THEME_BG    GColorWhite
#define THEME_TEXT  GColorBlack

// Content bounds for a window with the themed status bar at the top: the full
// window minus the status-bar strip.
GRect theme_content_bounds(Window *window);

// Create a themed status bar (warm background, shows the time), add it as a
// child of the window's root layer, and return it. Destroy it in unload.
StatusBarLayer *theme_status_bar_create(Window *window);

// Apply the shared normal/highlight row colours to a MenuLayer.
void theme_menu_colors(MenuLayer *menu_layer);

// Draw a section header as a solid colour band with white text. Use as the
// MenuLayer draw_header callback body.
void theme_draw_menu_header(GContext *ctx, const Layer *cell_layer,
                            const char *title);
