#include "stock_picker.h"
#include "data.h"
#include "roll_window.h"
#include "theme.h"

static Window *s_window;
static MenuLayer *s_menu_layer;
static StatusBarLayer *s_status_bar;

// The picker is an accordion: brands are shown collapsed, and selecting one
// expands its stocks inline. Only one brand is expanded at a time.
static int s_expanded_brand;  // -1 when all brands are collapsed

// A visible row is either a brand header or one stock within the expanded brand.
typedef struct {
  bool is_brand;
  int brand;
  int stock_id;  // valid when !is_brand
} RowInfo;

// Resolve a flat row index into what it represents, walking the brands in order
// and expanding the open brand's stocks in place.
static RowInfo prv_row_info(uint16_t row) {
  uint16_t r = 0;
  for (int b = 0; b < BRAND_COUNT; b++) {
    if (r == row) {
      return (RowInfo){ .is_brand = true, .brand = b, .stock_id = -1 };
    }
    r++;
    if (b == s_expanded_brand) {
      uint8_t count = data_brand_stock_count(b);
      for (uint8_t i = 0; i < count; i++) {
        if (r == row) {
          return (RowInfo){ .is_brand = false, .brand = b,
                            .stock_id = data_brand_stock_id(b, i) };
        }
        r++;
      }
    }
  }
  return (RowInfo){ .is_brand = false, .brand = -1, .stock_id = -1 };
}

static uint16_t prv_get_num_rows(MenuLayer *menu_layer, uint16_t section_index,
                                 void *context) {
  uint16_t rows = BRAND_COUNT;
  if (s_expanded_brand >= 0) {
    rows += data_brand_stock_count(s_expanded_brand);
  }
  return rows;
}

static int16_t prv_get_header_height(MenuLayer *menu_layer,
                                     uint16_t section_index, void *context) {
  return MENU_CELL_BASIC_HEADER_HEIGHT;
}

static void prv_draw_header(GContext *ctx, const Layer *cell_layer,
                            uint16_t section_index, void *context) {
  theme_draw_menu_header(ctx, cell_layer, "Film stock");
}

static void prv_draw_row(GContext *ctx, const Layer *cell_layer,
                         MenuIndex *cell_index, void *context) {
  RowInfo info = prv_row_info(cell_index->row);

  if (info.is_brand) {
    static char s_count[16];
    snprintf(s_count, sizeof(s_count), "%d films",
             data_brand_stock_count(info.brand));
    menu_cell_basic_draw(ctx, cell_layer, data_brand_name(info.brand), s_count,
                         NULL);

    // A +/- marker on the right shows the collapsed/expanded state. It reuses
    // the text colour MenuLayer already set for this (possibly highlighted) row.
    GRect bounds = layer_get_bounds(cell_layer);
    const char *marker = (info.brand == s_expanded_brand) ? "-" : "+";
    graphics_draw_text(ctx, marker,
                       fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD),
                       GRect(bounds.size.w - 30, (bounds.size.h - 32) / 2, 22,
                             32),
                       GTextOverflowModeTrailingEllipsis, GTextAlignmentCenter,
                       NULL);
    return;
  }

  if (info.stock_id < 0) {
    return;
  }
  static char s_subtitle[16];
  snprintf(s_subtitle, sizeof(s_subtitle), "ISO %s",
           data_iso_str(data_stock_iso_idx(info.stock_id)));
  menu_cell_basic_draw(ctx, cell_layer, data_stock_name(info.stock_id),
                       s_subtitle, NULL);
}

static void prv_select_click(MenuLayer *menu_layer, MenuIndex *cell_index,
                             void *context) {
  RowInfo info = prv_row_info(cell_index->row);

  if (info.is_brand) {
    // Toggle this brand; collapsing happens automatically since only one brand
    // is ever expanded.
    s_expanded_brand = (s_expanded_brand == info.brand) ? -1 : info.brand;
    menu_layer_reload_data(s_menu_layer);
    menu_layer_set_selected_index(s_menu_layer, *cell_index,
                                  MenuRowAlignCenter, false);
    return;
  }

  if (info.stock_id < 0) {
    return;
  }
  int roll_index = data_roll_create(info.stock_id);
  if (roll_index < 0) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Roll store full; cannot create roll");
    return;
  }

  // Open the new roll's screen, then remove the picker from the stack so that
  // Back from the roll lands on the Roll List, not back on the picker.
  roll_window_push(roll_index);
  window_stack_remove(s_window, false);
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  window_set_background_color(window, THEME_BG);

  s_menu_layer = menu_layer_create(theme_content_bounds(window));
  menu_layer_set_callbacks(s_menu_layer, NULL, (MenuLayerCallbacks) {
    .get_num_rows = prv_get_num_rows,
    .get_header_height = prv_get_header_height,
    .draw_header = prv_draw_header,
    .draw_row = prv_draw_row,
    .select_click = prv_select_click,
  });
  theme_menu_colors(s_menu_layer);
  menu_layer_set_click_config_onto_window(s_menu_layer, window);
  layer_add_child(window_layer, menu_layer_get_layer(s_menu_layer));

  s_status_bar = theme_status_bar_create(window);
}

static void prv_window_unload(Window *window) {
  status_bar_layer_destroy(s_status_bar);
  s_status_bar = NULL;
  menu_layer_destroy(s_menu_layer);
  s_menu_layer = NULL;
  window_destroy(s_window);
  s_window = NULL;
}

void stock_picker_window_push(void) {
  s_expanded_brand = -1;  // start fully collapsed
  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);
}
