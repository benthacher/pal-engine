#pragma once

#include <stdint.h>
#include "entity.h"

#define FPS                60
#define DT                 (1.0 / FPS)
#define FRAME_PERIOD_US    (DT * 1000000)

/**
 * @brief Runs screen loop
 *
 */
void game_run_loop();

/**
 * @brief Stops screen loop
 *
 */
void game_stop_loop();

void game_add_entity(struct entity *entity);
void game_remove_entity(struct entity *entity);