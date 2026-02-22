#include "ui_main.h"
#include <gui/elements.h>
#include <gui/view.h>
#include <furi.h>
#include <stdio.h>
#include <stdlib.h>

struct UiMain {
    View* view;
    AppState* state;
    UiMainCallback toggle_callback;
    void* toggle_context;
    UiMainCallback quit_callback;
    void* quit_context;
    UiMainCallback result_callback;
    void* result_context;
};

static void ui_main_draw_callback(Canvas* canvas, void* model) {
    UiMain* ui = (UiMain*)model;
    if(!ui || !ui->state) return;

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 10, "SubSweep Hunter");

    canvas_set_font(canvas, FontSecondary);

    // Sweep status
    canvas_draw_str(canvas, 2, 22, "Sweep:");
    canvas_draw_str(canvas, 40, 22, ui->state->sweep_active ? "ON " : "OFF");

    // Current frequency
    char freq_buf[32];
    snprintf(freq_buf, sizeof(freq_buf), "Freq: %lu MHz",
        (unsigned long)(ui->state->current_frequency / 1000000));
    canvas_draw_str(canvas, 2, 32, freq_buf);

    // RSSI
    char rssi_buf[24];
    snprintf(rssi_buf, sizeof(rssi_buf), "RSSI: %d dBm", (int)ui->state->current_rssi);
    canvas_draw_str(canvas, 2, 42, rssi_buf);

    // Signal count
    char sig_buf[24];
    snprintf(sig_buf, sizeof(sig_buf), "Signals: %lu", (unsigned long)ui->state->signal_count);
    canvas_draw_str(canvas, 2, 52, sig_buf);

    // Threshold
    char thr_buf[24];
    snprintf(thr_buf, sizeof(thr_buf), "Thr: %d dBm", (int)ui->state->threshold);
    canvas_draw_str(canvas, 72, 52, thr_buf);

    // Button hints
    elements_button_left(canvas, "Back");
    elements_button_center(canvas, ui->state->sweep_active ? "Stop" : "Start");
    elements_button_right(canvas, "Result");
}

static bool ui_main_input_callback(InputEvent* event, void* context) {
    UiMain* ui = (UiMain*)context;
    if(!ui) return false;

    if(event->type != InputTypeShort && event->type != InputTypeRepeat) return false;

    switch(event->key) {
    case InputKeyOk:
        if(ui->toggle_callback) ui->toggle_callback(ui->toggle_context);
        return true;

    case InputKeyUp:
        if(ui->state) ui->state->threshold += 5;
        return true;

    case InputKeyDown:
        if(ui->state) ui->state->threshold -= 5;
        return true;

    case InputKeyRight:
        if(ui->result_callback) ui->result_callback(ui->result_context);
        return true;

    case InputKeyBack:
        if(ui->quit_callback) ui->quit_callback(ui->quit_context);
        return true;

    default:
        return false;
    }
}

UiMain* ui_main_alloc(void) {
    UiMain* ui = malloc(sizeof(UiMain));
    ui->state = NULL;
    ui->toggle_callback = NULL;
    ui->toggle_context = NULL;
    ui->quit_callback = NULL;
    ui->quit_context = NULL;
    ui->result_callback = NULL;
    ui->result_context = NULL;

    ui->view = view_alloc();
    view_set_draw_callback(ui->view, ui_main_draw_callback);
    view_set_input_callback(ui->view, ui_main_input_callback);
    view_set_context(ui->view, ui);
    view_allocate_model(ui->view, ViewModelTypeLockFree, sizeof(UiMain*));

    // Store ui pointer in model for draw callback
    UiMain** model = view_get_model(ui->view);
    *model = ui;
    view_commit_model(ui->view, false);

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
}

void ui_main_set_toggle_callback(UiMain* ui, UiMainCallback callback, void* context) {
    ui->toggle_callback = callback;
    ui->toggle_context = context;
}

void ui_main_set_quit_callback(UiMain* ui, UiMainCallback callback, void* context) {
    ui->quit_callback = callback;
    ui->quit_context = context;
}

void ui_main_set_result_callback(UiMain* ui, UiMainCallback callback, void* context) {
    ui->result_callback = callback;
    ui->result_context = context;
}
