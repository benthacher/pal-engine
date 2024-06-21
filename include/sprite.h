#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "graphics.h"

struct sprite_frame {
    struct image *image;
    pal_float_t duration;
};

/**
 * @brief Struct to hold a sprite definition (frames, number of frames, whether to loop)
 *
 */
struct sprite_def {
    struct sprite_frame **frames;
    size_t num_frames;
    bool loop;
};

struct sprite {
    bool finished;
    size_t current_frame;
    struct sprite_def *sprite_def;
    pal_float_t frame_elapsed_time;
    pal_float_t previous_time;
};

/**
 * @brief Resets a sprite's animation
 *
 * @param sprite
 */
void sprite_init(struct sprite *sprite, struct sprite_def *sprite_def);

/**
 * @brief Updates a sprite's animation frame depending on elapsed time
 *
 * @param sprite
 */
void sprite_update(struct sprite *sprite);

/**
 * @brief Draws a sprite at a location on the screen with given angle and scale
 *
 * @param sprite
 * @param x
 * @param y
 * @param angle
 * @param scale
 */
void sprite_draw(struct sprite *sprite, int x, int y, pal_float_t angle, pal_float_t scale);