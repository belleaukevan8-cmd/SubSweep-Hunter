#include "subghz_sweeper.h"
#include <lib/subghz/subghz_tx_rx_worker.h>
#include <applications/drivers/subghz/cc1101_ext/cc1101_ext.h>
#include <lib/subghz/devices/devices.h>
#include <lib/subghz/subghz_setting.h>
#include <flipper_application/flipper_application.h>
#include <furi_hal_subghz.h>
#include <stdlib.h>
#include <furi.h>

struct SubGhzSweeper {
    bool initialized;
    bool running;
    uint32_t current_freq;
};

SubGhzSweeper* subghz_sweeper_alloc(void) {
    SubGhzSweeper* sweeper = malloc(sizeof(SubGhzSweeper));
    sweeper->initialized = false;
    sweeper->running = false;
    sweeper->current_freq = 433920000;
    return sweeper;
}

void subghz_sweeper_free(SubGhzSweeper* sweeper) {
    furi_assert(sweeper);
    free(sweeper);
}

bool subghz_sweeper_init(SubGhzSweeper* sweeper) {
    furi_assert(sweeper);

    // Request SubGHz ownership
    if(furi_hal_subghz_is_locked()) {
        return false;
    }

    furi_hal_subghz_reset();
    furi_hal_subghz_load_custom_preset(furi_hal_subghz_preset_ook_650khz_async_regs);
    furi_hal_subghz_set_frequency_and_path(433920000);
    furi_hal_subghz_start_async_rx(NULL, NULL);

    sweeper->initialized = true;
    sweeper->current_freq = 433920000;
    return true;
}

void subghz_sweeper_deinit(SubGhzSweeper* sweeper) {
    furi_assert(sweeper);
    if(sweeper->initialized) {
        furi_hal_subghz_stop_async_rx();
        furi_hal_subghz_sleep();
        sweeper->initialized = false;
        sweeper->running = false;
    }
}

void subghz_sweeper_start(SubGhzSweeper* sweeper) {
    furi_assert(sweeper);
    if(sweeper->initialized) {
        sweeper->running = true;
    }
}

void subghz_sweeper_stop(SubGhzSweeper* sweeper) {
    furi_assert(sweeper);
    sweeper->running = false;
}

bool subghz_sweeper_set_frequency(SubGhzSweeper* sweeper, uint32_t frequency) {
    furi_assert(sweeper);
    if(!sweeper->initialized) return false;

    // Validate frequency range (300-928 MHz for CC1101)
    if(frequency < 300000000 || frequency > 928000000) return false;

    furi_hal_subghz_stop_async_rx();
    uint32_t actual = furi_hal_subghz_set_frequency_and_path(frequency);
    furi_hal_subghz_start_async_rx(NULL, NULL);

    sweeper->current_freq = actual;
    return true;
}

int16_t subghz_sweeper_read_rssi(SubGhzSweeper* sweeper) {
    furi_assert(sweeper);
    if(!sweeper->initialized) return -127;

    // Read RSSI via public HAL API
    float rssi = furi_hal_subghz_get_rssi();
    return (int16_t)rssi;
}

void subghz_sweeper_tick(SubGhzSweeper* sweeper, AppState* state) {
    furi_assert(sweeper);
    furi_assert(state);

    if(!sweeper->running || !sweeper->initialized) return;

    // Set frequency to current state frequency
    subghz_sweeper_set_frequency(sweeper, state->current_frequency);

    // Small delay to let the radio settle
    furi_delay_ms(5);

    // Read RSSI
    int16_t rssi = subghz_sweeper_read_rssi(sweeper);
    app_state_update_rssi(state, rssi);

    // Check threshold
    if(rssi > state->threshold) {
        app_state_increment_signal(state);
    }

    // Advance frequency
    app_state_next_frequency(state);
}
