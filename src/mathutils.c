#include "mathutils.h"
#include <math.h>
#include <stdbool.h>

#include "pal.h"

float lerp(float x1, float x2, float t) {
    return x1 * (1 - t) + x2 * t;
}

void vec2_lerp(struct vec2 *v1, struct vec2 *v2, float t, struct vec2 *v_out) {
    v_out->x = lerp(v1->x, v2->x, t);
    v_out->y = lerp(v1->y, v2->y, t);
}

float vec2_mag(struct vec2 *v) {
    return hypotf(v->x, v->y);
}

float vec2_squared_mag(struct vec2 *v) {
    return v->x * v->x + v->y * v->y;
}

float vec2_dir(struct vec2 *v) {
    return atan2f(v->y, v->x);
}

void vec2_set_mag(struct vec2 *v, float mag, struct vec2 *v_out) {
    float dir = vec2_dir(v);

    v_out->x = mag * pal_cos(dir);
    v_out->y = mag * pal_sin(dir);
}

void vec2_set_dir(struct vec2 *v, float dir, struct vec2 *v_out) {
    float mag = vec2_mag(v);

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

float vec2_dot(struct vec2 *v1, struct vec2 *v2) {
    return v1->x * v2->x + v1->y * v2->y;
}

float vec2_cross(struct vec2 *v1, struct vec2 *v2) {
    return v1->x * v2->y - v1->y * v2->x;
}

void vec2_transform(struct vec2 *v, struct mat2 *m, struct vec2 *v_out) {
    v_out->x = m->a * v->x + m->b * v->y;
    v_out->y = m->c * v->x + m->d * v->y;
}

void vec2_rotate(struct vec2 *v, float angle, struct vec2 *v_out) {
    // construct a rotation matrix and return transformed vector
    float cos_angle = pal_cos(angle);
    float sin_angle = pal_sin(angle);

    vec2_transform(v, &(struct mat2) {
        .a = cos_angle, .b = -sin_angle,
        .c = sin_angle, .d =  cos_angle,
    }, v_out);
}

void vec2_scale(struct vec2 *v, float scale, struct vec2 *v_out) {
    v_out->x = v->x * scale;
    v_out->y = v->y * scale;
}

void vec2_normalize(struct vec2 *v, struct vec2 *v_out) {
    float mag = vec2_mag(v);
    v_out->x = v->x / mag;
    v_out->y = v->y / mag;
}

float mat2_det(struct mat2 *m) {
    return m->a * m->d - m->b * m->c;
}

bool mat2_inv(struct mat2 *m, struct mat2 *m_inv) {
    float det = mat2_det(m);

    if (det == 0)
        return false;

    float det_inv = 1 / det;
    m_inv->a =  det_inv * m->d;
    m_inv->b = -det_inv * m->b;
    m_inv->c = -det_inv * m->c;
    m_inv->d =  det_inv * m->a;

    return true;
}