#pragma once

#include <pebble.h>

typedef void (*ConfirmCallback)(void *context);

// Screen 5 - a reusable yes/no dialog for irreversible actions. Shows `message`
// with SELECT = confirm and BACK = cancel. On confirm it runs `callback` with
// `context`, then closes; on cancel it just closes.
void confirm_window_push(const char *message, ConfirmCallback callback,
                         void *context);
