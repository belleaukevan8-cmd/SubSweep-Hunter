#include "subghz_sweeper.h"

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_subghz.h>
#include <stdlib.h>

/*
 * Registres OOK 650kHz - chargés via furi_hal_subghz_load_registers() (API publique SDK)
 * Identiques à FuriHalSubGhzPresetOok650Async du firmware officiel
 * Format: paires [addr, value], terminées par [0,0]
 */
static const uint8_t subghz_preset_ook650_regs[] = {
    0x02, 0x0D, // IOCFG0
    0x03, 0x47, // FIFOTHR
    0x08, 0x32, // PKTCTRL0
    0x0B, 0x06, // FSCTRL1
    0x10, 0x17, // MDMCFG4
    0x11, 0x32, // MDMCFG3
    0x12, 0x30, // MDMCFG2
    0x13, 0x02, // MDMCFG1
    0x14, 0xF8, // MDMCFG0
    0x15, 0x47, // DEVIATN
    0x17, 0x30, // MCSM1
    0x18, 0x18, // MCSM0
    0x19, 0x14, // FOCCFG
    0x1A, 0x6C, // BSCFG
    0x1B, 0x07, // AGCCTRL2
    0x1C, 0x00, // AGCCTRL1
    0x1D, 0x91, // AGCCTRL0
    0x20, 0xFB, // WORCTRL
    0x26, 0x11, // FREND1
    0x27, 0xE9, // FREND0
    0x28, 0xC0, // FSCAL3
    0x29, 0x00, // FSCAL2
    0x2A, 0x00, // FSCAL1
    0x2B, 0x11, // FSCAL0
    0x00, 0x00  // fin
};

static const uint8_t subghz_patable_ook[8] = {0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

struct SubGhzSweeper {
    bool initialized;
    bool running;
};

SubGhzSweeper* subghz_sweeper_alloc(void) {
    SubGhzSweeper* sweeper = malloc(sizeof(SubGhzSweeper));
    sweeper->initialized = false;
    sweeper->running = false;
    return sweeper;
}

void subghz_sweeper_free(SubGhzSweeper* sweeper) {
    furi_assert(sweeper);
    free(sweeper);
}

bool subghz_sweeper_init(SubGhzSweeper* sweeper) {
    furi_assert(sweeper);

    furi_hal_subghz_reset();
    furi_hal_subghz_load_registers(subghz_preset_ook650_regs);
    furi_hal_subghz_load_patable(subghz_patable_ook);

    /* Démarrer sur 433.92 MHz - fréquence certifiée valide */
    furi_hal_subghz_idle();
    furi_hal_subghz_set_frequency_and_path(433920000);
    furi_hal_subghz_rx();

    sweeper->initialized = true;
    sweeper->running = false;
    return true;
}

void subghz_sweeper_deinit(SubGhzSweeper* sweeper) {
    furi_assert(sweeper);
    if(sweeper->initialized) {
        furi_hal_subghz_idle();
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
    if(sweeper->initialized) {
        furi_hal_subghz_idle();
    }
}

void subghz_sweeper_tick(SubGhzSweeper* sweeper, AppState* state) {
    furi_assert(sweeper);
    furi_assert(state);

    if(!sweeper->running || !sweeper->initialized) return;

    uint32_t freq = state->current_frequency;

    /*
     * DOUBLE GARDE-FOU : vérifier la fréquence avant tout appel radio.
     * Les plages valides sont exactement celles de furi_hal_subghz_is_frequency_valid() :
     *   281-361 MHz, 378-481 MHz, 749-962 MHz
     * Si la fréquence est hors plage → on avance sans toucher au radio.
     */
    bool valid = ((freq >= 281000000 && freq <= 361000000) ||
                  (freq >= 378000000 && freq <= 481000000) ||
                  (freq >= 749000000 && freq <= 962000000));

    if(!valid) {
        /* Ne jamais appeler set_frequency_and_path sur une freq invalide */
        app_state_next_frequency(state);
        return;
    }

    /* Séquence correcte: idle → set_frequency → rx → mesure */
    furi_hal_subghz_idle();
    furi_hal_subghz_set_frequency_and_path(freq);
    furi_hal_subghz_rx();

    /* Stabilisation radio ~5ms */
    furi_delay_ms(5);

    /* Lecture RSSI (API publique exposée SDK officiel) */
    float rssi_f = furi_hal_subghz_get_rssi();
    int16_t rssi = (int16_t)rssi_f;

    app_state_update_rssi(state, rssi);

    if(rssi > state->threshold) {
        app_state_increment_signal(state);
    }

    app_state_next_frequency(state);
}
