#pragma once

#include <stdint.h>
#include <stdbool.h>

/*
 * Bandes CC1101 valides - source: furi_hal_subghz.c firmware officiel
 *
 *   bool furi_hal_subghz_is_frequency_valid(uint32_t value) {
 *     if(!(value >= 281000000 && value <= 361000000) &&
 *        !(value >= 378000000 && value <= 481000000) &&
 *        !(value >= 749000000 && value <= 962000000))
 *         return false;
 *   }
 *
 * Toute fréquence hors ces 3 plages → furi_crash("SubGhz: Incorrect frequency during set.")
 */

#define BAND_COUNT 3

typedef struct {
    uint32_t start;
    uint32_t end;
    uint32_t step;
    const char* name;
} FreqBand;

typedef struct {
    bool sweep_active;
    uint8_t band_index;
    uint32_t current_frequency;
    int16_t current_rssi;
    uint32_t signal_count;
    int16_t threshold;
    int16_t rssi_max;
    uint32_t best_frequency;
    FreqBand bands[BAND_COUNT];
} AppState;

void app_state_init(AppState* state);
void app_state_reset(AppState* state);
void app_state_increment_signal(AppState* state);
void app_state_update_rssi(AppState* state, int16_t rssi);
bool app_state_next_frequency(AppState* state);
