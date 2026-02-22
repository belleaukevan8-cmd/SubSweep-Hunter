#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "app_state.h"

typedef struct SubGhzSweeper SubGhzSweeper;

SubGhzSweeper* subghz_sweeper_alloc(void);
void subghz_sweeper_free(SubGhzSweeper* sweeper);

bool subghz_sweeper_init(SubGhzSweeper* sweeper);
void subghz_sweeper_deinit(SubGhzSweeper* sweeper);

void subghz_sweeper_start(SubGhzSweeper* sweeper);
void subghz_sweeper_stop(SubGhzSweeper* sweeper);

bool subghz_sweeper_set_frequency(SubGhzSweeper* sweeper, uint32_t frequency);
int16_t subghz_sweeper_read_rssi(SubGhzSweeper* sweeper);

void subghz_sweeper_tick(SubGhzSweeper* sweeper, AppState* state);
