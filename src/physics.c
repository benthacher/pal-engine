#include "physics.h"
#include <math.h>
#include <stdlib.h>

static void find_furthest_vertex_squared(struct phys_data *phys) {
    if (phys->bounds.type == BOUNDS_TYPE_POLY) {
        float dist_squared;
        phys->bounds.furthest_vertex_squared = -1;

        for (int i = 0; i < phys->bounds.n_vertices; i++) {
            dist_squared = phys->bounds.vertices[i].x * phys->bounds.vertices[i].x + phys->bounds.vertices[i].y * phys->bounds.vertices[i].y;

            if (dist_squared > phys->bounds.furthest_vertex_squared)
                phys->bounds.furthest_vertex_squared = dist_squared;
        }
    } else if (phys->bounds.type == BOUNDS_TYPE_CIRCLE) {
        phys->bounds.furthest_vertex_squared = phys->bounds.radius * phys->bounds.radius;
    }
}

static void compute_area_and_inertia(struct phys_data *phys) {
    // calculate area and inertia in one shot
    float inertia = 0.0;

    if (phys->bounds.type == BOUNDS_TYPE_POLY) {
        float p1_dir, p1_mag;
        struct vec2 *p1;
        struct vec2 *p2;
        struct vec2 p2_rotated;

        for (int i = 0; i < phys->bounds.n_vertices; i++) {
            p1 = &phys->bounds.vertices[i];
            p2 = &phys->bounds.vertices[(i + 1) % phys->bounds.n_vertices];

            p1_dir = vec2_dir(p1);
            p1_mag = vec2_mag(p1);

            vec2_rotate(p2, -p1_dir, &p2_rotated);

            float b = p1_mag;
            float h = p2_rotated.y;
            float a = p2_rotated.x;

            inertia += (b * (h * h * h)) / 12 + ((b * b * b) * h + (b * b) * h * a + b * h * (a * a)) / 12;
            phys->bounds.area += vec2_mag(p2) * p1_mag * sin(vec2_dir(p2) - p1_dir) / 2;
        }
    } else if (phys->bounds.type == BOUNDS_TYPE_CIRCLE) {
        phys->bounds.area = M_PI * phys->bounds.radius * phys->bounds.radius;
        inertia = M_PI_2 * phys->bounds.radius * phys->bounds.radius * phys->bounds.radius * phys->bounds.radius;
    }

    phys->moment_of_inertia = inertia * phys->mass / phys->bounds.area;

    // compute inverse mass and inertia as they're used heavily in collision resolution
    phys->inv_mass = phys->mass > 0.0 ? 1.0 / phys->mass : INFINITY;
    phys->inv_moment_of_inertia = phys->moment_of_inertia > 0.0 ? 1.0 / phys->moment_of_inertia : INFINITY;
}

void physics_init(struct phys_data *phys, float mass) {
    phys->position.x = 0.0;
    phys->position.y = 0.0;
    phys->velocity.x = 0.0;
    phys->velocity.y = 0.0;
    phys->force.x = 0.0;
    phys->force.y = 0.0;
    phys->angle = 0.0;
    phys->angular_velocity = 0.0;
    phys->torque = 0.0;
    phys->bounds.area = 0.0;
    phys->mass = mass;
}

void physics_scale_bounds(struct phys_data *phys, float factor) {
    if (phys->bounds.type == BOUNDS_TYPE_POLY) {
        for (int i = 0; i < phys->bounds.n_vertices; i++) {
            vec2_scale(&phys->bounds.vertices[i], factor, &phys->bounds.vertices[i]);
        }
    } else if (phys->bounds.type == BOUNDS_TYPE_CIRCLE) {
        phys->bounds.radius *= factor;
    }

    // now that the bounds are scaled, recompute area and inertia, also finding furthest vertex squared
    compute_area_and_inertia(phys);
    find_furthest_vertex_squared(phys);
}

bool physics_check_point_collision(struct phys_data *phys, struct vec2 *point) {
    struct vec2 distance_vec;
    vec2_sub(point, &phys->position, &distance_vec);
    struct vec2 rotated_bounds[phys->bounds.n_vertices];

    // if the point isn't within the maximum vertex, just return false
    if (vec2_squared_mag(&distance_vec) > phys->bounds.furthest_vertex_squared)
        return false;

    if (phys->bounds.type == BOUNDS_TYPE_POLY) {
        struct vec2 p1, p2, p3;
        int pos = 0;
        int neg = 0;

        for (int i = 0; i < phys->bounds.n_vertices; i++) {
            vec2_rotate(&phys->bounds.vertices[i], phys->angle, &rotated_bounds[i]);
            vec2_rotate(&phys->bounds.vertices[(i + 1) % phys->bounds.n_vertices], phys->angle, &rotated_bounds[(i + 1) % phys->bounds.n_vertices]);

            vec2_add(&phys->position, &rotated_bounds[i], &p1);
            vec2_add(&phys->position, &rotated_bounds[(i + 1) % phys->bounds.n_vertices], &p2);

            vec2_sub(point, &p1, &p3);
            vec2_sub(&p2, &p1, &p2);

            float d = vec2_cross(&p3, &p2);

            if (d > 0) pos++;
            if (d < 0) neg++;

            //If the sign changes, then point is outside
            if (pos > 0 && neg > 0)
                return false;
        }

        return true;
    } else if (phys->bounds.type == BOUNDS_TYPE_CIRCLE) {
        // by this point we know the point we're testing is within the furthest point, and for a
        // circle, that means it's within the circle
        return true;
    }
}

