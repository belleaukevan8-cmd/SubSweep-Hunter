#include "ui_result.h"
#include <gui/elements.h>
#include <furi.h>
#include <stdio.h>
#include <stdlib.h>

struct UiResult {
    View* view;
    AppState* state;
    UiResultCallback back_callback;
    void* back_context;
    UiResultCallback save_callback;
    void* save_context;
};

static void ui_result_draw_callback(Canvas* canvas, void* model) {
    UiResult* ui = (UiResult*)model;
    if(!ui || !ui->state) return;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "Sweep Results");

    canvas_set_font(canvas, FontSecondary);

    char buf[40];

    snprintf(buf, sizeof(buf), "Total signals: %lu", (unsigned long)ui->state->signal_count);
    canvas_draw_str(canvas, 2, 24, buf);

    snprintf(buf, sizeof(buf), "Best freq: %lu MHz",
        (unsigned long)(ui->state->best_frequency / 1000000));
    canvas_draw_str(canvas, 2, 34, buf);

    snprintf(buf, sizeof(buf), "Max RSSI: %d dBm", (int)ui->state->rssi_max);
    canvas_draw_str(canvas, 2, 44, buf);

    elements_button_left(canvas, "Back");
    elements_button_right(canvas, "Save");
}

static bool ui_result_input_callback(InputEvent* event, void* context) {
    UiResult* ui = (UiResult*)context;
    if(!ui) return false;
    if(event->type != InputTypeShort) return false;

    switch(event->key) {
    case InputKeyBack:
        if(ui->back_callback) ui->back_callback(ui->back_context);
        return true;
    case InputKeyRight:
        if(ui->save_callback) ui->save_callback(ui->save_context);
        return true;
    default:
        return false;
    }
}

UiResult* ui_result_alloc(void) {
    UiResult* ui = malloc(sizeof(UiResult));
    ui->state = NULL;
    ui->back_callback = NULL;
    ui->back_context = NULL;
    ui->save_callback = NULL;
    ui->save_context = NULL;

    ui->view = view_alloc();
    view_set_draw_callback(ui->view, ui_result_draw_callback);
    view_set_input_callback(ui->view, ui_result_input_callback);
    view_set_context(ui->view, ui);
    view_allocate_model(ui->view, ViewModelTypeLockFree, sizeof(UiResult*));

    UiResult** model = view_get_model(ui->view);
    *model = ui;
    view_commit_model(ui->view, false);

    return ui;
}

void ui_result_free(UiResult* ui) {
    furi_assert(ui);
    view_free(ui->view);
    free(ui);
}

View* ui_result_get_view(UiResult* ui) {
    furi_assert(ui);
    return ui->view;
}

void ui_result_set_state(UiResult* ui, AppState* state) {
    furi_assert(ui);
    ui->state = state;
}

void ui_result_set_back_callback(UiResult* ui, UiResultCallback callback, void* context) {
    ui->back_callback = callback;
    ui->back_context = context;
}

void ui_result_set_save_callback(UiResult* ui, UiResultCallback callback, void* context) {
    ui->save_callback = callback;
    ui->save_context = context;
}

void ui_result_show_stats(UiResult* ui, AppState* state) {
    furi_assert(ui);
    ui->state = state;
}
