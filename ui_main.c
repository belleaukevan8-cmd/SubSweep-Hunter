#include "ui_main.h"
#include <gui/elements.h>
#include <furi.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * IMPORTANT: Le draw callback reçoit le "model" alloué par view_allocate_model.
 * Le context (via view_set_context) est reçu dans l'input_callback.
 * On stocke UiMain* dans le model pour que le draw y accède.
 */

struct UiMain {
    View* view;
    AppState* state;
    UiMainCallback toggle_cb;
    void* toggle_ctx;
    UiMainCallback quit_cb;
    void* quit_ctx;
    UiMainCallback result_cb;
    void* result_ctx;
};

/* Le model contient juste un pointeur vers UiMain */
typedef struct {
    UiMain* ui;
} UiMainModel;

static void ui_main_draw_callback(Canvas* canvas, void* _model) {
    UiMainModel* model = (UiMainModel*)_model;
    if(!model || !model->ui || !model->ui->state) return;

    AppState* s = model->ui->state;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 0, 10, "SubSweep Hunter");

    canvas_draw_line(canvas, 0, 12, 128, 12);

    canvas_set_font(canvas, FontSecondary);

    // Sweep status
    canvas_draw_str(canvas, 0, 23, s->sweep_active ? "Sweep: ON " : "Sweep: OFF");

    // Frequency
    char buf[32];
    snprintf(buf, sizeof(buf), "Freq:%lu MHz", (unsigned long)(s->current_frequency / 1000000));
    canvas_draw_str(canvas, 0, 33, buf);

    // RSSI
    snprintf(buf, sizeof(buf), "RSSI:%d dBm", (int)s->current_rssi);
    canvas_draw_str(canvas, 0, 43, buf);

    // Signals + threshold
    snprintf(buf, sizeof(buf), "Sig:%lu Thr:%d", (unsigned long)s->signal_count, (int)s->threshold);
    canvas_draw_str(canvas, 0, 53, buf);

    // Boutons
    elements_button_left(canvas, "Back");
    elements_button_center(canvas, s->sweep_active ? "Stop" : "Start");
    elements_button_right(canvas, "Result");
}

static bool ui_main_input_callback(InputEvent* event, void* context) {
    UiMain* ui = (UiMain*)context;
    if(!ui) return false;

    if(event->type != InputTypeShort && event->type != InputTypeRepeat) return false;

    switch(event->key) {
    case InputKeyOk:
        if(ui->toggle_cb) ui->toggle_cb(ui->toggle_ctx);
        return true;
    case InputKeyUp:
        if(ui->state) ui->state->threshold += 5;
        return true;
    case InputKeyDown:
        if(ui->state) ui->state->threshold -= 5;
        return true;
    case InputKeyRight:
        if(ui->result_cb) ui->result_cb(ui->result_ctx);
        return true;
    case InputKeyBack:
        if(ui->quit_cb) ui->quit_cb(ui->quit_ctx);
        return true;
    default:
        return false;
    }
}

UiMain* ui_main_alloc(void) {
    UiMain* ui = malloc(sizeof(UiMain));
    ui->state = NULL;
    ui->toggle_cb = NULL;
    ui->toggle_ctx = NULL;
    ui->quit_cb = NULL;
    ui->quit_ctx = NULL;
    ui->result_cb = NULL;
    ui->result_ctx = NULL;

    ui->view = view_alloc();
    view_set_context(ui->view, ui);
    view_set_draw_callback(ui->view, ui_main_draw_callback);
    view_set_input_callback(ui->view, ui_main_input_callback);

    // Allouer le model et y stocker le pointeur UiMain
    view_allocate_model(ui->view, ViewModelTypeLockFree, sizeof(UiMainModel));
    with_view_model(
        ui->view,
        UiMainModel * model,
        { model->ui = ui; },
        false);

    return ui;
}

void ui_main_free(UiMain* ui) {
    furi_assert(ui);
    view_free(ui->view);
    free(ui);
}

View* ui_main_get_view(UiMain* ui) {
    furi_assert(ui);
    return ui->view;
}

void ui_main_set_state(UiMain* ui, AppState* state) {
    furi_assert(ui);
    ui->state = state;
    // Mettre à jour aussi dans le model
    with_view_model(
        ui->view,
        UiMainModel * model,
        { model->ui = ui; },
        false);
}

void ui_main_set_toggle_callback(UiMain* ui, UiMainCallback cb, void* ctx) {
    ui->toggle_cb = cb;
    ui->toggle_ctx = ctx;
}

void ui_main_set_quit_callback(UiMain* ui, UiMainCallback cb, void* ctx) {
    ui->quit_cb = cb;
    ui->quit_ctx = ctx;
}

void ui_main_set_result_callback(UiMain* ui, UiMainCallback cb, void* ctx) {
    ui->result_cb = cb;
    ui->result_ctx = ctx;
}
