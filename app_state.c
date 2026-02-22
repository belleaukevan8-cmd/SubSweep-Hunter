#include "app_state.h"

void app_state_init(AppState* state) {
    state->sweep_active = false;

    /*
     * Bandes exactes du firmware officiel (furi_hal_subghz.c)
     * Step 500kHz = bon compromis vitesse/résolution
     */
    state->bands[0].start = 281000000;
    state->bands[0].end   = 361000000;
    state->bands[0].step  = 500000;
    state->bands[0].name  = "281-361";

    state->bands[1].start = 378000000;
    state->bands[1].end   = 481000000;
    state->bands[1].step  = 500000;
    state->bands[1].name  = "378-481";

    state->bands[2].start = 749000000;
    state->bands[2].end   = 962000000;
    state->bands[2].step  = 500000;
    state->bands[2].name  = "749-962";

    state->band_index = 0;
    state->current_frequency = state->bands[0].start;
    state->current_rssi = -127;
    state->signal_count = 0;
    state->threshold = -80;
    state->rssi_max = -127;
    state->best_frequency = state->bands[0].start;
}

void app_state_reset(AppState* state) {
    state->sweep_active = false;
    state->band_index = 0;
    state->current_frequency = state->bands[0].start;
    state->current_rssi = -127;
    state->signal_count = 0;
    state->rssi_max = -127;
    state->best_frequency = state->bands[0].start;
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
    FreqBand* band = &state->bands[state->band_index];

    if(state->current_frequency + band->step <= band->end) {
        state->current_frequency += band->step;
        return true;
    }

    /* Passer à la bande suivante */
    state->band_index++;
    if(state->band_index >= BAND_COUNT) {
        state->band_index = 0;
    }
    state->current_frequency = state->bands[state->band_index].start;
    return (state->band_index != 0);
}
