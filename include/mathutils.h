#pragma once

#include <stdbool.h>
#include "pal.h"

/**
 * @brief Absolute value function for integers
 *
 * @param num
 * @return int
 */
int pal_abs(int num);

/**
 * @brief Absolute value function for floats
 *
 * @param num
 * @return pal_float_t
 */
pal_float_t pal_fabs(pal_float_t num);

/**
 * @brief Floor function
 *
 * @param num
 * @return pal_float_t
 */
pal_float_t pal_floor(pal_float_t num);

/**
 * @brief Ceiling function
 *
 * @param num
 * @return pal_float_t
 */
pal_float_t pal_ceil(pal_float_t num);

/**
 * @brief Round function
 *
 * @param num
 * @return pal_float_t
 */
pal_float_t pal_round(pal_float_t num);

/**
 * @brief Minimum of two floats
 *
 * @param a
 * @param b
 * @return pal_float_t
 */
pal_float_t pal_fmin(pal_float_t a, pal_float_t b);

/**
 * @brief Maximum of two floats
 *
 * @param a
 * @param b
 * @return pal_float_t
 */
pal_float_t pal_fmax(pal_float_t a, pal_float_t b);

/**
 * @brief Minimum of two ints
 *
 * @param a
 * @param b
 * @return int
 */
int pal_min(int a, int b);

/**
 * @brief Maximum of two ints
 *
 * @param a
 * @param b
 * @return int
 */
int pal_max(int a, int b);

/**
 * @brief Separate a pal_float_t into integral and fractional parts
 *
 * @param num Number to get parts of
 * @param integral pointer to location to return integral part of num
 * @return pal_float_t fractional part
 */
pal_float_t pal_modf(pal_float_t num, pal_float_t *integral);

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
pal_float_t vec2_mag(const struct vec2 *v);

/**
 * @brief Computes the squared magnitude of vector
 *
 * @param v
 * @return pal_float_t
 */
pal_float_t vec2_squared_mag(const struct vec2 *v);

/**
 * @brief Direction of vector
 *
 * @param v
 * @return pal_float_t
 */
pal_float_t vec2_dir(const struct vec2 *v);

/**
 * @brief Sets magnitude of vector
 *
 * @param v
 * @param mag
 * @param v_out
 */
void vec2_set_mag(const struct vec2 *v, pal_float_t mag, struct vec2 *v_out);

/**
 * @brief Sets direction of vector
 *
 * @param v
 * @param dir
 * @param v_out
 */
void vec2_set_dir(const struct vec2 *v, pal_float_t dir, struct vec2 *v_out);

/**
 * @brief Adds two vectors
 *
 * @param v1
 * @param v2
 * @param sum
 */
void vec2_add(const struct vec2 *v1, const struct vec2 *v2, struct vec2 *sum);

/**
 * @brief Subtracts two vectors
 *
 * @param v1
 * @param v2
 * @param diff difference of vectors
 */
void vec2_sub(const struct vec2 *v1, const struct vec2 *v2, struct vec2 *diff);

/**
 * @brief Linear interpolation between two vectors
 *
 * @param v1
 * @param v2
 * @param t
 * @param v_out interpolated vector
 */
void vec2_lerp(const struct vec2 *v1, const struct vec2 *v2, pal_float_t t, struct vec2 *v_out);

/**
 * @brief Computes the dot product of two vectors
 *
 * @param v1
 * @param v2
 * @return pal_float_t dot product
 */
pal_float_t vec2_dot(const struct vec2 *v1, const struct vec2 *v2);

/**
 * @brief Computes the 2d cross product between two vectors (z components = 1)
 *
 * @param v1
 * @param v2
 * @return pal_float_t
 */
pal_float_t vec2_cross(const struct vec2 *v1, const struct vec2 *v2);

/**
 * @brief Perform 2D transformation with matrix m on vector v, returning a new vector
 *
 * @param v
 * @param m
 * @param v_out transformed vector
 */
void vec2_transform(const struct vec2 *v, const struct mat2 *m, struct vec2 *v_out);

/**
 * @brief Rotate vector v by angle, return rotated vector
 *
 * @param v
 * @param angle
 * @param v_out transformed vector
 */
void vec2_rotate(const struct vec2 *v, pal_float_t angle, struct vec2 *v_out);

/**
 * @brief Scale vector v by scale, return scaled vector
 *
 * @param v
 * @param scale
 * @param v_out transformed vector
 */
void vec2_scale(const struct vec2 *v, pal_float_t scale, struct vec2 *v_out);

/**
 * @brief Normalizes vector
 *
 * @param v
 * @param v_out
 */
void vec2_normalize(const struct vec2 *v, struct vec2 *v_out);

/**
 * @brief Determinant of matrix
 *
 * @param m
 * @return pal_float_t
 */
pal_float_t mat2_det(const struct mat2 *m);

/**
 * @brief Multiplies 2 matrices together
 *
 * @param m1
 * @param m2
 * @param product
 */
void mat2_multiply(const struct mat2 *m1, const struct mat2 *m2, struct mat2 *product);

/**
 * @brief Computes the inverse of given matrix m
 *
 * @param m
 * @param m_inv
 * @return bool true if matrix is able to be inverted and false if not
 */
bool mat2_inv(const struct mat2 *m, struct mat2 *m_inv);

/**
 * @brief Returns random number in range [0, 1)
 * (not including 1)
 *
 * @return pal_float_t
 */
pal_float_t rand_float();

/**
 * @brief Returns random number in range [min, max)
 * (not including max)
 *
 * @param min
 * @param max
 * @return pal_float_t
 */
pal_float_t rand_float_range(pal_float_t min, pal_float_t max);

/**
 * @brief Returns random integer in range [min, max)
 * (not including max)
 *
 * @param min
 * @param max
 * @return int
 */
int rand_range(int min, int max);