#include "app_state.h"

void app_state_init(AppState* state) {
    state->sweep_active = false;
    state->current_frequency = 433920000;
    state->current_rssi = -127;
    state->signal_count = 0;
    state->threshold = -80;
    state->sweep_start_freq = 300000000;
    state->sweep_end_freq = 928000000;
    state->step_size = 1000000;
    state->rssi_max = -127;
    state->best_frequency = 433920000;
}

void app_state_reset(AppState* state) {
    state->sweep_active = false;
    state->current_frequency = state->sweep_start_freq;
    state->current_rssi = -127;
    state->signal_count = 0;
    state->rssi_max = -127;
    state->best_frequency = state->sweep_start_freq;
}

void app_state_increment_signal(AppState* state) {
    state->signal_count++;
}

void app_state_update_rssi(AppState* state, int16_t rssi) {
    state->current_rssi = rssi;
    if(rssi > state->rssi_max) {
        state->rssi_max = rssi;
        state->best_frequency = state->current_frequency;
    }
}

bool app_state_next_frequency(AppState* state) {
    if(state->current_frequency + state->step_size > state->sweep_end_freq) {
        state->current_frequency = state->sweep_start_freq;
        return false;
    }
    state->current_frequency += state->step_size;
    return true;
}
