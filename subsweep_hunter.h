#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <gui/view_dispatcher.h>
#include <storage/storage.h>
#include <lib/subghz/subghz_tx_rx_worker.h>

#include "app_state.h"
#include "subghz_sweeper.h"
#include "storage_manager.h"
#include "ui_main.h"
#include "ui_result.h"

typedef enum {
    SubSweepViewMain = 0,
    SubSweepViewResult,
} SubSweepView;

typedef struct {
    Gui* gui;
    ViewDispatcher* view_dispatcher;
    AppState state;
    SubGhzSweeper* sweeper;
    StorageManager* storage_manager;
    UiMain* ui_main;
    UiResult* ui_result;
    FuriTimer* sweep_timer;
} SubSweepHunterApp;

int32_t subsweep_hunter_app(void* p);
