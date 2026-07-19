#include "roll_list.h"
#include "data.h"
#include "stock_picker.h"

static Window *s_window;
static MenuLayer *s_menu_layer;

// The menu is a single section: one row per roll (newest first), then a final
// "+ New Roll" row. Row index `count` is always the "+ New Roll" row.
static bool prv_is_new_roll_row(MenuIndex *index) {
  return index->row >= data_roll_count();
}

// Map a display row (newest first) to a storage index (oldest first).
static uint8_t prv_storage_index(uint16_t row) {
  return data_roll_count() - 1 - row;
}

static uint16_t prv_get_num_rows(MenuLayer *menu_layer, uint16_t section_index,
                                 void *context) {
  return data_roll_count() + 1;  // +1 for the "+ New Roll" row
}

static int16_t prv_get_header_height(MenuLayer *menu_layer,
                                     uint16_t section_index, void *context) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void prv_draw_header(GContext *ctx, const Layer *cell_layer,
                            uint16_t section_index, void *context) {
  const char *title = (data_roll_count() == 0) ? "No rolls yet" : "Rolls";
  menu_cell_basic_header_draw(ctx, cell_layer, title);
}

static void prv_draw_row(GContext *ctx, const Layer *cell_layer,
                         MenuIndex *cell_index, void *context) {
  if (prv_is_new_roll_row(cell_index)) {
    menu_cell_basic_draw(ctx, cell_layer, "+ New Roll", NULL, NULL);
    return;
  }

  Roll *roll = data_roll_get(prv_storage_index(cell_index->row));
  if (!roll) {
    return;
  }

  static char s_subtitle[32];
  char date_buf[16];
  time_t created = roll->created;
  struct tm *tm = localtime(&created);
  strftime(date_buf, sizeof(date_buf), "%d %b", tm);
  snprintf(s_subtitle, sizeof(s_subtitle), "%s · %d frames", date_buf,
           roll->frame_count);

  menu_cell_basic_draw(ctx, cell_layer, data_stock_name(roll->stock_id),
                       s_subtitle, NULL);
}

static void prv_select_click(MenuLayer *menu_layer, MenuIndex *cell_index,
                             void *context) {
  if (prv_is_new_roll_row(cell_index)) {
    stock_picker_window_push();
    return;
  }
  // Part 2/3 opens the Roll screen for this roll.
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Roll %d selected",
          prv_storage_index(cell_index->row));
}

static void prv_select_long_click(MenuLayer *menu_layer, MenuIndex *cell_index,
                                  void *context) {
  if (prv_is_new_roll_row(cell_index)) {
    return;
  }
  // Part 4 wires this to the delete-confirm dialog.
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Delete roll %d requested",
          prv_storage_index(cell_index->row));
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

static void prv_window_appear(Window *window) {
  // Reload when returning from the Stock Picker (or, later, a Roll screen) so
  // newly created or changed rolls show up.
  if (s_menu_layer) {
    menu_layer_reload_data(s_menu_layer);
  }
}

static void prv_window_unload(Window *window) {
  menu_layer_destroy(s_menu_layer);
  s_menu_layer = NULL;
}

void roll_list_window_push(void) {
  if (!s_window) {
    s_window = window_create();
    window_set_window_handlers(s_window, (WindowHandlers) {
      .load = prv_window_load,
      .appear = prv_window_appear,
      .unload = prv_window_unload,
    });
  }
  window_stack_push(s_window, true);
}
