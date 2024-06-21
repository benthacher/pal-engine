#pragma once

#include <stdbool.h>
#include "pal.h"

/**
 * @brief Linear interpolation
 *
 * @param x1
 * @param x2
 * @param t
 * @return pal_float_t
 */
pal_float_t lerp(pal_float_t x1, pal_float_t x2, pal_float_t t);

/**
 * @brief Magnitude of vector
 *
 * @param v
 * @return pal_float_t
 */
pal_float_t vec2_mag(struct vec2 *v);

/**
 * @brief Computes the squared magnitude of vector
 *
 * @param v
 * @return pal_float_t
 */
pal_float_t vec2_squared_mag(struct vec2 *v);

/**
 * @brief Direction of vector
 *
 * @param v
 * @return pal_float_t
 */
pal_float_t vec2_dir(struct vec2 *v);

/**
 * @brief Sets magnitude of vector
 *
 * @param v
 * @param mag
 * @param v_out
 */
void vec2_set_mag(struct vec2 *v, pal_float_t mag, struct vec2 *v_out);

/**
 * @brief Sets direction of vector
 *
 * @param v
 * @param dir
 * @param v_out
 */
void vec2_set_dir(struct vec2 *v, pal_float_t dir, struct vec2 *v_out);

/**
 * @brief Adds two vectors
 *
 * @param v1
 * @param v2
 * @param sum
 */
void vec2_add(struct vec2 *v1, struct vec2 *v2, struct vec2 *sum);

/**
 * @brief Subtracts two vectors
 *
 * @param v1
 * @param v2
 * @param diff difference of vectors
 */
void vec2_sub(struct vec2 *v1, struct vec2 *v2, struct vec2 *diff);

/**
 * @brief Linear interpolation between two vectors
 *
 * @param v1
 * @param v2
 * @param t
 * @param v_out interpolated vector
 */
void vec2_lerp(struct vec2 *v1, struct vec2 *v2, pal_float_t t, struct vec2 *v_out);

/**
 * @brief Computes the dot product of two vectors
 *
 * @param v1
 * @param v2
 * @return pal_float_t dot product
 */
pal_float_t vec2_dot(struct vec2 *v1, struct vec2 *v2);

/**
 * @brief Computes the 2d cross product between two vectors (z components = 1)
 *
 * @param v1
 * @param v2
 * @return pal_float_t
 */
pal_float_t vec2_cross(struct vec2 *v1, struct vec2 *v2);

/**
 * @brief Perform 2D transformation with matrix m on vector v, returning a new vector
 *
 * @param v
 * @param m
 * @param v_out transformed vector
 */
void vec2_transform(struct vec2 *v, struct mat2 *m, struct vec2 *v_out);

/**
 * @brief Rotate vector v by angle, return rotated vector
 *
 * @param v
 * @param angle
 * @param v_out transformed vector
 */
void vec2_rotate(struct vec2 *v, pal_float_t angle, struct vec2 *v_out);

/**
 * @brief Scale vector v by scale, return scaled vector
 *
 * @param v
 * @param scale
 * @param v_out transformed vector
 */
void vec2_scale(struct vec2 *v, pal_float_t scale, struct vec2 *v_out);

/**
 * @brief Normalizes vector
 *
 * @param v
 * @param v_out
 */
void vec2_normalize(struct vec2 *v, struct vec2 *v_out);

/**
 * @brief Determinant of matrix
 *
 * @param m
 * @return pal_float_t
 */
pal_float_t mat2_det(struct mat2 *m);

/**
 * @brief Computes the inverse of given matrix m
 *
 * @param m
 * @param m_inv
 * @return bool true if matrix is able to be inverted and false if not
 */
bool mat2_inv(struct mat2 *m, struct mat2 *m_inv);