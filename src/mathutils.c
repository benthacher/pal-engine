#include "mathutils.h"
#include <math.h>
#include <stdbool.h>

#include "pal.h"

int pal_abs(int num) {
    return num > 0 ? num : -num;
}

pal_float_t pal_fabs(pal_float_t num) {
#if defined PAL_USE_FLOAT32
    return fabsf(num);
#else
    return fabs(num);
#endif
}

pal_float_t pal_floor(pal_float_t num) {
#if defined PAL_USE_FLOAT32
    return floorf(num);
#else
    return floor(num);
#endif
}

pal_float_t pal_ceil(pal_float_t num) {
#if defined PAL_USE_FLOAT32
    return ceilf(num);
#else
    return ceil(num);
#endif
}

pal_float_t pal_round(pal_float_t num) {
#if defined PAL_USE_FLOAT32
    return roundf(num);
#else
    return round(num);
#endif
}

pal_float_t pal_fmin(pal_float_t a, pal_float_t b) {
    return a < b ? a : b;
}

pal_float_t pal_fmax(pal_float_t a, pal_float_t b) {
    return a > b ? a : b;
}

int pal_min(int a, int b) {
    return a < b ? a : b;
}

int pal_max(int a, int b) {
    return a > b ? a : b;
}

pal_float_t lerp(pal_float_t x1, pal_float_t x2, pal_float_t t) {
    return x1 * (1 - t) + x2 * t;
}

void vec2_lerp(struct vec2 *v1, struct vec2 *v2, pal_float_t t, struct vec2 *v_out) {
    v_out->x = lerp(v1->x, v2->x, t);
    v_out->y = lerp(v1->y, v2->y, t);
}

pal_float_t vec2_mag(struct vec2 *v) {
    return pal_hypot(v->x, v->y);
}

pal_float_t vec2_squared_mag(struct vec2 *v) {
    return v->x * v->x + v->y * v->y;
}

pal_float_t vec2_dir(struct vec2 *v) {
    return pal_atan2(v->y, v->x);
}

void vec2_set_mag(struct vec2 *v, pal_float_t mag, struct vec2 *v_out) {
    pal_float_t dir = vec2_dir(v);

    v_out->x = mag * pal_cos(dir);
    v_out->y = mag * pal_sin(dir);
}

void vec2_set_dir(struct vec2 *v, pal_float_t dir, struct vec2 *v_out) {
    pal_float_t mag = vec2_mag(v);

    v_out->x = mag * pal_cos(dir);
    v_out->y = mag * pal_sin(dir);
}

void vec2_add(struct vec2 *v1, struct vec2 *v2, struct vec2 *sum) {
    sum->x = v1->x + v2->x;
    sum->y = v1->y + v2->y;
}

void vec2_sub(struct vec2 *v1, struct vec2 *v2, struct vec2 *diff) {
    diff->x = v1->x - v2->x;
    diff->y = v1->y - v2->y;
}

pal_float_t vec2_dot(struct vec2 *v1, struct vec2 *v2) {
    return v1->x * v2->x + v1->y * v2->y;
}

pal_float_t vec2_cross(struct vec2 *v1, struct vec2 *v2) {
    return v1->x * v2->y - v1->y * v2->x;
}

void vec2_transform(const struct vec2 *v, const struct mat2 *m, struct vec2 *v_out) {
    v_out->x = m->a * v->x + m->b * v->y;
    v_out->y = m->c * v->x + m->d * v->y;
}

void vec2_rotate(struct vec2 *v, pal_float_t angle, struct vec2 *v_out) {
    // construct a rotation matrix and return transformed vector
    pal_float_t cos_angle = pal_cos(angle);
    pal_float_t sin_angle = pal_sin(angle);

    vec2_transform(v, &(struct mat2) {
        .a = cos_angle, .b = -sin_angle,
        .c = sin_angle, .d =  cos_angle,
    }, v_out);
}

void vec2_scale(struct vec2 *v, pal_float_t scale, struct vec2 *v_out) {
    v_out->x = v->x * scale;
    v_out->y = v->y * scale;
}

void vec2_normalize(struct vec2 *v, struct vec2 *v_out) {
    pal_float_t mag = vec2_mag(v);
    v_out->x = v->x / mag;
    v_out->y = v->y / mag;
}

pal_float_t mat2_det(struct mat2 *m) {
    return m->a * m->d - m->b * m->c;
}

void mat2_multiply(struct mat2 *m1, struct mat2 *m2, struct mat2 *product) {
    product->a = m1->a * m2->a + m1->b * m2->c;
    product->b = m1->a * m2->b + m1->b * m2->d;
    product->c = m1->c * m2->a + m1->d * m2->c;
    product->d = m1->c * m2->b + m1->d * m2->d;
}

bool mat2_inv(struct mat2 *m, struct mat2 *m_inv) {
    pal_float_t det = mat2_det(m);

    if (det == 0)
        return false;

    pal_float_t det_inv = 1 / det;
    m_inv->a =  det_inv * m->d;
    m_inv->b = -det_inv * m->b;
    m_inv->c = -det_inv * m->c;
    m_inv->d =  det_inv * m->a;

    return true;
}

pal_float_t rand_float() {
    return pal_rand() / ((pal_float_t) PAL_RAND_MAX + 1);
}

pal_float_t rand_float_range(pal_float_t min, pal_float_t max) {
    return pal_rand() / (((pal_float_t) PAL_RAND_MAX + 1) / (max - min)) + min;
}

int rand_range(int min, int max) {
    return pal_rand() % (max - min) + min;
}