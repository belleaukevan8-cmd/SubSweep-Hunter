#pragma once

#include <gui/view.h>
#include "app_state.h"

typedef struct UiResult UiResult;
typedef void (*UiResultCallback)(void* context);

UiResult* ui_result_alloc(void);
void ui_result_free(UiResult* ui);

View* ui_result_get_view(UiResult* ui);

void ui_result_set_state(UiResult* ui, AppState* state);
void ui_result_set_back_callback(UiResult* ui, UiResultCallback callback, void* context);
void ui_result_set_save_callback(UiResult* ui, UiResultCallback callback, void* context);

void ui_result_show_stats(UiResult* ui, AppState* state);
