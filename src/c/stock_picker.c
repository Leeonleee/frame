#include "stock_picker.h"
#include "data.h"
#include "roll_window.h"

static Window *s_window;
static MenuLayer *s_menu_layer;

// One menu section per brand; each row within a section is a stock.
static uint16_t prv_get_num_sections(MenuLayer *menu_layer, void *context) {
  return BRAND_COUNT;
}

static uint16_t prv_get_num_rows(MenuLayer *menu_layer, uint16_t section_index,
                                 void *context) {
  return data_brand_stock_count(section_index);
}

static int16_t prv_get_header_height(MenuLayer *menu_layer,
                                     uint16_t section_index, void *context) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void prv_draw_header(GContext *ctx, const Layer *cell_layer,
                            uint16_t section_index, void *context) {
  menu_cell_basic_header_draw(ctx, cell_layer, data_brand_name(section_index));
}

static void prv_draw_row(GContext *ctx, const Layer *cell_layer,
                         MenuIndex *cell_index, void *context) {
  int stock_id = data_brand_stock_id(cell_index->section, cell_index->row);
  if (stock_id < 0) {
    return;
  }

  static char s_subtitle[16];
  snprintf(s_subtitle, sizeof(s_subtitle), "ISO %s",
           data_iso_str(data_stock_iso_idx(stock_id)));

  menu_cell_basic_draw(ctx, cell_layer, data_stock_name(stock_id), s_subtitle,
                       NULL);
}

static void prv_select_click(MenuLayer *menu_layer, MenuIndex *cell_index,
                             void *context) {
  int stock_id = data_brand_stock_id(cell_index->section, cell_index->row);
  if (stock_id < 0) {
    return;
  }

  int roll_index = data_roll_create(stock_id);
  if (roll_index < 0) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Roll store full; cannot create roll");
    return;
  }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Created roll %d (%s)", roll_index,
          data_stock_name(stock_id));

  // Open the new roll's screen, then remove the picker from the stack so that
  // Back from the roll lands on the Roll List, not back on the picker.
  roll_window_push(roll_index);
  window_stack_remove(s_window, false);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_menu_layer = menu_layer_create(bounds);
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_sections = prv_get_num_sections,
    .get_num_rows = prv_get_num_rows,
    .get_header_height = prv_get_header_height,
    .draw_header = prv_draw_header,
    .draw_row = prv_draw_row,
    .select_click = prv_select_click,
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

void stock_picker_window_push(void) {
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);
}
