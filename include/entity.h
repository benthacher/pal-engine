#pragma once

#include "physics.h"
#include "sprite.h"
#include "graphics.h"

#include <stdlib.h>
#include <stddef.h>

// forward declare for event handler
struct entity;

enum entity_draw_type {
    ENTITY_DRAW_TYPE_INVISIBLE,
    // simple: fill solid color based on bounds
    ENTITY_DRAW_TYPE_SIMPLE,
    // simple outline: stroke line based on bounds
    ENTITY_DRAW_TYPE_SIMPLE_OUTLINE,
    // sprite: draw sprite at physical location
    ENTITY_DRAW_TYPE_SPRITE,
};

enum entity_bounds_type {
    ENTITY_BOUNDS_TYPE_CIRCLE,
    ENTITY_BOUNDS_TYPE_POLY,
    ENTITY_BOUNDS_TYPE_RECTANGLE,
};

enum entity_event {
    ENTITY_EVENT_UPDATE = 0,
    ENTITY_EVENT_CLICK,
    ENTITY_EVENT_RELEASE,
    ENTITY_EVENT_BUTTON,

    NUM_ENTITY_EVENTS
};

_Static_assert(NUM_ENTITY_EVENTS < 32, "Too many entity events! Increase the size of config_flags or decrease number of events!");

/**
 * @brief Event handler typedef
 *
 * struct entity * entity event has been emitted on
 * void * event data (type dependent)
 */
typedef void (*entity_event_handler_t)(struct entity *, void *);

struct entity_config_flags {
    uint32_t do_physics : 1;
};

struct entity {
    struct phys_data phys;      // physical data (position, velocity, etc.)
    enum entity_draw_type type; // draw type
    union {                     // draw type specific data
        struct color color;
        struct {
            struct sprite sprite;
            float scale;
        };
    };
    struct entity_config_flags config_flags;

    // private stuff
    entity_event_handler_t _event_handlers[NUM_ENTITY_EVENTS];
    void *_event_data[NUM_ENTITY_EVENTS];
    uint32_t _event_flags;
};

/**
 * @brief Initializes entity with given mass. This function adds the entity to the list and
 *
 * @param entity pointer to entity
 * @param mass mass
 */
void entity_init(struct entity *entity, float mass);

/**
 * @brief Sets config on entity
 *
 * @param entity
 * @param config
 */
void entity_set_config(struct entity *entity, struct entity_config_flags config);

/**
 * @brief Scales entity's bounds by given factor
 *
 * @param entity
 * @param factor
 */
void entity_scale(struct entity *entity, float factor);

/**
 * @brief Sets draw type of entity.
 * Depending on draw type, the argument after type MUST be of the following type:
        ENTITY_DRAW_TYPE_INVISIBLE:      (none)
        ENTITY_DRAW_TYPE_SIMPLE:         (struct color) color
        ENTITY_DRAW_TYPE_SIMPLE_OUTLINE: (struct color) color
        ENTITY_DRAW_TYPE_SPRITE:         (struct sprite_def *) sprite_def
 *
 * @param entity
 * @param type
 * @param ...
 */
void entity_set_draw_type(struct entity *entity, enum entity_draw_type type, ...);

/**
 * @brief Sets bounds of entity.
 * Depending on draw type, the argument after type MUST be of the following type:
        ENTITY_BOUNDS_TYPE_CIRCLE:    (float) radius
        ENTITY_BOUNDS_TYPE_POLY:      (size_t) n_vertices, (struct vec2 *) vertices
        ENTITY_BOUNDS_TYPE_RECTANGLE: (float) width, (float) height
 *
 * @param entity
 * @param type
 * @param ...
 */
void entity_set_bounds(struct entity *entity, enum entity_bounds_type type, ...);

/**
 * @brief Assigns event handler for given event
 *
 * @param entity
 * @param handler
 * @param event
 */
void entity_attach_handler(struct entity *entity, enum entity_event event, entity_event_handler_t handler);

/**
 * @brief Emits event to entity
 *
 * @param entity
 * @param event
 * @param data
 * @param data_len
 */
void entity_event_emit(struct entity *entity, enum entity_event event, void *data, size_t data_len);

/**
 * @brief Call event handler for entity based on given event with given data
 *
 * @param entity
 * @param event
 * @param event_data
 */
void entity_call_event_handler(struct entity *entity, enum entity_event event);

/**
 * @brief Checks if given event is set on entity
 *
 * @param entity
 * @param event
 * @return event status
 */
bool entity_event_check(struct entity *entity, enum entity_event event);

/**
 * @brief Clears given event on entity
 *
 * @param entity
 * @param event
 */
void entity_event_clear(struct entity *entity, enum entity_event event);

/**
 * @brief Renders entity
 *
 * @param entity
 */
void entity_render(struct entity *entity);