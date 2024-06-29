#include "graphics.h"

#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#include "mathutils.h"
#include "pal.h"

void graphics_draw_line(int x1, int y1, int x2, int y2, struct color color) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int start_x = pal_max(dx > 0 ? x1 : x2, 0);
    int start_y = pal_max(dy > 0 ? y1 : y2, 0);
    int end_x =   pal_min(dx > 0 ? x2 : x1, PAL_SCREEN_WIDTH);
    int end_y =   pal_min(dy > 0 ? y2 : y1, PAL_SCREEN_HEIGHT);

    int draw_x, draw_y;

    if (pal_fabs(dx) >= pal_fabs(dy)) {
        for (draw_x = start_x; draw_x < end_x; draw_x++) {
            draw_y = y1 + dy * (draw_x - x1) / dx;
            pal_screen_draw_pixel(draw_x, draw_y, color);
        }
    } else {
        for (draw_y = start_y; draw_y < end_y; draw_y++) {
            draw_x = x1 + dx * (draw_y - y1) / dy;
            pal_screen_draw_pixel(draw_x, draw_y, color);
        }
    }
}

void graphics_draw_rect(int x, int y, int width, int height, struct color c) {
    for (int dy = y; dy < y + height; dy++) {
        for (int dx = x; dx < x + width; dx++) {
            pal_screen_draw_pixel(x + dx, y + dy, c);
        }
    }
}

void graphics_draw_circle(int x, int y, pal_float_t radius, struct color c) {
    int start_x = pal_fmax(x - radius, 0);
    int start_y = pal_fmax(y - radius, 0);
    int end_x =   pal_fmin(x + radius, PAL_SCREEN_WIDTH);
    int end_y =   pal_fmin(y + radius, PAL_SCREEN_HEIGHT);
    int draw_x;
    int draw_y;
    pal_float_t dist;

    for (draw_y = start_y; draw_y <= end_y; draw_y++) {
        for (draw_x = start_x; draw_x <= end_x; draw_x++) {
            dist = pal_hypot(draw_x - x, draw_y - y);

            if (dist < radius)
                pal_screen_draw_pixel(draw_x, draw_y, c);
        }
    }
}

void graphics_stroke_circle(int x, int y, pal_float_t radius, struct color c, pal_float_t stroke_width) {
    int start_x = pal_fmax(x - radius, 0);
    int start_y = pal_fmax(y - radius, 0);
    int end_x =   pal_fmin(x + radius, PAL_SCREEN_WIDTH);
    int end_y =   pal_fmin(y + radius, PAL_SCREEN_HEIGHT);
    int draw_x;
    int draw_y;
    pal_float_t dist;

    for (draw_y = start_y; draw_y <= end_y; draw_y++) {
        for (draw_x = start_x; draw_x <= end_x; draw_x++) {
            dist = pal_hypot(draw_x - x, draw_y - y);

            if (dist <= radius && dist > radius - stroke_width)
                pal_screen_draw_pixel(draw_x, draw_y, c);
        }
    }
}

void graphics_draw_transformed_rect(int x, int y, int width, int height, struct color c, struct mat2 *m) {
    struct vec2 p_trans;
    struct mat2 m_inv;
    uint32_t row, col;
    int draw_x, draw_y;

    if (!mat2_inv(m, &m_inv))
        return;

    // compute transformed rect corners
    struct vec2 tr, tl, bl, br;
    vec2_transform(&(struct vec2) {  width / 2, -height / 2 }, m, &tr);
    vec2_transform(&(struct vec2) { -width / 2, -height / 2 }, m, &tl);
    vec2_transform(&(struct vec2) { -width / 2,  height / 2 }, m, &bl);
    vec2_transform(&(struct vec2) {  width / 2,  height / 2 }, m, &br);

    // get start and end point for looping through screen pixels
    struct vec2 start = { pal_floor(pal_fmin(tr.x, pal_fmin(tl.x, pal_fmin(bl.x, br.x)))), pal_floor(pal_fmin(tr.y, pal_fmin(tl.y, pal_fmin(bl.y, br.y)))) } ;
    struct vec2 end =   { pal_fmax(tr.x, pal_fmax(tl.x, pal_fmax(bl.x, br.x))), pal_fmax(tr.y, pal_fmax(tl.y, pal_fmax(bl.y, br.y))) } ;

    struct vec2 center_offset = { width / 2, height / 2 };

    struct vec2 p = start;
    for (p.y = start.y; p.y < end.y; p.y++) {
        for (p.x = start.x; p.x < end.x; p.x++) {
            // transform p to p_trans to find the nearest pixel in rect
            vec2_transform(&p, &m_inv, &p_trans);
            vec2_add(&p_trans, &center_offset, &p_trans);

            // NEED TO ROUND/FLOOR not truncate
            col = pal_round(p_trans.x);
            row = pal_round(p_trans.y);

            if (col >= 0 && row >= 0 && col < width && row < height) {
                // draw
                draw_x = p.x + x;
                draw_y = p.y + y;

                pal_screen_draw_pixel(draw_x, draw_y, c);
            }
        }
    }
}

void graphics_draw_transformed_image(struct image *image, int x, int y, struct mat2 *m) {
    struct vec2 p_trans;
    struct mat2 m_inv;
    uint32_t row, col;
    int draw_x, draw_y;

    if (!mat2_inv(m, &m_inv))
        return;

    // compute transformed image corners
    struct vec2 tr, tl, bl, br;
    vec2_transform(&(struct vec2) {  image->width / 2, -image->height / 2 }, m, &tr);
    vec2_transform(&(struct vec2) { -image->width / 2, -image->height / 2 }, m, &tl);
    vec2_transform(&(struct vec2) { -image->width / 2,  image->height / 2 }, m, &bl);
    vec2_transform(&(struct vec2) {  image->width / 2,  image->height / 2 }, m, &br);

    // get start and end point for looping through screen pixels
    struct vec2 start = { pal_fmin(tr.x, pal_fmin(tl.x, pal_fmin(bl.x, br.x))), pal_fmin(tr.y, pal_fmin(tl.y, pal_fmin(bl.y, br.y))) - 1 } ;
    struct vec2 end =   { pal_fmax(tr.x, pal_fmax(tl.x, pal_fmax(bl.x, br.x))), pal_fmax(tr.y, pal_fmax(tl.y, pal_fmax(bl.y, br.y))) + 1 } ;

    struct vec2 center_offset = { image->width / 2, image->height / 2 };

    struct vec2 p = start;
    for (p.y = start.y; p.y < end.y; p.y++) {
        for (p.x = start.x; p.x < end.x; p.x++) {
            // transform p to p_trans to find the nearest pixel in image
            vec2_transform(&p, &m_inv, &p_trans);
            vec2_add(&p_trans, &center_offset, &p_trans);

            // NEED TO ROUND/FLOOR not truncate
            col = pal_round(p_trans.x);
            row = pal_round(p_trans.y);

            if (col >= 0 && row >= 0 && col < image->width && row < image->height) {
                // draw
                draw_x = p.x + x;
                draw_y = p.y + y;

                pal_screen_draw_pixel(draw_x, draw_y, image->data[row * image->width + col]);
            }
        }
    }
}

void graphics_draw_image(struct image *image, int x, int y, pal_float_t angle, pal_float_t scale) {
    pal_float_t cos_angle = pal_cos(angle);
    pal_float_t sin_angle = pal_sin(angle);

    struct mat2 m = {
        cos_angle * scale, -sin_angle * scale,
        sin_angle * scale,  cos_angle * scale,
    };

    graphics_draw_transformed_image(image, x, y, &m);
}
