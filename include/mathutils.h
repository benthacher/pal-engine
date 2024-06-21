#pragma once

#include <stdbool.h>

/**
 * @brief 2D Vector
 *
 */
struct vec2 {
    float x;
    float y;
};

/**
 * @brief 3D Vector
 *
 */
struct vec3 {
    float x;
    float y;
    float z;
};

/**
 * @brief 2D Matrix
 *
 */
struct mat2 {
    float a;
    float b;
    float c;
    float d;
};

/**
 * @brief Linear interpolation
 *
 * @param x1
 * @param x2
 * @param t
 * @return float
 */
float lerp(float x1, float x2, float t);

/**
 * @brief Magnitude of vector
 *
 * @param v
 * @return float
 */
float vec2_mag(struct vec2 *v);

/**
 * @brief Computes the squared magnitude of vector
 *
 * @param v
 * @return float
 */
float vec2_squared_mag(struct vec2 *v);

/**
 * @brief Direction of vector
 *
 * @param v
 * @return float
 */
float vec2_dir(struct vec2 *v);

/**
 * @brief Sets magnitude of vector
 *
 * @param v
 * @param mag
 * @param v_out
 */
void vec2_set_mag(struct vec2 *v, float mag, struct vec2 *v_out);

/**
 * @brief Sets direction of vector
 *
 * @param v
 * @param dir
 * @param v_out
 */
void vec2_set_dir(struct vec2 *v, float dir, struct vec2 *v_out);

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
void vec2_lerp(struct vec2 *v1, struct vec2 *v2, float t, struct vec2 *v_out);

/**
 * @brief Computes the dot product of two vectors
 *
 * @param v1
 * @param v2
 * @return float dot product
 */
float vec2_dot(struct vec2 *v1, struct vec2 *v2);

/**
 * @brief Computes the 2d cross product between two vectors (z components = 1)
 *
 * @param v1
 * @param v2
 * @return float
 */
float vec2_cross(struct vec2 *v1, struct vec2 *v2);

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
void vec2_rotate(struct vec2 *v, float angle, struct vec2 *v_out);

/**
 * @brief Scale vector v by scale, return scaled vector
 *
 * @param v
 * @param scale
 * @param v_out transformed vector
 */
void vec2_scale(struct vec2 *v, float scale, struct vec2 *v_out);

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
 * @return float
 */
float mat2_det(struct mat2 *m);

/**
 * @brief Computes the inverse of given matrix m
 *
 * @param m
 * @param m_inv
 */
bool mat2_inv(struct mat2 *m, struct mat2 *m_inv);