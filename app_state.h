#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    bool sweep_active;
    uint32_t current_frequency;
    int16_t current_rssi;
    uint32_t signal_count;
    int16_t threshold;
    uint32_t sweep_start_freq;
    uint32_t sweep_end_freq;
    uint32_t step_size;
    int16_t rssi_max;
    uint32_t best_frequency;
} AppState;
