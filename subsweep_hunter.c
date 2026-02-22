#include "subsweep_hunter.h"
#include <furi.h>
#include <stdlib.h>

// ─── Timer callback : avance le sweep périodiquement ─────────────────────────

static void sweep_timer_callback(void* context) {
    SubSweepHunterApp* app = (SubSweepHunterApp*)context;
    if(app->state.sweep_active) {
        subghz_sweeper_tick(app->sweeper, &app->state);
        view_dispatcher_send_custom_event(app->view_dispatcher, 0);
    }
}

// ─── Callbacks UI → logique ──────────────────────────────────────────────────

static void on_toggle_sweep(void* context) {
    SubSweepHunterApp* app = (SubSweepHunterApp*)context;
    if(app->state.sweep_active) {
        app->state.sweep_active = false;
        subghz_sweeper_stop(app->sweeper);
        furi_timer_stop(app->sweep_timer);
    } else {
        app->state.sweep_active = true;
        subghz_sweeper_start(app->sweeper);
        // 50ms = ~20 fréquences/sec
        furi_timer_start(app->sweep_timer, 50);
    }
}

static void on_quit(void* context) {
    SubSweepHunterApp* app = (SubSweepHunterApp*)context;
    if(app->state.sweep_active) {
        app->state.sweep_active = false;
        subghz_sweeper_stop(app->sweeper);
        furi_timer_stop(app->sweep_timer);
    }
    view_dispatcher_stop(app->view_dispatcher);
}

static void on_show_result(void* context) {
    SubSweepHunterApp* app = (SubSweepHunterApp*)context;
    ui_result_set_state(app->ui_result, &app->state);
    view_dispatcher_switch_to_view(app->view_dispatcher, SubSweepViewResult);
}

static void on_result_back(void* context) {
    SubSweepHunterApp* app = (SubSweepHunterApp*)context;
    view_dispatcher_switch_to_view(app->view_dispatcher, SubSweepViewMain);
}

static void on_save_result(void* context) {
    SubSweepHunterApp* app = (SubSweepHunterApp*)context;
    storage_manager_save_log(app->storage_manager, &app->state);
}

// ─── Custom event handler (redraw) ───────────────────────────────────────────

static bool custom_event_callback(void* context, uint32_t event) {
    UNUSED(event);
    UNUSED(context);
    return true;
}

// ─── Navigation (bouton back du view_dispatcher) ─────────────────────────────

static bool navigation_event_callback(void* context) {
    SubSweepHunterApp* app = (SubSweepHunterApp*)context;
    view_dispatcher_switch_to_view(app->view_dispatcher, SubSweepViewMain);
    return true;
}

// ─── Allocation ──────────────────────────────────────────────────────────────

static SubSweepHunterApp* subsweep_hunter_alloc(void) {
    SubSweepHunterApp* app = malloc(sizeof(SubSweepHunterApp));

    // État initial
    app_state_init(&app->state);

    // GUI + ViewDispatcher
    app->gui = furi_record_open(RECORD_GUI);
    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_enable_queue(app->view_dispatcher);
    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_custom_event_callback(app->view_dispatcher, custom_event_callback);
    view_dispatcher_set_navigation_event_callback(
        app->view_dispatcher, navigation_event_callback);

    // Vue principale
    app->ui_main = ui_main_alloc();
    ui_main_set_state(app->ui_main, &app->state);
    ui_main_set_toggle_callback(app->ui_main, on_toggle_sweep, app);
    ui_main_set_quit_callback(app->ui_main, on_quit, app);
    ui_main_set_result_callback(app->ui_main, on_show_result, app);
    view_dispatcher_add_view(
        app->view_dispatcher, SubSweepViewMain, ui_main_get_view(app->ui_main));

    // Vue résultat
    app->ui_result = ui_result_alloc();
    ui_result_set_back_callback(app->ui_result, on_result_back, app);
    ui_result_set_save_callback(app->ui_result, on_save_result, app);
    view_dispatcher_add_view(
        app->view_dispatcher, SubSweepViewResult, ui_result_get_view(app->ui_result));

    // SubGHz sweeper
    app->sweeper = subghz_sweeper_alloc();
    subghz_sweeper_init(app->sweeper);

    // Storage
    app->storage_manager = storage_manager_alloc();
    storage_manager_init(app->storage_manager);

    // Timer périodique pour le sweep
    app->sweep_timer =
        furi_timer_alloc(sweep_timer_callback, FuriTimerTypePeriodic, app);

    return app;
}

// ─── Libération ──────────────────────────────────────────────────────────────

static void subsweep_hunter_free(SubSweepHunterApp* app) {
    furi_assert(app);

    furi_timer_stop(app->sweep_timer);
    furi_timer_free(app->sweep_timer);

    subghz_sweeper_stop(app->sweeper);
    subghz_sweeper_deinit(app->sweeper);
    subghz_sweeper_free(app->sweeper);

    storage_manager_deinit(app->storage_manager);
    storage_manager_free(app->storage_manager);

    view_dispatcher_remove_view(app->view_dispatcher, SubSweepViewMain);
    view_dispatcher_remove_view(app->view_dispatcher, SubSweepViewResult);
    ui_main_free(app->ui_main);
    ui_result_free(app->ui_result);

    view_dispatcher_free(app->view_dispatcher);
    furi_record_close(RECORD_GUI);

    free(app);
}

// ─── Point d'entrée ──────────────────────────────────────────────────────────

int32_t subsweep_hunter_app(void* p) {
    UNUSED(p);

    SubSweepHunterApp* app = subsweep_hunter_alloc();
    view_dispatcher_switch_to_view(app->view_dispatcher, SubSweepViewMain);
    view_dispatcher_run(app->view_dispatcher);
    subsweep_hunter_free(app);

    return 0;
}
