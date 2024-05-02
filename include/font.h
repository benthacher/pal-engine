#pragma once

#include <stddef.h>
#include <stdbool.h>
#include "graphics.h"

/**
 * @brief Draws text starting at given x and y
 *
 * @param x
 * @param y
 * @param text
 * @param invert_colors
 */
void draw_text(int x, int y, char *text, bool invert_colors);