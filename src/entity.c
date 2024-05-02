#include "entity.h"

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "graphics.h"
#include "physics.h"
#include "sprite.h"

static struct entity_config_flags default_config_flags = {
    .do_physics = 1
};

void entity_set_sprite_def(struct entity *entity, struct sprite_def *sprite_def) {
    if (entity->type != ENTITY_DRAW_TYPE_SPRITE)
        return;

    // set sprite def to given sprite def then reset
    entity->sprite.sprite_def = sprite_def;
    sprite_init(&entity->sprite, sprite_def);
}

void entity_set_config(struct entity *entity, struct entity_config_flags config) {
    entity->config_flags = config;
}

void entity_init(struct entity *entity, float mass) {
    // make sure event handlers are null
    for (int i = 0; i < NUM_ENTITY_EVENTS; i++) {
        entity->_event_handlers[i] = NULL;
    }

    entity_set_config(entity, default_config_flags);

    physics_init(&entity->phys, mass);
}

void entity_set_draw_type(struct entity *entity, enum entity_draw_type type, ...) {
    va_list type_data;

    entity->type = type;

    // depending on type, variadic args mean different things
    switch (entity->type) {
        case ENTITY_DRAW_TYPE_INVISIBLE:
            break;
        case ENTITY_DRAW_TYPE_SIMPLE:
        case ENTITY_DRAW_TYPE_SIMPLE_OUTLINE:
            // treat first va arg as color
            va_start(type_data, type);
            entity->color = va_arg(type_data, struct color);
            va_end(type_data);

            break;
        case ENTITY_DRAW_TYPE_SPRITE:
            // treat first va arg as sprite def pointer
            va_start(type_data, type);
            entity->scale = 1.0;
            struct sprite_def *sprite_def = va_arg(type_data, struct sprite_def *);
            sprite_init(&entity->sprite, sprite_def);
            va_end(type_data);

            break;
    }
}

void entity_set_bounds(struct entity *entity, enum entity_bounds_type type, ...) {
    va_list bounds_args;

    entity->phys.bounds.type = type;

    // deal with bounds type specific data
    va_start(bounds_args, type);

    switch (entity->phys.bounds.type) {
        case ENTITY_BOUNDS_TYPE_POLY: {
                size_t n_vertices = va_arg(bounds_args, size_t);

                struct vec2 *vert;
                struct vec2 verts[n_vertices];

                if (n_vertices > MAX_POLY_SIDES)
                    n_vertices = MAX_POLY_SIDES;

                for (int i = 0; i < n_vertices; i++) {
                    // get vec2 pointer and copy x and y to vertex at i
                    vert = va_arg(bounds_args, struct vec2 *);

                    verts[i].x = vert->x;
                    verts[i].y = vert->y;
                }

                physics_set_bounds_poly(&entity->phys, n_vertices, verts);
            }
            break;
        case ENTITY_BOUNDS_TYPE_CIRCLE: {
            double radius = va_arg(bounds_args, double);
            physics_set_bounds_circle(&entity->phys, radius);
            break;
        }
        case ENTITY_BOUNDS_TYPE_RECTANGLE:
            double width = va_arg(bounds_args, double);
            double height = va_arg(bounds_args, double);
            physics_set_bounds_rect(&entity->phys, width, height);
            break;
    }

    va_end(bounds_args);
}

void entity_attach_handler(struct entity *entity, enum entity_event event, entity_event_handler_t handler) {
    if (event > NUM_ENTITY_EVENTS)
        return;

    entity->_event_handlers[event] = handler;
}

void entity_event_emit(struct entity *entity, enum entity_event event, void *data, size_t data_len) {
    if (event > NUM_ENTITY_EVENTS)
        return;

    if (data != NULL && data_len > 0) {
        // copy data to heap
        void *heap_data = malloc(data_len);
        memcpy(heap_data, data, data_len);
        entity->_event_data[event] = heap_data;
    } else {
        entity->_event_data[event] = NULL;
    }

    // set event flag
    entity->_event_flags |= (1 << event);
}

void entity_call_event_handler(struct entity *entity, enum entity_event event) {
    if (event > NUM_ENTITY_EVENTS)
        return;

    if (entity->_event_handlers[event] != NULL) {
        entity->_event_handlers[event](entity, entity->_event_data[event]);
        free(entity->_event_data[event]);
    }

    entity_event_clear(entity, event);
}

