#include "storage_manager.h"
#include <storage/storage.h>
#include <furi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct StorageManager {
    Storage* storage;
    bool initialized;
};

StorageManager* storage_manager_alloc(void) {
    StorageManager* manager = malloc(sizeof(StorageManager));
    manager->storage = NULL;
    manager->initialized = false;
    return manager;
}

void storage_manager_free(StorageManager* manager) {
    furi_assert(manager);
    storage_manager_deinit(manager);
    free(manager);
}

bool storage_manager_init(StorageManager* manager) {
    furi_assert(manager);
    manager->storage = furi_record_open(RECORD_STORAGE);
    manager->initialized = (manager->storage != NULL);
    return manager->initialized;
}

void storage_manager_deinit(StorageManager* manager) {
    furi_assert(manager);
    if(manager->storage) {
        furi_record_close(RECORD_STORAGE);
        manager->storage = NULL;
    }
    manager->initialized = false;
}

bool storage_manager_save_log(StorageManager* manager, AppState* state) {
    furi_assert(manager);
    furi_assert(state);
    if(!manager->initialized) return false;

    File* file = storage_file_alloc(manager->storage);
    bool ok = storage_file_open(file, SUBSWEEP_LOG_PATH, FSAM_WRITE, FSOM_OPEN_APPEND);

    if(ok) {
        char buf[128];
        int len = snprintf(
            buf,
            sizeof(buf),
            "Tick:%lu | Freq:%lu Hz | RSSI:%d dBm | Signals:%lu\n",
            (unsigned long)furi_get_tick(),
            (unsigned long)state->best_frequency,
            (int)state->rssi_max,
            (unsigned long)state->signal_count);
        if(len > 0) {
            storage_file_write(file, buf, (uint16_t)len);
        }
        storage_file_close(file);
    }

    storage_file_free(file);
    return ok;
}
