#include "entity.h"

#include <math.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "graphics.h"
#include "physics.h"
#include "sprite.h"
#include "game.h"

// every uninitialized entity event is zero, meaning no event data is used
static size_t event_data_lengths[NUM_ENTITY_EVENTS] = {
    [ENTITY_EVENT_BUTTON_UP] = sizeof(enum button),
    [ENTITY_EVENT_BUTTON_DOWN] = sizeof(enum button),
    [ENTITY_EVENT_COLLISION] = sizeof(struct collision_descriptor *),
};

const char *entity_event_str(enum entity_event event) {
    switch (event) {
        case ENTITY_EVENT_UPDATE: return "ENTITY_EVENT_UPDATE";
        case ENTITY_EVENT_CLICK: return "ENTITY_EVENT_CLICK";
        case ENTITY_EVENT_RELEASE: return "ENTITY_EVENT_RELEASE";
        case ENTITY_EVENT_DRAG_START: return "ENTITY_EVENT_DRAG_START";
        case ENTITY_EVENT_DRAG_STOP: return "ENTITY_EVENT_DRAG_STOP";
        case ENTITY_EVENT_SPRITE_LOOP_END: return "ENTITY_EVENT_SPRITE_LOOP_END";
        case ENTITY_EVENT_BUTTON_UP: return "ENTITY_EVENT_BUTTON_UP";
        case ENTITY_EVENT_BUTTON_DOWN: return "ENTITY_EVENT_BUTTON_DOWN";
        case ENTITY_EVENT_COLLISION: return "ENTITY_EVENT_COLLISION";
        default: return "Unknown Event";
    }
}

void entity_set_sprite_def(struct entity *entity, struct sprite_def *sprite_def) {
    if (entity->type != ENTITY_DRAW_TYPE_SPRITE)
        return;

    // set sprite def to given sprite def then reset
    entity->sprite.sprite_def = sprite_def;
    sprite_init(&entity->sprite, sprite_def);
}

void entity_init(struct entity *entity, pal_float_t mass) {
    // make sure event handlers are null
    for (int i = 0; i < NUM_ENTITY_EVENTS; i++) {
        entity->_event_handlers[i] = NULL;
    }

    // clear event flags
    entity->_state_flags = 0;

    // initialize event queue
    queue_init(&entity->_event_queue, entity->_event_queue_buffer, sizeof(entity->_event_queue_buffer));

    // enable physics by default
    entity_state_set(entity, ENTITY_STATE_DO_PHYSICS);

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
            pal_float_t radius = (pal_float_t) va_arg(bounds_args, double);
            physics_set_bounds_circle(&entity->phys, radius);
            break;
        }
        case ENTITY_BOUNDS_TYPE_RECTANGLE:
            pal_float_t width = (pal_float_t) va_arg(bounds_args, double);
            pal_float_t height = (pal_float_t) va_arg(bounds_args, double);
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
    // if event is invalid or event handler hasn't been attached, return
    if (event > NUM_ENTITY_EVENTS || entity->_event_handlers[event] == NULL)
        return;

    // add event id byte to event queue
    entity_event_id_t event_id = event;
    queue_write(&entity->_event_queue, &event_id, sizeof(event_id));

    // write event data to queue
    if (data != NULL && event_data_lengths[event] > 0) {
        if (!queue_write(&entity->_event_queue, data, event_data_lengths[event]))
            printf("Failed to write event data to queue! Increase the size of entity event queue.\n");
    }
}

void entity_handle_pending_events(struct entity *entity) {
    entity_event_id_t event_id;
    void *event_data;

    while (queue_get_remaining(&entity->_event_queue) > 0) {
        if (!queue_read(&entity->_event_queue, &event_id, sizeof(event_id)))
            break;

        if (entity->_event_handlers[event_id] != NULL) {
            if (event_data_lengths[event_id] > 0) {
                uint8_t event_data_payload[event_data_lengths[event_id]];

                // read event data into array and call handler with data
                if (!queue_read(&entity->_event_queue, event_data_payload, event_data_lengths[event_id]))
                    break;

                entity->_event_handlers[event_id](entity, (void *) event_data_payload);
            } else {
                // event has no data associated with it, call handler without passing data
                entity->_event_handlers[event_id](entity, NULL);
            }
        }
    }
}

void entity_state_set(struct entity *entity, enum entity_state state) {
    if (state > NUM_ENTITY_STATES)
        return;

    entity->_state_flags |= 1 << state;
}

void entity_state_clear(struct entity *entity, enum entity_state state) {
    if (state > NUM_ENTITY_STATES)
        return;

    entity->_state_flags &= ~(1 << state);
}

bool entity_state_check(struct entity *entity, enum entity_state state) {
    if (state > NUM_ENTITY_STATES)
        return false;

    return (entity->_state_flags & (1 << state)) ? true : false;
}

void entity_scale(struct entity *entity, pal_float_t factor) {
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
    struct vec2 start = {
        .x = pal_floor(pal_fmax(pal_fmin(v1->x, pal_fmin(v2->x, v3->x)), 0.0)),
        .y = pal_floor(pal_fmax(pal_fmin(v1->y, pal_fmin(v2->y, v3->y)), 0.0))
    };
    struct vec2 end = {
        .x = pal_fmin(pal_fmax(v1->x, pal_fmax(v2->x, v3->x)), PAL_SCREEN_WIDTH),
        .y = pal_fmin(pal_fmax(v1->y, pal_fmax(v2->y, v3->y)), PAL_SCREEN_HEIGHT)
    };

    struct vec2 p = start;
    for (p.y = start.y; p.y < end.y; p.y++) {
        for (p.x = start.x; p.x < end.x; p.x++) {
            if (edge_test(v1, v2, &p) && edge_test(v2, v3, &p) && edge_test(v3, v1, &p))
                pal_screen_draw_pixel(p.x, p.y, color);
        }
    }
}

