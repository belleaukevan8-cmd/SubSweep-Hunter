#pragma once

#include <gui/view.h>
#include "app_state.h"

typedef struct UiMain UiMain;
typedef void (*UiMainCallback)(void* context);

UiMain* ui_main_alloc(void);
void ui_main_free(UiMain* ui);

View* ui_main_get_view(UiMain* ui);

void ui_main_set_state(UiMain* ui, AppState* state);
void ui_main_set_toggle_callback(UiMain* ui, UiMainCallback callback, void* context);
void ui_main_set_quit_callback(UiMain* ui, UiMainCallback callback, void* context);
void ui_main_set_result_callback(UiMain* ui, UiMainCallback callback, void* context);
