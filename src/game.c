#include "game.h"

#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "entity.h"
#include "audio.h"
#include "pal.h"
#include "mathutils.h"

struct entity_list_node {
    struct entity *entity;
    struct entity_list_node *next;
    struct entity_list_node *prev;
};

static double frame_start, frame_duration;
static bool running = false;
static struct entity_list_node *entity_list_head = NULL;

static struct pointer {
    struct vec2 current_position;
    enum pointer_state current_state;
    struct vec2 previous_position;
    struct vec2 velocity;
    struct vec2 dragging_entity_offset;
    struct entity *dragging_entity;
    bool previous_position_valid;
    bool can_click_entity;

    double previous_time;
} pointer = {
    .previous_position_valid = false,
    .can_click_entity = true,
    .dragging_entity = NULL
};

static enum button_state buttons[NUM_BUTTONS] = {
    [BUTTON_LEFT] = BUTTON_STATE_UP,
    [BUTTON_RIGHT] = BUTTON_STATE_UP,
    [BUTTON_LEFT_TRIGGER] = BUTTON_STATE_UP,
    [BUTTON_RIGHT_TRIGGER] = BUTTON_STATE_UP,
};

void game_stop_loop() {
    running = false;
}

void game_add_entity(struct entity *entity) {
    struct entity_list_node *new_node = (struct entity_list_node *) malloc(sizeof(struct entity_list_node));
    new_node->entity = entity;

    if (entity_list_head == NULL) {
        new_node->next = NULL;
    } else {
        new_node->next = entity_list_head;
        entity_list_head->prev = new_node;
    }

    new_node->prev = NULL;
    entity_list_head = new_node;
}

void game_remove_entity(struct entity *entity) {
    struct entity_list_node *node = entity_list_head;

    while (node != NULL && node->entity != entity) {
        node = node->next;
    }

    // node not found
    if (node == NULL)
        return;

    if (node->prev != NULL)
        node->prev->next = node->next;

    if (node->next != NULL)
        node->next->prev = node->prev;

    free(node);
}

