#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "app_state.h"

/*
 * SubGHz Sweeper - utilise uniquement les API publiques exposées aux FAP externes:
 *   - furi_hal_subghz_get_rssi()        => exposée SDK
 *   - furi_hal_subghz_set_frequency_and_path() => exposée SDK
 *   - furi_hal_subghz_idle()            => exposée SDK
 *   - furi_hal_subghz_rx()              => exposée SDK
 *   - furi_hal_subghz_sleep()           => exposée SDK
 *   - furi_hal_subghz_reset()           => exposée SDK
 *   - furi_hal_subghz_load_registers()  => exposée SDK
 *   - furi_hal_subghz_load_patable()    => exposée SDK
 *
 * PAS d'utilisation de:
 *   - furi_hal_subghz_is_locked()    => NON exposée
 *   - furi_hal_subghz_start_async_rx() => nécessite callbacks non-null complexes
 *   - furi_hal_subghz_load_custom_preset() => incompatible selon SDK version
 *   - subghz_devices_init/deinit     => nécessite lib/subghz complète
 */

typedef struct SubGhzSweeper SubGhzSweeper;

SubGhzSweeper* subghz_sweeper_alloc(void);
void subghz_sweeper_free(SubGhzSweeper* sweeper);

bool subghz_sweeper_init(SubGhzSweeper* sweeper);
void subghz_sweeper_deinit(SubGhzSweeper* sweeper);

void subghz_sweeper_start(SubGhzSweeper* sweeper);
void subghz_sweeper_stop(SubGhzSweeper* sweeper);

void subghz_sweeper_tick(SubGhzSweeper* sweeper, AppState* state);
