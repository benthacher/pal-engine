#pragma once

#include <stdint.h>
#include <stdlib.h>
#include "mathutils.h"

#define MAX_POLY_SIDES 10

enum bounds_type {
    BOUNDS_TYPE_CIRCLE,
    BOUNDS_TYPE_POLY,
};

struct bounds {
    enum bounds_type type;
    union {
        struct {
            struct vec2 vertices[MAX_POLY_SIDES];
            uint8_t n_vertices;
        };
        pal_float_t radius;
    };
    pal_float_t furthest_vertex_squared;
    pal_float_t furthest_vertex_distance;
    pal_float_t area;
};

struct phys_data {
    struct vec2 position;
    struct vec2 velocity;
    struct vec2 force;
    pal_float_t angle;
    pal_float_t angular_velocity;
    pal_float_t torque;
    pal_float_t elasticity;
    pal_float_t mass, inv_mass;
    pal_float_t moment_of_inertia, inv_moment_of_inertia;
    struct bounds bounds;
    struct bounds translated_bounds;
};

/**
 * @brief Struct describing collision
 *
 */
struct collision_descriptor {
    bool should_resolve;
    pal_float_t penitration_depth;
    struct phys_data *phys1;
    struct phys_data *phys2;
    struct vec2 normal;
    struct vec2 contact;
};

// void physics_resolve_collision(struct collision_manifold *collision);
/**
 * @brief Initializes physics data
 *
 * @param phys
 * @param mass
 */
void physics_init(struct phys_data *phys, pal_float_t mass);

/**
 * @brief Sets up phys_data to be polygonal
 *
 * @param phys
 * @param n_vertices
 * @param vertices
 */
void physics_set_bounds_poly(struct phys_data *phys, size_t n_vertices, struct vec2 *vertices);

/**
 * @brief Sets up phys_data to have circular bounds
 *
 * @param phys
 * @param radius
 */
void physics_set_bounds_circle(struct phys_data *phys, pal_float_t radius);

/**
 * @brief Sets up phys_data to be rectangular
 *
 * @param phys
 * @param width
 * @param height
 */
void physics_set_bounds_rect(struct phys_data *phys, pal_float_t width, pal_float_t height);

/**
 * @brief Integrates phys_data given time step (dt)
 *
 * @param phys
 * @param dt time step in seconds
 */
void physics_integrate(struct phys_data *phys, pal_float_t dt);

/**
 * @brief Scales phys_data bounds by given factor
 *
 * @param phys
 * @param factor
 */
void physics_scale_bounds(struct phys_data *phys, pal_float_t factor);

/**
 * @brief Computes translated bounds based on angle and position of phys_data
 *
 * @param phys
 */
void physics_compute_translated_bounds(struct phys_data *phys);

/**
 * @brief Checks if point is inside phys_data bounds
 *
 * @param phys
 * @param point
 * @return true
 * @return false
 */
bool physics_check_point_collision(struct phys_data *phys, struct vec2 *point);

/**
 * @brief Detects collision between two objects, filling in collision information if collision is detected
 *
 * @param phys1
 * @param phys2
 * @param collision
 * @return true
 * @return false
 */
bool physics_detect_collision(struct phys_data *phys1, struct phys_data *phys2, struct collision_descriptor *collision);

/**
 * @brief Resolve collision based on collision descriptor
 *
 * @param collision
 */
void physics_resolve_collision(struct collision_descriptor *collision);