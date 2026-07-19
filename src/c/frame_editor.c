#include "frame_editor.h"
#include "data.h"
#include "theme.h"
#include "settings.h"

typedef enum {
  FIELD_SHUTTER,
  FIELD_APERTURE,
  FIELD_ISO,
  FIELD_COUNT,
} Field;

static Window *s_window;
static Layer *s_field_layer;
static StatusBarLayer *s_status_bar;

// Working state for the editor session.
static uint8_t s_roll_index;
static int s_frame_index;   // < 0 means a new frame
static Frame s_frame;       // the values being edited
static Field s_field;       // which field is highlighted

// --- Field access -----------------------------------------------------------
static uint8_t *prv_field_value(Field field) {
  switch (field) {
    case FIELD_SHUTTER:  return &s_frame.shutter_idx;
    case FIELD_APERTURE: return &s_frame.aperture_idx;
    case FIELD_ISO:      return &s_frame.iso_idx;
    default:             return &s_frame.shutter_idx;
  }
}

static uint8_t prv_field_max(Field field) {
  switch (field) {
    case FIELD_SHUTTER:  return data_shutter_count() - 1;
    case FIELD_APERTURE: return data_aperture_count() - 1;
    case FIELD_ISO:      return data_iso_count() - 1;
    default:             return 0;
  }
}

static const char *prv_field_label(Field field) {
  switch (field) {
    case FIELD_SHUTTER:  return "SHUTTER";
    case FIELD_APERTURE: return "APERTURE";
    case FIELD_ISO:      return "ISO";
    default:             return "";
  }
}

static const char *prv_field_display(Field field) {
  switch (field) {
    case FIELD_SHUTTER:  return data_shutter_str(s_frame.shutter_idx);
    case FIELD_APERTURE: return data_aperture_str(s_frame.aperture_idx);
    case FIELD_ISO:      return data_iso_str(s_frame.iso_idx);
    default:             return "";
  }
}

// --- Drawing ----------------------------------------------------------------
static void prv_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);

  const int16_t row_h = 44;
  const int16_t total = row_h * FIELD_COUNT;
  int16_t start_y = (bounds.size.h - total) / 2 - 8;

  for (int i = 0; i < FIELD_COUNT; i++) {
    GRect row = GRect(6, start_y + i * row_h, bounds.size.w - 12, row_h - 6);
    bool selected = (i == s_field);

    GColor text_color = THEME_TEXT;
    if (selected) {
      graphics_context_set_fill_color(ctx, THEME_SELECT_BG);
      graphics_fill_rect(ctx, row, 6, GCornersAll);
      text_color = THEME_SELECT_TEXT;
    }
    graphics_context_set_text_color(ctx, text_color);

    GRect text_rect = GRect(row.origin.x + 8,
                            row.origin.y + (row.size.h - 26) / 2,
                            row.size.w - 16, 26);
    graphics_draw_text(ctx, prv_field_label(i),
                       fonts_get_system_font(FONT_KEY_GOTHIC_18),
                       text_rect, GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentLeft, NULL);
    graphics_draw_text(ctx, prv_field_display(i),
                       fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                       GRect(text_rect.origin.x, text_rect.origin.y - 4,
                             text_rect.size.w, text_rect.size.h + 4),
                       GTextOverflowModeTrailingEllipsis,
                       GTextAlignmentRight, NULL);
  }

  // Discoverability hint for the save gesture.
  graphics_context_set_text_color(ctx, THEME_ACCENT);
  graphics_draw_text(ctx, "Hold SELECT to save",
                     fonts_get_system_font(FONT_KEY_GOTHIC_14),
                     GRect(0, bounds.size.h - 22, bounds.size.w, 18),
                     GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter,
                     NULL);
}

// --- Interaction ------------------------------------------------------------
static void prv_up_click(ClickRecognizerRef recognizer, void *context) {
  uint8_t *value = prv_field_value(s_field);
  if (*value < prv_field_max(s_field)) {
    (*value)++;
    layer_mark_dirty(s_field_layer);
  }
}

static void prv_down_click(ClickRecognizerRef recognizer, void *context) {
  uint8_t *value = prv_field_value(s_field);
  if (*value > 0) {
    (*value)--;
    layer_mark_dirty(s_field_layer);
  }
}

static void prv_select_click(ClickRecognizerRef recognizer, void *context) {
  s_field = (s_field + 1) % FIELD_COUNT;  // cycle highlight
  layer_mark_dirty(s_field_layer);
}

static void prv_select_long_click(ClickRecognizerRef recognizer,
                                  void *context) {
  if (s_frame_index < 0) {
    data_frame_add(s_roll_index, s_frame);
  } else {
    data_frame_update(s_roll_index, s_frame_index, s_frame);
  }
  if (settings_vibrate_on_save()) {
    vibes_short_pulse();
  }
  window_stack_pop(true);  // back to the Roll screen, which reloads
}

static void prv_click_config_provider(void *context) {
  window_single_repeating_click_subscribe(BUTTON_ID_UP, 120, prv_up_click);
  window_single_repeating_click_subscribe(BUTTON_ID_DOWN, 120, prv_down_click);
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click);
  window_long_click_subscribe(BUTTON_ID_SELECT, 0, prv_select_long_click, NULL);
}

// --- Window lifecycle -------------------------------------------------------
static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);

  s_field_layer = layer_create(theme_content_bounds(window));
  layer_set_update_proc(s_field_layer, prv_update_proc);
  layer_add_child(window_layer, s_field_layer);

  s_status_bar = theme_status_bar_create(window);
}

static void prv_window_unload(Window *window) {
  status_bar_layer_destroy(s_status_bar);
  s_status_bar = NULL;
  layer_destroy(s_field_layer);
  s_field_layer = NULL;
  window_destroy(s_window);
  s_window = NULL;
}

void frame_editor_push(uint8_t roll_index, int frame_index) {
  s_roll_index = roll_index;
  s_frame_index = frame_index;
  s_field = FIELD_SHUTTER;

  if (frame_index < 0) {
    s_frame = data_frame_default(roll_index);
  } else {
    Roll *roll = data_roll_get(roll_index);
    s_frame = roll->frames[frame_index];
  }

  s_window = window_create();
  window_set_background_color(s_window, THEME_BG);
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);
}
