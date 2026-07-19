#include "confirm.h"

static Window *s_window;
static Layer *s_layer;

// Only one confirm dialog is ever open at a time, so its state can be static.
static char s_message[48];
static ConfirmCallback s_callback;
static void *s_context;

static void prv_update_proc(Layer *layer, GContext *ctx) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_text_color(ctx, GColorBlack);

  // The question, centered in the upper half.
  graphics_draw_text(ctx, s_message,
                     fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD),
                     GRect(8, 34, bounds.size.w - 16, 80),
                     GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);

  // Button hints along the bottom.
  graphics_draw_text(ctx, "SELECT = Delete\nBACK = Cancel",
                     fonts_get_system_font(FONT_KEY_GOTHIC_18),
                     GRect(0, bounds.size.h - 52, bounds.size.w, 48),
                     GTextOverflowModeWordWrap, GTextAlignmentCenter, NULL);
}

static void prv_select_click(ClickRecognizerRef recognizer, void *context) {
  if (s_callback) {
    s_callback(s_context);
  }
  window_stack_pop(true);  // return to the list, which reloads on appear
}

static void prv_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, prv_select_click);
  // BACK keeps its default behavior: pop the dialog and cancel.
}

static void prv_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  s_layer = layer_create(layer_get_bounds(window_layer));
  layer_set_update_proc(s_layer, prv_update_proc);
  layer_add_child(window_layer, s_layer);
}

static void prv_window_unload(Window *window) {
  layer_destroy(s_layer);
  s_layer = NULL;
  window_destroy(s_window);
  s_window = NULL;
}

void confirm_window_push(const char *message, ConfirmCallback callback,
                         void *context) {
  strncpy(s_message, message, sizeof(s_message) - 1);
  s_message[sizeof(s_message) - 1] = '\0';
  s_callback = callback;
  s_context = context;

  s_window = window_create();
  window_set_background_color(s_window, GColorWhite);
  window_set_click_config_provider(s_window, prv_click_config_provider);
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = prv_window_load,
    .unload = prv_window_unload,
  });
  window_stack_push(s_window, true);
}
