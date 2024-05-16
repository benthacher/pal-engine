#pragma once

#include <stdint.h>
#include "entity.h"
#include "mathutils.h"

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

enum button_state game_get_button(enum button button);
void game_get_pointer_position(struct vec2 *position);
void game_get_pointer_velocity(struct vec2 *velocity);