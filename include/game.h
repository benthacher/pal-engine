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
void game_loop_run();

/**
 * @brief Stops screen loop
 *
 */
void game_loop_stop();

/**
 * @brief Adds entity to game
 *
 * @param entity
 */
void game_entity_add(struct entity *entity);

/**
 * @brief Removes entity from game
 *
 * @param entity
 */
void game_entity_remove(struct entity *entity);

/**
 * @brief Get current input state of given button
 *
 * @param button
 * @return enum button_state
 */
enum button_state game_button_check(enum button button);

/**
 * @brief Returns current pointer state
 *
 * @return enum pointer_state
 */
enum pointer_state game_pointer_get_state();

/**
 * @brief Get current input state of pointer position
 *
 * @param position
 */
const struct vec2 *game_pointer_get_position();

/**
 * @brief Get current input state of pointer velocity
 *
 * @return const struct vec2*
 */
const struct vec2 *game_pointer_get_velocity();

/**
 * @brief Transforms screen coordinates to world coordinates
 *
 * @param screen_x
 * @param screen_y
 * @param world_pos
 */
void game_camera_screen_to_world(const int screen_x, const int screen_y, struct vec2 *world_pos);

/**
 * @brief Transforms world coordinates to screen coordinates
 *
 * @param world_pos
 * @param screen_x
 * @param screen_y
 */
void game_camera_world_to_screen(const struct vec2 *world_pos, int *screen_x, int *screen_y);

/**
 * @brief Set camera's velocity
 *
 * @param velocity
 */
void game_camera_set_velocity(const struct vec2 *velocity);

/**
 * @brief Set camera's position
 *
 * @param position
 */
void game_camera_set_position(const struct vec2 *position);

/**
 * @brief Gets camera position in world
 *
 * @param position
 */
const struct vec2 *game_camera_get_position();

/**
 * @brief Calculates if point is on screen
 *
 * @param world_pos
 * @return true
 * @return false
 */
bool game_is_point_on_screen(const struct vec2 *world_pos);

/**
 * @brief Set camera's transform
 *
 * @param transform
 */
void game_camera_set_transform(const struct mat2 *transform);

/**
 * @brief Gets camera's transform
 *
 * @return const struct mat2*
 */
const struct mat2 *game_camera_get_transform();

/**
 * @brief Gets camera's inverse transform
 *
 * @return const struct mat2*
 */
const struct mat2 *game_camera_get_inv_transform();