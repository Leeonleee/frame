#include "theme.h"

GRect theme_content_bounds(Window *window) {
  GRect bounds = layer_get_bounds(window_get_root_layer(window));
  bounds.origin.y += STATUS_BAR_LAYER_HEIGHT;
  bounds.size.h -= STATUS_BAR_LAYER_HEIGHT;
  return bounds;
}

StatusBarLayer *theme_status_bar_create(Window *window) {
  StatusBarLayer *status_bar = status_bar_layer_create();
  status_bar_layer_set_colors(status_bar, THEME_CHROME_BG, THEME_CHROME_TEXT);
  status_bar_layer_set_separator_mode(status_bar,
                                      StatusBarLayerSeparatorModeNone);
  layer_add_child(window_get_root_layer(window),
                  status_bar_layer_get_layer(status_bar));
  return status_bar;
}

void theme_menu_colors(MenuLayer *menu_layer) {
  menu_layer_set_normal_colors(menu_layer, THEME_BG, THEME_TEXT);
  menu_layer_set_highlight_colors(menu_layer, THEME_SELECT_BG,
                                  THEME_SELECT_TEXT);
}

void theme_draw_menu_header(GContext *ctx, const Layer *cell_layer,
                            const char *title) {
  GRect bounds = layer_get_bounds(cell_layer);
  graphics_context_set_fill_color(ctx, THEME_CHROME_BG);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
  graphics_context_set_text_color(ctx, THEME_CHROME_TEXT);
  // Centered on round watches (where left-aligned text is clipped by the
  // circular mask), left-aligned on rectangular ones.
  int16_t inset = PBL_IF_ROUND_ELSE(0, 5);
  graphics_draw_text(ctx, title,
                     fonts_get_system_font(FONT_KEY_GOTHIC_14_BOLD),
                     GRect(bounds.origin.x + inset, bounds.origin.y - 2,
                           bounds.size.w - 2 * inset, bounds.size.h),
                     GTextOverflowModeTrailingEllipsis,
                     PBL_IF_ROUND_ELSE(GTextAlignmentCenter, GTextAlignmentLeft),
                     NULL);
}
