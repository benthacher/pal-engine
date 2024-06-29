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

/**
 * @brief Adds entity to game
 *
 * @param entity
 */
void game_add_entity(struct entity *entity);

/**
 * @brief Removes entity from game
 *
 * @param entity
 */
void game_remove_entity(struct entity *entity);

/**
 * @brief Get current input state of given button
 *
 * @param button
 * @return enum button_state
 */
enum button_state game_get_button(enum button button);

/**
 * @brief Returns current pointer state
 *
 * @return enum pointer_state
 */
enum pointer_state game_get_pointer_state();

/**
 * @brief Get current input state of pointer position
 *
 * @param position
 */
void game_get_pointer_position(struct vec2 *position);

/**
 * @brief Get current input state of pointer velocity
 *
 * @return const struct vec2*
 */
const struct vec2 *game_get_pointer_velocity();

/**
 * @brief Transforms screen coordinates to world coordinates
 *
 * @param screen_x
 * @param screen_y
 * @param world_pos
 */
void game_camera_screen_to_world(int screen_x, int screen_y, struct vec2 *world_pos);

/**
 * @brief Transforms world coordinates to screen coordinates
 *
 * @param world_pos
 * @param screen_x
 * @param screen_y
 */
void game_camera_world_to_screen(struct vec2 *world_pos, int *screen_x, int *screen_y);

/**
 * @brief Set camera's velocity
 *
 * @param velocity
 */
void game_camera_set_velocity(struct vec2 *velocity);

/**
 * @brief Set camera's position
 *
 * @param position
 */
void game_camera_set_position(struct vec2 *position);

/**
 * @brief Gets camera position in world
 *
 * @param position
 */
void game_camera_get_position(struct vec2 *position);

/**
 * @brief Calculates if point is on screen
 *
 * @param world_pos
 * @return true
 * @return false
 */
bool game_is_point_on_screen(struct vec2 *world_pos);

/**
 * @brief Set camera's transform
 *
 * @param transform
 */
void game_camera_set_transform(struct mat2 *transform);

/**
 * @brief Gets camera's transform
 *
 * @param transform
 */
void game_camera_get_transform(struct mat2 *transform);

/**
 * @brief Gets camera's inverse transform
 *
 * @param inv_transform
 */
void game_camera_get_inv_transform(struct mat2 *inv_transform);