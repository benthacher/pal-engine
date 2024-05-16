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
        float radius;
    };
    float furthest_vertex_squared;
    float area;
};

struct phys_data {
    struct vec2 position;
    struct vec2 velocity;
    struct vec2 force;
    float angle;
    float angular_velocity;
    float torque;
    float mass, inv_mass;
    float moment_of_inertia, inv_moment_of_inertia;
    struct bounds bounds;
};

/**
 * @brief Struct describing collision
 *
 */
struct collision_manifold {
    struct phys_data *phys1;
    struct phys_data *phys2;
    struct vec2 *normal;
    float penitration_depth;
    struct vec2 *contact;
};

// void physics_resolve_collision(struct collision_manifold *collision);
void physics_init(struct phys_data *phys, float mass);
void physics_set_bounds_poly(struct phys_data *phys, size_t n_vertices, struct vec2 *vertices);
void physics_set_bounds_circle(struct phys_data *phys, float radius);
void physics_set_bounds_rect(struct phys_data *phys, float width, float height);
void physics_integrate(struct phys_data *phys, float dt);
void physics_scale_bounds(struct phys_data *phys, float factor);
bool physics_check_point_collision(struct phys_data *phys, struct vec2 *point);