void physics_resolve_collision(struct collision_manifold *collision) {
    // const penResolution = collision->normal.scale(collision->penitration_depth / (collision->phys1->inv_mass + collision->phys2->inv_mass));
    // collision->phys1->position.add(penResolution.copy().scale(collision->phys1->inv_mass));
    // collision->phys2->position.add(penResolution.scale(-collision->phys2->inv_mass));

    // //1. Closing velocity
    // const collArm1 = collision->contact.copy().subtract(collision->phys1->position);
    // const rotVel1  = collArm1.copy().perp().scale(collision->phys1->angVel);
    // const closVel1 = collision->phys1->vel.copy().add(rotVel1);
    // const collArm2 = collision->contact.copy().subtract(collision->phys2->position);
    // const rotVel2  = collArm2.copy().perp().scale(collision->phys2->angVel);
    // const closVel2 = collision->phys2->vel.copy().add(rotVel2);

    // //2. Impulse augmentation
    // const impAug1 = collision->phys1->inv_inertia * collArm1.cross(collision->normal) ** 2;
    // const impAug2 = collision->phys2->inv_inertia * collArm2.cross(collision->normal) ** 2;

    // const relativeVel = closVel1.subtract(closVel2);
    // const separationVel = relativeVel.dot(collision->normal);
    // const newSeparationVel = -separationVel * Math.min(collision->phys1->elasticity, collision->phys2->elasticity);
    // const vsep_diff = newSeparationVel - separationVel;

    // const impulse = vsep_diff / (collision->phys1->inv_mass + collision->phys2->inv_mass + impAug1 + impAug2);
    // const impulseVec = collision->normal.copy().scale(impulse);

    // //3. Changing the velocities
    // collision->phys1->vel.add(impulseVec.copy().scale(collision->phys1->inv_mass));
    // collision->phys2->vel.add(impulseVec.copy().scale(-collision->phys2->inv_mass));

    // collision->phys1->angVel += collision->phys1->inv_inertia * collArm1.cross(impulseVec);
    // collision->phys2->angVel -= collision->phys2->inv_inertia * collArm2.cross(impulseVec);
}

void physics_integrate(struct phys_data *phys, float dt) {
    struct vec2 vel_step, acc_step;

    vec2_scale(&phys->force, phys->inv_mass * dt, &acc_step);
    vec2_scale(&phys->velocity, dt, &vel_step);

    vec2_add(&phys->velocity, &acc_step, &phys->velocity);
    vec2_add(&phys->position, &vel_step, &phys->position);

    phys->angular_velocity += phys->torque * phys->inv_moment_of_inertia * dt;
    phys->angle += phys->angular_velocity * dt;
}

void physics_set_bounds_circle(struct phys_data *phys, float radius) {
    phys->bounds.type = BOUNDS_TYPE_CIRCLE;
    phys->bounds.radius = radius;

    // initialize things like area and inertia now that the bounds are set
    compute_area_and_inertia(phys);
    find_furthest_vertex_squared(phys);
}

void physics_set_bounds_poly(struct phys_data *phys, size_t n_vertices, struct vec2 *vertices) {
    struct vec2 *vert;

    phys->bounds.type = BOUNDS_TYPE_POLY;
    phys->bounds.n_vertices = n_vertices > MAX_POLY_SIDES ? MAX_POLY_SIDES : n_vertices;

    for (int i = 0; i < phys->bounds.n_vertices; i++) {
        phys->bounds.vertices[i].x = vertices[i].x;
        phys->bounds.vertices[i].y = vertices[i].y;
    }

    // initialize things like area and inertia now that the bounds are set
    compute_area_and_inertia(phys);
    find_furthest_vertex_squared(phys);
}

void physics_set_bounds_rect(struct phys_data *phys, float width, float height) {
    struct vec2 verts[4] = {
        {  width / 2,  height / 2 },
        { -width / 2,  height / 2 },
        { -width / 2, -height / 2 },
        {  width / 2, -height / 2 }
    };

    physics_set_bounds_poly(phys, 4, verts);
}