void entity_scale(struct entity *entity, float factor) {
    if (entity->type == ENTITY_DRAW_TYPE_SPRITE) {
        entity->scale *= factor;
    }

    physics_scale_bounds(&entity->phys, factor);
}

static inline bool edge_test(struct vec2 *v1, struct vec2 *v2, struct vec2 *v3) {
    return (v3->x - v1->x) * (v2->y - v1->y) - (v3->y - v1->y) * (v2->x - v1->x) <= 0;
}

static void render_triangle(struct vec2 *v1, struct vec2 *v2, struct vec2 *v3, struct color color) {
    // get start and end point for looping through screen pixels
    struct vec2 start = { floor(fmin(v1->x, fmin(v2->x, v3->x))), floor(fmin(v1->y, fmin(v2->y, v3->y))) } ;
    struct vec2 end =   { fmax(v1->x, fmax(v2->x, v3->x)), fmax(v1->y, fmax(v2->y, v3->y)) } ;

    struct vec2 p = start;
    for (p.y = start.y; p.y < end.y; p.y++) {
        for (p.x = start.x; p.x < end.x; p.x++) {
            if (edge_test(v1, v2, &p) && edge_test(v2, v3, &p) && edge_test(v3, v1, &p))
                pal_screen_draw_pixel(p.x, p.y, color);
        }
    }
}

static void bounds_render_filled(int x, int y, struct bounds *bounds, float angle, struct color color) {
    if (bounds->type == BOUNDS_TYPE_POLY) {
        struct vec2 p1, p2;
        struct vec2 center = { x, y };
        for (int i = 0; i < bounds->n_vertices; i++) {
            // rotate verticies
            vec2_rotate(&bounds->vertices[i], angle, &p1);
            vec2_rotate(&bounds->vertices[(i + 1) % bounds->n_vertices], angle, &p2);

            // translate vertices to position on screen
            vec2_add(&center, &p1, &p1);
            vec2_add(&center, &p2, &p2);

            render_triangle(&center, &p1, &p2, color);
        }
    } else if (bounds->type == BOUNDS_TYPE_CIRCLE) {
        graphics_draw_circle(x, y, bounds->radius, color);
    }
}

static void bounds_render_stroked(int x, int y, struct bounds *bounds, float angle, struct color color) {
    if (bounds->type == BOUNDS_TYPE_POLY) {
        struct vec2 p1, p2;
        struct vec2 center = { x, y };
        for (int i = 0; i < bounds->n_vertices; i++) {
            // rotate verticies
            vec2_rotate(&bounds->vertices[i], angle, &p1);
            vec2_rotate(&bounds->vertices[(i + 1) % bounds->n_vertices], angle, &p2);

            // translate vertices to position on screen
            vec2_add(&center, &p1, &p1);
            vec2_add(&center, &p2, &p2);

            // draw line from p1 to p2
            graphics_draw_line(p1.x, p1.y, p2.x, p2.y, color);
        }
    } else if (bounds->type == BOUNDS_TYPE_CIRCLE) {
        graphics_stroke_circle(x, y, bounds->radius, color, 1);
    }
}

void entity_render(struct entity *entity) {
    // draw entity at physical position
    int draw_x = round(entity->phys.position.x);
    int draw_y = round(entity->phys.position.y);

    switch (entity->type) {
        case ENTITY_DRAW_TYPE_SIMPLE:
            // fill in bounds
            bounds_render_filled(draw_x, draw_y, &entity->phys.bounds, entity->phys.angle, entity->color);
            break;
        case ENTITY_DRAW_TYPE_SIMPLE_OUTLINE:
            // stroke bounds
            bounds_render_stroked(draw_x, draw_y, &entity->phys.bounds, entity->phys.angle, entity->color);
            break;
        case ENTITY_DRAW_TYPE_SPRITE:
            if (entity->sprite.sprite_def == NULL)
                return;

            sprite_draw(&entity->sprite, draw_x, draw_y, entity->phys.angle, entity->scale);
            sprite_update(&entity->sprite);
            break;
        case ENTITY_DRAW_TYPE_INVISIBLE:
        default:
            break;
    }
}


bool entity_event_check(struct entity *entity, enum entity_event event) {
    if (event > NUM_ENTITY_EVENTS)
        return false;

    return (entity->_event_flags & (1 << event)) ? true : false;
}

void entity_event_clear(struct entity *entity, enum entity_event event) {
    if (event > NUM_ENTITY_EVENTS)
        return;

    entity->_event_flags &= ~(1 << event);
}