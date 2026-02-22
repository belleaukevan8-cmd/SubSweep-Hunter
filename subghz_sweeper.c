#include "subghz_sweeper.h"

#include <furi.h>
#include <furi_hal.h>
#include <furi_hal_subghz.h>
#include <stdlib.h>

/*
 * Registres CC1101 pour mode réception simple OOK 650kHz (identique à FuriHalSubGhzPresetOok650Async)
 * On les charge manuellement via furi_hal_subghz_load_registers() qui est exposée SDK.
 * Source: firmware officiel furi_hal_subghz.c
 */
static const uint8_t subghz_preset_ook650_regs[] = {
    /* addr, value */
    0x00, 0x00, // IOCFG2
    0x01, 0x00, // IOCFG1
    0x02, 0x0D, // IOCFG0 - GDO0 assert on sync word
    0x03, 0x07, // FIFOTHR
    0x07, 0x04, // PKTCTRL1
    0x08, 0x05, // PKTCTRL0 - variable length, CRC enable
    0x0B, 0x06, // FSCTRL1
    0x10, 0x17, // MDMCFG4 - chanbw 325kHz
    0x11, 0x32, // MDMCFG3 - datarate
    0x12, 0x30, // MDMCFG2 - OOK/ASK, no preamble
    0x13, 0x00, // MDMCFG1
    0x14, 0x00, // MDMCFG0
    0x15, 0x35, // DEVIATN
    0x18, 0x18, // MCSM0
    0x19, 0x16, // FOCCFG
    0x1B, 0x43, // AGCCTRL2
    0x1C, 0x40, // AGCCTRL1
    0x1D, 0x91, // AGCCTRL0
    0x20, 0xFB, // WORCTRL
    0x26, 0x11, // FREND1
    0x27, 0xE9, // FREND0
    0x29, 0x59, // FSCAL1
    0x2C, 0x88, // TEST2
    0x2D, 0x31, // TEST1
    0x2E, 0x09, // TEST0
    0x00, 0x00, // fin
};

static const uint8_t subghz_patable_ook[] = {0x00, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

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

    // Reset radio to safe state
    furi_hal_subghz_reset();

    // Charger les registres OOK via API publique exposée SDK
    furi_hal_subghz_load_registers(subghz_preset_ook650_regs);
    furi_hal_subghz_load_patable(subghz_patable_ook);

    // Mettre à une fréquence par défaut valide
    furi_hal_subghz_set_frequency_and_path(433920000);

    // Passer en mode réception (idle d'abord, puis rx)
    furi_hal_subghz_idle();
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

    // Changer de fréquence: idle avant, puis rx après
    furi_hal_subghz_idle();
    furi_hal_subghz_set_frequency_and_path(state->current_frequency);
    furi_hal_subghz_rx();

    // Laisser le radio se stabiliser (~5ms)
    furi_delay_ms(5);

    // Lire RSSI via API publique exposée SDK officiel
    float rssi_f = furi_hal_subghz_get_rssi();
    int16_t rssi = (int16_t)rssi_f;

    // Mettre à jour l'état
    app_state_update_rssi(state, rssi);

    // Comparer avec le seuil
    if(rssi > state->threshold) {
        app_state_increment_signal(state);
    }

    // Fréquence suivante
    app_state_next_frequency(state);
}
