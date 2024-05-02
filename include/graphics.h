#pragma once

#include <stdint.h>
#include "mathutils.h"
#include "pal.h"

struct image {
    struct color *data; // pointer to preallocated image data
    int width;
    int height;
};

/**
 * @brief Draws line from (x1,y1) to (x2,y2)
 *
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @param color
 */
void graphics_draw_line(int x1, int y1, int x2, int y2, struct color color);

/**
 * @brief Draws rectangle starting at pos with width and height and color
 *
 * @param pos
 * @param width
 * @param height
 * @param color
 */
void graphics_draw_rect(int x, int y, int width, int height, struct color color);

/**
 * @brief Draw circle centered at position pos with radius
 *
 * @param pos
 * @param radius
 * @param color
 */
void graphics_draw_circle(int x, int y, float radius, struct color color);

/**
 * @brief Draws circle edge
 *
 * @param pos
 * @param radius
 * @param color
 * @param stroke_width
 */
void graphics_stroke_circle(int x, int y, float radius, struct color color, float stroke_width);

/**
 * @brief Draws transformed rectangle centered at pos
 *
 * @param pos
 * @param width
 * @param height
 * @param color
 * @param m
 */
void graphics_draw_transformed_rect(int x, int y, int width, int height, struct color color, struct mat2 *m);

/**
 * @brief Draws image transformed by input matrix m (2x2)
 *
 * @param image
 * @param pos
 * @param m
 */
void graphics_draw_transformed_image(struct image *image, int x, int y, struct mat2 *m);

/**
 * @brief Draws image to screen centered at pos, at given angle and scale
 *
 * @param image
 * @param pos
 * @param scale
 */
void graphics_draw_image(struct image *image, int x, int y, float angle, float scale);
