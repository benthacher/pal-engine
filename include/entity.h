#pragma once

#include "physics.h"
#include "sprite.h"
#include "graphics.h"
#include "queue.h"

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
    ENTITY_EVENT_DRAG_START,
    ENTITY_EVENT_DRAG_STOP,
    ENTITY_EVENT_SPRITE_LOOP_END,
    ENTITY_EVENT_BUTTON_UP,
    ENTITY_EVENT_BUTTON_DOWN,
    ENTITY_EVENT_COLLISION,

    NUM_ENTITY_EVENTS
};

enum entity_state {
    ENTITY_STATE_DRAGGING = 0,
    ENTITY_STATE_DRAGGABLE,
    ENTITY_STATE_DO_PHYSICS,
    ENTITY_STATE_DO_COLLISIONS,
    ENTITY_STATE_CLICKED,
    ENTITY_STATE_SPRITE_LOOP_ENDED,
    ENTITY_STATE_SHOULD_BE_REMOVED,

    NUM_ENTITY_STATES
};

typedef uint8_t entity_event_id_t;

_Static_assert(NUM_ENTITY_EVENTS < 256, "Too many entity events! Increase the size of entity_event_id_t or decrease number of events!");
_Static_assert(NUM_ENTITY_STATES < 32, "Too many entity states! Increase the size of _state_flags or decrease number of states!");

/**
 * @brief Event handler typedef
 *
 * struct entity * entity event has been emitted on
 * void * event data (type dependent)
 */
typedef void (*entity_event_handler_t)(struct entity *, void *);

struct entity {
    struct phys_data phys;      // physical data (position, velocity, etc.)
    enum entity_draw_type type; // draw type
    union {                     // draw type specific data
        struct color color;
        struct {
            struct sprite sprite;
            pal_float_t scale;
        };
    };

    // private stuff
    struct queue _event_queue;
    uint8_t _event_queue_buffer[100];
    entity_event_handler_t _event_handlers[NUM_ENTITY_EVENTS];
    uint32_t _state_flags;
};

/**
 * @brief Initializes entity with given mass. This function adds the entity to the list and
 *
 * @param entity pointer to entity
 * @param mass mass
 */
void entity_init(struct entity *entity, pal_float_t mass);

/**
 * @brief Scales entity's bounds by given factor
 *
 * @param entity
 * @param factor
 */
void entity_scale(struct entity *entity, pal_float_t factor);

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
        ENTITY_BOUNDS_TYPE_CIRCLE:    (pal_float_t) radius
        ENTITY_BOUNDS_TYPE_POLY:      (size_t) n_vertices, (struct vec2 *) vertices
        ENTITY_BOUNDS_TYPE_RECTANGLE: (pal_float_t) width, (pal_float_t) height
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
 * @brief Handles all pending events on entity
 *
 * @param entity
 */
void entity_handle_pending_events(struct entity *entity);

/**
 * @brief Sets given state on entity
 *
 * @param entity
 * @param state
 */
void entity_state_set(struct entity *entity, enum entity_state state);

/**
 * @brief Clears given state on entity
 *
 * @param entity
 * @param state
 */
void entity_state_clear(struct entity *entity, enum entity_state state);

/**
 * @brief Checks if given state is present on entity
 *
 * @param entity
 * @param state
 * @return true
 * @return false
 */
bool entity_state_check(struct entity *entity, enum entity_state state);

/**
 * @brief Renders entity
 *
 * @param entity
 */
void entity_render(struct entity *entity);