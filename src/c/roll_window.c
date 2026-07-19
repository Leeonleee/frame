#include "roll_window.h"
#include "data.h"
#include "frame_editor.h"
#include "confirm.h"

static Window *s_window;
static MenuLayer *s_menu_layer;
static uint8_t s_roll_index;

// Row 0 is "+ Add Frame". Rows 1..frame_count are frames, newest first.
static bool prv_is_add_row(MenuIndex *index) {
  return index->row == 0;
}

// Map a frame display row (1-based, newest first) to a frame storage index.
static uint8_t prv_frame_storage_index(uint16_t row) {
  Roll *roll = data_roll_get(s_roll_index);
  return roll->frame_count - row;
}

static uint16_t prv_get_num_rows(MenuLayer *menu_layer, uint16_t section_index,
                                 void *context) {
  Roll *roll = data_roll_get(s_roll_index);
  return (roll ? roll->frame_count : 0) + 1;  // +1 for "+ Add Frame"
}

static int16_t prv_get_header_height(MenuLayer *menu_layer,
                                     uint16_t section_index, void *context) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void prv_draw_header(GContext *ctx, const Layer *cell_layer,
                            uint16_t section_index, void *context) {
  Roll *roll = data_roll_get(s_roll_index);
  menu_cell_basic_header_draw(ctx, cell_layer,
                              roll ? data_stock_name(roll->stock_id) : "Roll");
}

static void prv_draw_row(GContext *ctx, const Layer *cell_layer,
                         MenuIndex *cell_index, void *context) {
  if (prv_is_add_row(cell_index)) {
    menu_cell_basic_draw(ctx, cell_layer, "+ Add Frame", NULL, NULL);
    return;
  }

  uint8_t frame_idx = prv_frame_storage_index(cell_index->row);
  Roll *roll = data_roll_get(s_roll_index);
  Frame *frame = &roll->frames[frame_idx];

  static char s_title[16];
  static char s_subtitle[32];
  snprintf(s_title, sizeof(s_title), "Frame %d", frame_idx + 1);
  snprintf(s_subtitle, sizeof(s_subtitle), "%s · %s · ISO %s",
           data_shutter_str(frame->shutter_idx),
           data_aperture_str(frame->aperture_idx),
           data_iso_str(frame->iso_idx));

  menu_cell_basic_draw(ctx, cell_layer, s_title, s_subtitle, NULL);
}

static void prv_select_click(MenuLayer *menu_layer, MenuIndex *cell_index,
                             void *context) {
  if (prv_is_add_row(cell_index)) {
    frame_editor_push(s_roll_index, -1);  // new frame
    return;
  }
  frame_editor_push(s_roll_index, prv_frame_storage_index(cell_index->row));
}

static void prv_confirm_delete_frame(void *context) {
  data_frame_delete(s_roll_index, (uint8_t)(uintptr_t)context);
}

static void prv_select_long_click(MenuLayer *menu_layer, MenuIndex *cell_index,
                                  void *context) {
  if (prv_is_add_row(cell_index)) {
    return;
  }
  uint8_t frame_index = prv_frame_storage_index(cell_index->row);
  confirm_window_push("Delete this frame?", prv_confirm_delete_frame,
                      (void *)(uintptr_t)frame_index);
}

static void prv_window_appear(Window *window) {
  if (s_menu_layer) {
    menu_layer_reload_data(s_menu_layer);
  }
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = prv_get_num_rows,
    .get_header_height = prv_get_header_height,
    .draw_header = prv_draw_header,
    .draw_row = prv_draw_row,
    .select_click = prv_select_click,
    .select_long_click = prv_select_long_click,
  });
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));
}

static void prv_window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  s_menu_layer = NULL;
  window_destroy(s_window);
  s_window = NULL;
}

void roll_window_push(uint8_t roll_index) {
  s_roll_index = roll_index;
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .appear = prv_window_appear,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);
}