static bool is_screen_pos_on_screen(int screen_x, int screen_y) {
    return screen_x >= 0 && screen_x <= PAL_SCREEN_WIDTH &&
           screen_y >= 0 && screen_y <= PAL_SCREEN_HEIGHT;
}

static void entity_render_filled(struct entity *entity) {
    struct vec2 entity_screen;
    int entity_x, entity_y;
    game_camera_world_to_screen(&entity->phys.position, &entity_x, &entity_y);
    entity_screen.x = entity_x;
    entity_screen.y = entity_y;

    if (entity->phys.bounds.type == BOUNDS_TYPE_POLY) {
        struct vec2 *p1, *p2;
        int p1_x, p1_y, p2_x, p2_y;
        struct vec2 p1_screen, p2_screen;
        for (int i = 0; i < entity->phys.translated_bounds.n_vertices; i++) {
            p1 = &entity->phys.translated_bounds.vertices[i];
            p2 = &entity->phys.translated_bounds.vertices[(i + 1) % entity->phys.translated_bounds.n_vertices];

            // transform p1 and p2 to screen coordinates
            game_camera_world_to_screen(p1, &p1_x, &p1_y);
            game_camera_world_to_screen(p2, &p2_x, &p2_y);

            p1_screen.x = p1_x;
            p1_screen.y = p1_y;
            p2_screen.x = p2_x;
            p2_screen.y = p2_y;

            render_triangle(&entity_screen, &p2_screen, &p1_screen, entity->color);
        }
    } else if (entity->phys.bounds.type == BOUNDS_TYPE_CIRCLE) {
        graphics_draw_circle(entity_x, entity_y, entity->phys.bounds.radius * mat2_det(game_camera_get_transform()), entity->color);
    }
}

static void entity_render_stroked(struct entity *entity) {
    int p1_screen_x, p1_screen_y;
    int p2_screen_x, p2_screen_y;

    if (entity->phys.bounds.type == BOUNDS_TYPE_POLY) {
        struct vec2 *p1, *p2;
        for (int i = 0; i < entity->phys.translated_bounds.n_vertices; i++) {
            p1 = &entity->phys.translated_bounds.vertices[i];
            p2 = &entity->phys.translated_bounds.vertices[(i + 1) % entity->phys.translated_bounds.n_vertices];

            game_camera_world_to_screen(p1, &p1_screen_x, &p1_screen_y);
            game_camera_world_to_screen(p2, &p2_screen_x, &p2_screen_y);
            // draw line from p1 to p2
            graphics_draw_line(p1_screen_x, p1_screen_y, p2_screen_x, p2_screen_y, entity->color);
        }
    } else if (entity->phys.bounds.type == BOUNDS_TYPE_CIRCLE) {
        game_camera_world_to_screen(&entity->phys.position, &p1_screen_x, &p1_screen_y);
        graphics_stroke_circle(p1_screen_x, p1_screen_y, entity->phys.bounds.radius * pal_sqrt(pal_fabs(mat2_det(game_camera_get_transform()))), entity->color, 1);
    }
}

void entity_render(struct entity *entity) {
    // draw entity at physical position
    int draw_x = pal_round(entity->phys.position.x);
    int draw_y = pal_round(entity->phys.position.y);
    bool previous_finished_flag;

    switch (entity->type) {
        case ENTITY_DRAW_TYPE_SIMPLE:
            // fill in bounds
            entity_render_filled(entity);
            break;
        case ENTITY_DRAW_TYPE_SIMPLE_OUTLINE:
            // stroke bounds
            entity_render_stroked(entity);
            break;
        case ENTITY_DRAW_TYPE_SPRITE:
            if (entity->sprite.sprite_def == NULL)
                return;

            game_camera_world_to_screen(&entity->phys.position, &draw_x, &draw_y);

            struct mat2 transform, final_transform;
            pal_float_t cos_angle = pal_cos(entity->phys.angle);
            pal_float_t sin_angle = pal_sin(entity->phys.angle);

            struct mat2 sprite_transform = {
                cos_angle * entity->scale,  -sin_angle * entity->scale,
                sin_angle * entity->scale,  cos_angle * entity->scale,
            };
            struct mat2 camera_reflection = { 1, 0, 0, -1 };

            mat2_multiply(&sprite_transform, game_camera_get_transform(), &transform);
            mat2_multiply(&transform, &camera_reflection, &final_transform);
            graphics_draw_transformed_image(entity->sprite.sprite_def->frames[entity->sprite.current_frame]->image, draw_x, draw_y, &final_transform);

            previous_finished_flag = entity->sprite.finished;
            sprite_update(&entity->sprite);

            if (entity->sprite.finished && !previous_finished_flag) {
                entity_event_emit(entity, ENTITY_EVENT_SPRITE_LOOP_END, NULL, 0);
            }

        case ENTITY_DRAW_TYPE_INVISIBLE:
        default:
            break;
    }
}