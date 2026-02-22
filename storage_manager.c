#include "storage_manager.h"
#include <storage/storage.h>
#include <furi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct StorageManager {
    Storage* storage;
    File* file;
    bool initialized;
};

StorageManager* storage_manager_alloc(void) {
    StorageManager* manager = malloc(sizeof(StorageManager));
    manager->storage = NULL;
    manager->file = NULL;
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
    if(!manager->storage) return false;

    manager->file = storage_file_alloc(manager->storage);
    if(!manager->file) {
        furi_record_close(RECORD_STORAGE);
        manager->storage = NULL;
        return false;
    }

    manager->initialized = true;
    return true;
}

void storage_manager_deinit(StorageManager* manager) {
    furi_assert(manager);
    if(manager->file) {
        storage_file_close(manager->file);
        storage_file_free(manager->file);
        manager->file = NULL;
    }
    if(manager->storage) {
        furi_record_close(RECORD_STORAGE);
        manager->storage = NULL;
    }
    manager->initialized = false;
}

bool storage_manager_create_log_file(StorageManager* manager) {
    furi_assert(manager);
    if(!manager->initialized) return false;

    bool result = storage_file_open(
        manager->file, SUBSWEEP_LOG_PATH, FSAM_WRITE, FSOM_OPEN_APPEND);

    if(result) {
        const char* header = "# SubSweep Hunter Log\n# Timestamp | Frequency | RSSI | Signals\n";
        storage_file_write(manager->file, header, strlen(header));
        storage_file_close(manager->file);
    }
    return result;
}

bool storage_manager_append_entry(StorageManager* manager, uint32_t frequency, int16_t rssi, uint32_t signal_count) {
    furi_assert(manager);
    if(!manager->initialized) return false;

    bool opened = storage_file_open(
        manager->file, SUBSWEEP_LOG_PATH, FSAM_WRITE, FSOM_OPEN_APPEND);
    if(!opened) return false;

    char buf[64];
    int len = snprintf(buf, sizeof(buf), "%lu | %lu Hz | %d dBm | %lu\n",
        (unsigned long)furi_get_tick(),
        (unsigned long)frequency,
        (int)rssi,
        (unsigned long)signal_count);

    storage_file_write(manager->file, buf, len);
    storage_file_close(manager->file);
    return true;
}

bool storage_manager_save_log(StorageManager* manager, AppState* state) {
    furi_assert(manager);
    furi_assert(state);

    storage_manager_create_log_file(manager);
    return storage_manager_append_entry(
        manager,
        state->best_frequency,
        state->rssi_max,
        state->signal_count);
}
