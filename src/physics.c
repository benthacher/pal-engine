#include "physics.h"
#include <math.h>
#include <stdlib.h>

// normalized collision axes used in SAT algorithm. Maximum number of axes happens when two polygons with max sides collide
static struct vec2 collision_axes[MAX_POLY_SIDES * 2];
static int num_phys1_collision_axes;
static int num_phys2_collision_axes;

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

    phys->bounds.furthest_vertex_distance = sqrtf(phys->bounds.furthest_vertex_squared);
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
        phys->bounds.radius = phys->translated_bounds.radius = phys->bounds.radius * factor;
    }

    // now that the bounds are scaled, recompute area and inertia, also finding furthest vertex squared
    compute_area_and_inertia(phys);
    find_furthest_vertex_squared(phys);
}

void physics_compute_translated_bounds(struct phys_data *phys) {
    if (phys->bounds.type == BOUNDS_TYPE_CIRCLE)
        return;

    for (int i = 0; i < phys->bounds.n_vertices; i++) {
        vec2_rotate(&phys->bounds.vertices[i], phys->angle, &phys->translated_bounds.vertices[i]);
        vec2_add(&phys->position, &phys->translated_bounds.vertices[i], &phys->translated_bounds.vertices[i]);
    }
}

bool physics_check_point_collision(struct phys_data *phys, struct vec2 *point) {
    struct vec2 distance_vec;
    vec2_sub(point, &phys->position, &distance_vec);

    // if the point isn't within the maximum vertex, just return false
    if (vec2_squared_mag(&distance_vec) > phys->bounds.furthest_vertex_squared)
        return false;

    if (phys->bounds.type == BOUNDS_TYPE_POLY) {
        struct vec2 p1, p2, p3;
        int pos = 0;
        int neg = 0;

        for (int i = 0; i < phys->translated_bounds.n_vertices; i++) {
            vec2_sub(point, &phys->translated_bounds.vertices[i], &p3);
            vec2_sub(&phys->translated_bounds.vertices[(i + 1) % phys->bounds.n_vertices], &phys->translated_bounds.vertices[i], &p2);

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

static void closest_vertex_to_point(struct phys_data *phys, struct vec2 *point, struct vec2 *closest) {
    struct vec2 *closest_ptr = &phys->translated_bounds.vertices[0];
    struct vec2 distance_vector;
    float distance;
    float min = INFINITY;

    for (int i = 0; i < phys->translated_bounds.n_vertices; i++) {
        vec2_sub(&phys->translated_bounds.vertices[i], point, &distance_vector);
        distance = vec2_squared_mag(&distance_vector);
        if (distance < min) {
            closest_ptr = &phys->translated_bounds.vertices[i];
            min = distance;
        }
    }

    *closest = *closest_ptr;
}

static void get_sat_axes(struct phys_data *phys1, struct phys_data *phys2) {
    num_phys1_collision_axes = num_phys2_collision_axes = 0;
    struct vec2 *axis;

    // If both phys_datas are circles, we just need the normalized difference vector as an axis
    if (phys1->translated_bounds.type == BOUNDS_TYPE_CIRCLE && phys2->translated_bounds.type == BOUNDS_TYPE_CIRCLE) {
        vec2_sub(&phys2->position, &phys1->position, &collision_axes[0]);
        vec2_normalize(&collision_axes[0], &collision_axes[0]);
        num_phys1_collision_axes = 1;
    // If phys1 is a circle, get the closest point of phys2's translated_bounds and get an axis from it
    // From phys2's normal_angles, make a set of normal vectors and add them to axes
    } else if (phys1->translated_bounds.type == BOUNDS_TYPE_CIRCLE) {
        // axis from closest point of phys2's translated_bounds to phys1
        axis = &collision_axes[0];
        closest_vertex_to_point(phys2, &phys1->position, axis);
        vec2_sub(axis, &phys1->position, axis);
        vec2_normalize(axis, axis);
        num_phys1_collision_axes = 1;

        for (int i = 0; i < phys2->translated_bounds.n_vertices; i++) {
            // get edge
            struct vec2 *v1 = &phys2->translated_bounds.vertices[i];
            struct vec2 *v2 = &phys2->translated_bounds.vertices[(i + 1) % phys2->translated_bounds.n_vertices];
            axis = &collision_axes[num_phys1_collision_axes + num_phys2_collision_axes];

            // axis = v1 - v2
            vec2_sub(v1, v2, axis);
            vec2_normalize(axis, axis);
            // get vector normal to axis
            float axis_x = axis->x;
            axis->x = -axis->y;
            axis->y = axis_x;

            num_phys2_collision_axes++;
        }
    } else if (phys2->translated_bounds.type == BOUNDS_TYPE_CIRCLE) {
        for (int i = 0; i < phys1->translated_bounds.n_vertices; i++) {
            // get edge
            struct vec2 *v1 = &phys1->translated_bounds.vertices[i];
            struct vec2 *v2 = &phys1->translated_bounds.vertices[(i + 1) % phys1->translated_bounds.n_vertices];
            axis = &collision_axes[num_phys1_collision_axes];

            // axis = v1 - v2
            vec2_sub(v1, v2, axis);
            vec2_normalize(axis, axis);
            // get vector normal to axis
            float axis_x = axis->x;
            axis->x = -axis->y;
            axis->y = axis_x;

            num_phys1_collision_axes++;
        }

        // axis from closest point of phys2's translated_bounds to phys1
        axis = &collision_axes[num_phys1_collision_axes];
        closest_vertex_to_point(phys1, &phys2->position, axis);
        vec2_sub(axis, &phys2->position, axis);
        vec2_normalize(axis, axis);
        num_phys2_collision_axes = 1;
    } else { // both objects are polys
        for (int i = 0; i < phys1->translated_bounds.n_vertices; i++) {
            // get edge
            struct vec2 *v1 = &phys1->translated_bounds.vertices[i];
            struct vec2 *v2 = &phys1->translated_bounds.vertices[(i + 1) % phys1->translated_bounds.n_vertices];
            axis = &collision_axes[num_phys1_collision_axes];

            // axis = v1 - v2
            vec2_sub(v1, v2, axis);
            vec2_normalize(axis, axis);
            // get vector normal to axis
            float axis_x = axis->x;
            axis->x = -axis->y;
            axis->y = axis_x;

            num_phys1_collision_axes++;
        }
        for (int i = 0; i < phys2->translated_bounds.n_vertices; i++) {
            // get edge
            struct vec2 *v1 = &phys2->translated_bounds.vertices[i];
            struct vec2 *v2 = &phys2->translated_bounds.vertices[(i + 1) % phys2->translated_bounds.n_vertices];
            axis = &collision_axes[num_phys1_collision_axes + num_phys2_collision_axes];

            // axis = v1 - v2
            vec2_sub(v1, v2, axis);
            vec2_normalize(axis, axis);
            // get vector normal to axis
            float axis_x = axis->x;
            axis->x = -axis->y;
            axis->y = axis_x;

            num_phys2_collision_axes++;
        }
    }
}

struct projection {
    float min;
    float max;
    struct vec2 collision_point;
};

static void project_phys_data(struct vec2 *axis, struct phys_data *phys, struct projection *projection) {
    float proj;

    if (phys->translated_bounds.type == BOUNDS_TYPE_CIRCLE) {
        float r = phys->translated_bounds.radius;
        float pos_proj = vec2_dot(axis, &phys->position);

        projection->min = pos_proj - r;
        projection->max = pos_proj + r;
        vec2_scale(axis, -r, &projection->collision_point);
        vec2_add(&phys->position, &projection->collision_point, &projection->collision_point);
    } else {
        projection->min = vec2_dot(axis, &phys->translated_bounds.vertices[0]);
        projection->max = projection->min;
        projection->collision_point = phys->translated_bounds.vertices[0];

        for (int i = 1; i < phys->translated_bounds.n_vertices; i++) {
            proj = vec2_dot(axis, &phys->translated_bounds.vertices[i]);

            if (proj < projection->min) {
                projection->min = proj;
                projection->collision_point = phys->translated_bounds.vertices[i];
            }

            if (proj > projection->max)
                projection->max = proj;
        }
    }
}

static inline bool aabb_collision(float x1, float y1, float width1, float height1, float x2, float y2, float width2, float height2) {
    return (x1 < x2 + width2  &&
            x1 + width1 > x2  &&
            y1 < y2 + height2 &&
            y1 + height1 > y2);
}

bool physics_detect_collision(struct phys_data *phys1, struct phys_data *phys2, struct collision_descriptor *collision) {
    float overlap;
    struct vec2 *smallest_axis;
    struct vec2 *axis;
    struct phys_data *vertex_obj;
    struct projection proj1, proj2, contact_vertex;

    collision->phys1 = phys1;
    collision->phys2 = phys2;
    collision->penitration_depth = INFINITY;

    if (!aabb_collision(phys1->position.x, phys1->position.y, phys1->bounds.furthest_vertex_distance * 2, phys1->bounds.furthest_vertex_distance * 2,
                        phys2->position.x, phys2->position.y, phys2->bounds.furthest_vertex_distance * 2, phys2->bounds.furthest_vertex_distance * 2))
        return false;

    get_sat_axes(phys1, phys2);

    for (int i = 0; i < num_phys1_collision_axes + num_phys2_collision_axes; i++) {
        axis = &collision_axes[i];

        project_phys_data(axis, phys1, &proj1);
        project_phys_data(axis, phys2, &proj2);

        overlap = fmin(proj1.max, proj2.max) - fmax(proj1.min, proj2.min);

        if (overlap < 0)
            return false;

        if ((proj1.max > proj2.max && proj1.min < proj2.min) ||
            (proj1.max < proj2.max && proj1.min > proj2.min)) {

            float mins = fabs(proj1.min - proj2.min);
            float maxs = fabs(proj1.max - proj2.max);

            if (mins < maxs) {
                overlap += mins;
            } else {
                overlap += maxs;
                vec2_scale(axis, -1, axis);
            }
        }

        if (overlap < collision->penitration_depth) {
            collision->penitration_depth = overlap;
            smallest_axis = axis;

            if (i < num_phys1_collision_axes) {
                vertex_obj = phys2;
                if (proj1.max > proj2.max) {
                    vec2_scale(axis, -1, axis);
                    smallest_axis = axis;
                }
            } else {
                vertex_obj = phys1;
                if (proj1.max < proj2.max) {
                    vec2_scale(axis, -1, axis);
                    smallest_axis = axis;
                }
            }
        }
    }

    project_phys_data(smallest_axis, vertex_obj, &contact_vertex);

    if (vertex_obj == phys2)
        vec2_scale(smallest_axis, -1, smallest_axis);

    collision->normal = *smallest_axis;
    collision->contact = contact_vertex.collision_point;

    return true;
}

void physics_resolve_collision(struct collision_descriptor *collision) {
    struct vec2 resolution_dist1, resolution_dist2;

    float resolution_dist_mag = collision->penitration_depth / (collision->phys1->inv_mass + collision->phys2->inv_mass);
    float resolution_dist1_mag = resolution_dist_mag * collision->phys1->inv_mass;
    float resolution_dist2_mag = -resolution_dist_mag * collision->phys2->inv_mass;

    vec2_scale(&collision->normal, resolution_dist1_mag, &resolution_dist1);
    vec2_scale(&collision->normal, resolution_dist2_mag, &resolution_dist2);

    vec2_add(&collision->phys1->position, &resolution_dist1, &collision->phys1->position);
    vec2_add(&collision->phys2->position, &resolution_dist2, &collision->phys2->position);

    struct vec2 collision_arm1, collision_arm2;
    struct vec2 closing_vel1, closing_vel2;

    // Closing velocity
    vec2_sub(&collision->contact, &collision->phys1->position, &collision_arm1);
    closing_vel1.x = -collision_arm1.y;
    closing_vel1.y = collision_arm1.x;
    vec2_scale(&closing_vel1, collision->phys1->angular_velocity, &closing_vel1);
    vec2_add(&collision->phys1->velocity, &closing_vel1, &closing_vel1);

    vec2_sub(&collision->contact, &collision->phys2->position, &collision_arm2);
    closing_vel2.x = -collision_arm2.y;
    closing_vel2.y = collision_arm2.x;
    vec2_scale(&closing_vel2, collision->phys2->angular_velocity, &closing_vel2);
    vec2_add(&collision->phys2->velocity, &closing_vel2, &closing_vel2);

    // Impulse augmentation
    float collision_arm1_cross_normal = vec2_cross(&collision_arm1, &collision->normal);
    float impulse_aug1 = collision->phys1->inv_moment_of_inertia * collision_arm1_cross_normal * collision_arm1_cross_normal;
    float collision_arm2_cross_normal = vec2_cross(&collision_arm2, &collision->normal);
    float impulse_aug2 = collision->phys2->inv_moment_of_inertia * collision_arm2_cross_normal * collision_arm2_cross_normal;

    struct vec2 relative_velocity;
    vec2_sub(&closing_vel1, &closing_vel2, &relative_velocity);

    float separation_velocity = vec2_dot(&relative_velocity, &collision->normal);
    float new_separation_velocity = -separation_velocity * fmin(1.0, 1.0); // elasticities
    float separation_velocity_diff = new_separation_velocity - separation_velocity;
    //2. Impulse augmentation

    float impulse = separation_velocity_diff / (collision->phys1->inv_mass + collision->phys2->inv_mass + impulse_aug1 + impulse_aug2);

    struct vec2 impulse_vector;
    vec2_scale(&collision->normal, impulse, &impulse_vector);

    struct vec2 impulse_vec1, impulse_vec2;

    //3. Changing the velocities
    vec2_scale(&impulse_vector, collision->phys1->inv_mass, &impulse_vec1);
    vec2_scale(&impulse_vector, -collision->phys2->inv_mass, &impulse_vec2);

    vec2_add(&collision->phys1->velocity, &impulse_vec1, &collision->phys1->velocity);
    vec2_add(&collision->phys2->velocity, &impulse_vec2, &collision->phys2->velocity);

    collision->phys1->angular_velocity += collision->phys1->inv_moment_of_inertia * vec2_cross(&collision_arm1, &impulse_vector);
    collision->phys2->angular_velocity -= collision->phys2->inv_moment_of_inertia * vec2_cross(&collision_arm2, &impulse_vector);
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
    phys->bounds.type = phys->translated_bounds.type = BOUNDS_TYPE_CIRCLE;
    phys->bounds.radius = phys->translated_bounds.radius = radius;

    // initialize things like area and inertia now that the bounds are set
    compute_area_and_inertia(phys);
    find_furthest_vertex_squared(phys);
}

void physics_set_bounds_poly(struct phys_data *phys, size_t n_vertices, struct vec2 *vertices) {
    struct vec2 *vert;

    phys->bounds.type = phys->translated_bounds.type = BOUNDS_TYPE_POLY;
    phys->bounds.n_vertices = phys->translated_bounds.n_vertices = n_vertices > MAX_POLY_SIDES ? MAX_POLY_SIDES : n_vertices;

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
