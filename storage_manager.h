#pragma once

#include "app_state.h"
#include <storage/storage.h>
#include <stdbool.h>

#define SUBSWEEP_LOG_PATH "/ext/subsweep_logs.txt"

typedef struct StorageManager StorageManager;

StorageManager* storage_manager_alloc(void);
void storage_manager_free(StorageManager* manager);

bool storage_manager_init(StorageManager* manager);
void storage_manager_deinit(StorageManager* manager);

bool storage_manager_create_log_file(StorageManager* manager);
bool storage_manager_append_entry(StorageManager* manager, uint32_t frequency, int16_t rssi, uint32_t signal_count);
bool storage_manager_save_log(StorageManager* manager, AppState* state);