static void emit_events() {
    struct pal_event e;
    double current_time = pal_get_time();

    while (pal_poll_event(&e)) {
        switch (e.type) {
            case PAL_EVENT_TYPE_BUTTON:
                for (struct entity_list_node *node = entity_list_head; node != NULL; node = node->next) {
                    entity_event_emit(node->entity, e.button.state == BUTTON_STATE_UP ? ENTITY_EVENT_BUTTON_UP : ENTITY_EVENT_BUTTON_DOWN, &e.button.which, sizeof(enum button));
                }
                buttons[e.button.which] = e.button.state;
                break;

            case PAL_EVENT_TYPE_POINTER:
                pointer.current_position = e.pointer.position;
                pointer.current_state = e.pointer.state;

                break;

            case PAL_EVENT_TYPE_IMU:

                break;
        }
    }

    // emit pointer events for entities that the pointer interacted with
    if (pointer.current_state == POINTER_STATE_UP) {
        // release dragging entity if one exists
        if (pointer.dragging_entity) {
            entity_event_emit(pointer.dragging_entity, ENTITY_EVENT_DRAG_STOP, NULL, 0);
            entity_state_clear(pointer.dragging_entity, ENTITY_STATE_DRAGGING);
            pointer.dragging_entity->phys.velocity = pointer.velocity;
        }

        pointer.dragging_entity = NULL;

        for (struct entity_list_node *node = entity_list_head; node != NULL; node = node->next) {
            if (entity_state_check(node->entity, ENTITY_STATE_CLICKED)) {
                entity_event_emit(node->entity, ENTITY_EVENT_RELEASE, NULL, 0);
                entity_state_clear(node->entity, ENTITY_STATE_CLICKED);
            }
        }

        pointer.can_click_entity = true;
        pointer.previous_position_valid = false;
    } else if (pointer.current_state == POINTER_STATE_DOWN) {
        if (pointer.dragging_entity != NULL) {
            // entity is already being dragged, update the entity's position
            vec2_add(&pointer.current_position, &pointer.dragging_entity_offset, &pointer.dragging_entity->phys.position);
        } else if (pointer.can_click_entity) {
            for (struct entity_list_node *node = entity_list_head; node != NULL; node = node->next) {
                if (!physics_check_point_collision(&node->entity->phys, &pointer.current_position))
                    continue;

                if (entity_state_check(node->entity, ENTITY_STATE_DRAGGABLE) && pointer.dragging_entity == NULL) {
                    // Drag start!!!
                    pointer.dragging_entity = node->entity;

                    vec2_sub(&pointer.dragging_entity->phys.position, &pointer.current_position, &pointer.dragging_entity_offset);
                    entity_event_emit(pointer.dragging_entity, ENTITY_EVENT_DRAG_START, &pointer.dragging_entity_offset, sizeof(pointer.dragging_entity_offset));
                    entity_state_set(pointer.dragging_entity, ENTITY_STATE_DRAGGING);

                    pointer.dragging_entity->phys.velocity.x = pointer.dragging_entity->phys.velocity.y = 0.0;
                }

                entity_event_emit(node->entity, ENTITY_EVENT_CLICK, NULL, 0);
                entity_state_set(node->entity, ENTITY_STATE_CLICKED);
            }

            pointer.can_click_entity = false;
        }

        if (pointer.previous_position_valid) {
            pointer.velocity.x = (pointer.current_position.x - pointer.previous_position.x) / (current_time - pointer.previous_time);
            pointer.velocity.y = (pointer.current_position.y - pointer.previous_position.y) / (current_time - pointer.previous_time);
        } else {
            pointer.velocity.x = 0.0;
            pointer.velocity.y = 0.0;
        }

        pointer.previous_position = pointer.current_position;
        pointer.previous_time = current_time;
        pointer.previous_position_valid = true;
    }
}

enum button_state game_get_button(enum button button) {
    return buttons[button];
}

void game_get_pointer_position(struct vec2 *position) {
    position->x = pointer.current_position.x;
    position->y = pointer.current_position.y;
}

void game_get_pointer_velocity(struct vec2 *velocity) {
    velocity->x = pointer.velocity.x;
    velocity->y = pointer.velocity.y;
}

static void render_all() {
    // render entities
    for (struct entity_list_node *e = entity_list_head; e != NULL; e = e->next) {
        entity_render(e->entity);
    }
}

static void update_all(float dt) {
    for (struct entity_list_node *e = entity_list_head; e != NULL; e = e->next)
        physics_compute_translated_bounds(&e->entity->phys);

    for (struct entity_list_node *e = entity_list_head; e != NULL; e = e->next) {
        if (e->entity->_event_handlers[ENTITY_EVENT_UPDATE])
            e->entity->_event_handlers[ENTITY_EVENT_UPDATE](e->entity, (void *) &dt);

        if (entity_state_check(e->entity, ENTITY_STATE_DO_PHYSICS))
            physics_integrate(&e->entity->phys, dt);
    }
}

void entity_handle_all_events() {
    for (struct entity_list_node *node = entity_list_head; node != NULL; node = node->next) {
        entity_handle_pending_events(node->entity);
    }
}

void game_run_loop() {
    running = true;

    audio_start();

    while (running) {
        // get frame start timestamp
        frame_start = pal_get_time();
        pal_screen_clear((struct color) { 0xff, 0xff, 0xff });

        emit_events();

        entity_handle_all_events();

        update_all(DT);

        // render
        render_all();
        pal_screen_render();

        // get frame end timestamp
        frame_duration = pal_get_time() - frame_start;

        int sleep_us = FRAME_PERIOD_US - frame_duration * 1000000;

        // sleep for remainder of frame
        usleep(sleep_us > 0 ? sleep_us : 0);
    }

    audio_request_stop();
}