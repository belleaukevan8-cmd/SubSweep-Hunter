#include "ui_result.h"
#include <gui/elements.h>
#include <furi.h>
#include <stdio.h>
#include <stdlib.h>

struct UiResult {
    View* view;
    AppState* state;
    UiResultCallback back_cb;
    void* back_ctx;
    UiResultCallback save_cb;
    void* save_ctx;
};

typedef struct {
    UiResult* ui;
} UiResultModel;

static void ui_result_draw_callback(Canvas* canvas, void* _model) {
    UiResultModel* model = (UiResultModel*)_model;
    if(!model || !model->ui || !model->ui->state) return;

    AppState* s = model->ui->state;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "Sweep Results");
    canvas_draw_line(canvas, 0, 12, 128, 12);

    canvas_set_font(canvas, FontSecondary);

    char buf[40];

    snprintf(buf, sizeof(buf), "Signals: %lu", (unsigned long)s->signal_count);
    canvas_draw_str(canvas, 0, 24, buf);

    snprintf(buf, sizeof(buf), "Best: %lu MHz", (unsigned long)(s->best_frequency / 1000000));
    canvas_draw_str(canvas, 0, 34, buf);

    snprintf(buf, sizeof(buf), "Max RSSI: %d dBm", (int)s->rssi_max);
    canvas_draw_str(canvas, 0, 44, buf);

    elements_button_left(canvas, "Back");
    elements_button_right(canvas, "Save");
}

static bool ui_result_input_callback(InputEvent* event, void* context) {
    UiResult* ui = (UiResult*)context;
    if(!ui) return false;
    if(event->type != InputTypeShort) return false;

    switch(event->key) {
    case InputKeyBack:
        if(ui->back_cb) ui->back_cb(ui->back_ctx);
        return true;
    case InputKeyRight:
        if(ui->save_cb) ui->save_cb(ui->save_ctx);
        return true;
    default:
        return false;
    }
}

UiResult* ui_result_alloc(void) {
    UiResult* ui = malloc(sizeof(UiResult));
    ui->state = NULL;
    ui->back_cb = NULL;
    ui->back_ctx = NULL;
    ui->save_cb = NULL;
    ui->save_ctx = NULL;

    ui->view = view_alloc();
    view_set_context(ui->view, ui);
    view_set_draw_callback(ui->view, ui_result_draw_callback);
    view_set_input_callback(ui->view, ui_result_input_callback);

    view_allocate_model(ui->view, ViewModelTypeLockFree, sizeof(UiResultModel));
    with_view_model(
        ui->view,
        UiResultModel * model,
        { model->ui = ui; },
        false);

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
    with_view_model(
        ui->view,
        UiResultModel * model,
        { model->ui = ui; },
        false);
}

void ui_result_set_back_callback(UiResult* ui, UiResultCallback cb, void* ctx) {
    ui->back_cb = cb;
    ui->back_ctx = ctx;
}

void ui_result_set_save_callback(UiResult* ui, UiResultCallback cb, void* ctx) {
    ui->save_cb = cb;
    ui->save_ctx = ctx;
}
