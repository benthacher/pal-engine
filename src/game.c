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

#define MAX_COLLISIONS 100
struct entity_list_node {
    struct entity *entity;
    struct entity_list_node *next;
    struct entity_list_node *prev;
};

static pal_float_t frame_start, frame_duration;
static bool running = false;
static struct entity_list_node *entity_list_head = NULL;
static struct collision_descriptor collisions[MAX_COLLISIONS];
static size_t num_collisions = 0;

static struct pointer {
    struct vec2 current_position;
    enum pointer_state current_state;
    struct vec2 previous_position;
    struct vec2 velocity;
    struct vec2 dragging_entity_offset;
    struct entity *dragging_entity;
    bool previous_position_valid;
    bool can_click_entity;

    pal_float_t previous_time;
} pointer = {
    .previous_position_valid = false,
    .can_click_entity = true,
    .dragging_entity = NULL,
    .current_state = POINTER_STATE_UP
};

static enum button_state buttons[NUM_BUTTONS] = {
    [BUTTON_LEFT] = BUTTON_STATE_UP,
    [BUTTON_RIGHT] = BUTTON_STATE_UP,
    [BUTTON_LEFT_TRIGGER] = BUTTON_STATE_UP,
    [BUTTON_RIGHT_TRIGGER] = BUTTON_STATE_UP,
};

static struct camera {
    bool dragging;
    enum camera_pointer_control pointer_control;
    pal_float_t zoom;
    pal_float_t angle;
    struct vec2 position;
    struct vec2 velocity;
    struct mat2 transform;
    struct mat2 inv_transform;
    struct vec2 drag_start_screen;
    union {
        pal_float_t drag_start_angle;
        pal_float_t drag_start_zoom;
        struct vec2 drag_start_camera_pos;
    };
} game_camera = {
    .dragging = false,
    .pointer_control = CAMERA_POINTER_CONTROL_NONE,
    .zoom = 1,
    .angle = 0,
    .position = { 0, 0 },
    .velocity = { 0, 0 },
};

struct vec2 screen_center;

void game_loop_stop() {
    running = false;
}

void game_entity_add(struct entity *entity) {
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

void game_entity_remove(struct entity *entity) {
    entity_state_set(entity, ENTITY_STATE_SHOULD_BE_REMOVED);
}

static void remove_entity_node_from_list(struct entity_list_node *node) {
    if (node->prev != NULL)
        node->prev->next = node->next;

    if (node->next != NULL)
        node->next->prev = node->prev;

    free(node);
}

static void detect_and_add_collision(struct entity *entity1, struct entity *entity2) {
    if (num_collisions == MAX_COLLISIONS)
        return;

    if (!entity_state_check(entity1, ENTITY_STATE_DO_COLLISIONS) || !entity_state_check(entity2, ENTITY_STATE_DO_COLLISIONS))
        return;

    // check if collision has been detected
    for (size_t i = 0; i < num_collisions; i++) {
        if ((collisions[i].phys1 == &entity1->phys && collisions[i].phys2 == &entity2->phys) ||
            (collisions[i].phys1 == &entity2->phys && collisions[i].phys2 == &entity1->phys))
            return;
    }

    // if collision was detected, increment number of collisions so the next descriptor is filled in
    if (physics_detect_collision(&entity1->phys, &entity2->phys, &collisions[num_collisions])) {
        // send pointer to descriptor to both entities involved
        struct collision_descriptor *desc = &collisions[num_collisions];

        entity_event_emit(entity1, ENTITY_EVENT_COLLISION, (void *) &desc, sizeof(struct collision_descriptor *));
        entity_event_emit(entity2, ENTITY_EVENT_COLLISION, (void *) &desc, sizeof(struct collision_descriptor *));
        num_collisions++;
    }
}

static void camera_integrate(pal_float_t dt) {
    struct vec2 camera_scaled_velocity;

    // Integrate camera velocity, slow it down exponentially
    vec2_scale(&game_camera.velocity, dt, &camera_scaled_velocity);
    vec2_add(&game_camera.position, &camera_scaled_velocity, &game_camera.position);
    vec2_scale(&game_camera.velocity, 0.9, &game_camera.velocity);
}

static void camera_calculate_transform() {
    struct mat2 final_transform;

    pal_float_t cos_angle = pal_cos(game_camera.angle);
    pal_float_t sin_angle = pal_sin(game_camera.angle);

    struct mat2 camera_scale_and_rotate = {
        cos_angle * game_camera.zoom, -sin_angle * game_camera.zoom,
        sin_angle * game_camera.zoom,  cos_angle * game_camera.zoom,
    };

    struct mat2 camera_reflection = { 1, 0, 0, -1 };

    mat2_multiply(&camera_scale_and_rotate, &camera_reflection, &final_transform);

    game_camera_set_transform(&final_transform);
}

static void camera_handle_pan() {
    struct vec2 diff;
    struct vec2 diff_world;

    if (pointer.current_state == POINTER_STATE_DOWN) {
        if (!game_camera.dragging) {
            // get camera start position
            game_camera.drag_start_camera_pos = game_camera.position;
            // get screen coordinates
            game_camera.drag_start_screen = pointer.current_position;
        }

        game_camera.dragging = true;

        vec2_sub(&pointer.current_position, &game_camera.drag_start_screen, &diff);
        vec2_transform(&diff, &game_camera.inv_transform, &diff_world);
        vec2_scale(&diff_world, -1, &diff_world);
        vec2_add(&diff_world, &game_camera.drag_start_camera_pos, &game_camera.position);
    } else {
        if (game_camera.dragging) {
            struct vec2 flipped_pointer_velocity;
            vec2_transform(&pointer.velocity, &game_camera.inv_transform, &flipped_pointer_velocity);
            vec2_scale(&flipped_pointer_velocity, -1, &flipped_pointer_velocity);
            game_camera.velocity = flipped_pointer_velocity;
        }

        game_camera.dragging = false;
    }
}

static void camera_handle_zoom() {
    if (pointer.current_state == POINTER_STATE_DOWN) {
        if (!game_camera.dragging) {
            // get camera start position
            game_camera.drag_start_zoom = game_camera.zoom;
            // get screen coordinates
            game_camera.drag_start_screen = pointer.current_position;
        }

        game_camera.dragging = true;

        game_camera.zoom = game_camera.drag_start_zoom + (game_camera.drag_start_screen.y - pointer.current_position.y) / 10;

        game_camera.zoom = pal_fmax(game_camera.zoom, 0.0);

        camera_calculate_transform();
    } else {
        game_camera.dragging = false;
    }
}

static void camera_handle_roll() {
    struct vec2 center_to_pointer;
    struct vec2 center_to_drag_start;

    if (pointer.current_state == POINTER_STATE_DOWN) {
        if (!game_camera.dragging) {
            // get camera start position
            game_camera.drag_start_angle = game_camera.angle;
            // get screen coordinates
            game_camera.drag_start_screen = pointer.current_position;
        }

        game_camera.dragging = true;

        vec2_sub(&pointer.current_position, &screen_center, &center_to_pointer);
        vec2_sub(&game_camera.drag_start_screen, &screen_center, &center_to_drag_start);

        game_camera.angle = game_camera.drag_start_angle + (vec2_dir(&center_to_pointer) - vec2_dir(&center_to_drag_start));

        camera_calculate_transform();
    } else {
        game_camera.dragging = false;
    }
}

static void camera_handle_events() {
    switch (game_camera.pointer_control) {
        case CAMERA_POINTER_CONTROL_PAN:
            camera_handle_pan();
            break;
        case CAMERA_POINTER_CONTROL_ZOOM:
            camera_handle_zoom();
            break;
        case CAMERA_POINTER_CONTROL_ROLL:
            camera_handle_roll();
            break;
        case CAMERA_POINTER_CONTROL_NONE:
        default:
            break;
    }
}

static void emit_events() {
    struct pal_event e;
    pal_float_t current_time = pal_get_time();

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

    camera_handle_events();

    // detect collisions

    num_collisions = 0;

    for (struct entity_list_node *e1 = entity_list_head; e1 != NULL; e1 = e1->next) {
        for (struct entity_list_node *e2 = entity_list_head; e2 != NULL; e2 = e2->next) {
            if (e1 == e2)
                continue;

            detect_and_add_collision(e1->entity, e2->entity);
        }
    }
}

enum button_state game_button_check(enum button button) {
    return buttons[button];
}

enum pointer_state game_pointer_get_state() {
    return pointer.current_state;
}

const struct vec2 *game_pointer_get_position() {
    return (const struct vec2 *) &pointer.current_position;
}

const struct vec2 *game_pointer_get_velocity() {
    return (const struct vec2 *) &pointer.velocity;
}

static void entity_event_emit_immediate(struct entity *entity, enum entity_event event_id, void *data) {
    if (entity->_event_handlers[event_id] != NULL)
        entity->_event_handlers[event_id](entity, data);
}

static void render_all() {
    // render entities
    for (struct entity_list_node *e = entity_list_head; e != NULL; e = e->next) {
        entity_render(e->entity);
    }
}

static void update_all(pal_float_t dt) {
    for (size_t i = 0; i < num_collisions; i++)
        physics_resolve_collision(&collisions[i]);

    for (struct entity_list_node *e = entity_list_head; e != NULL; e = e->next) {
        entity_event_emit_immediate(e->entity, ENTITY_EVENT_UPDATE, (void *) &dt);

        if (entity_state_check(e->entity, ENTITY_STATE_SHOULD_BE_REMOVED)) {
            struct entity_list_node *next = e->next;
            // have entity handle all pending events before removing it
            entity_handle_pending_events(e->entity);
            // call destroy handler (destructor) if one exists
            entity_event_emit_immediate(e->entity, ENTITY_EVENT_DESTROY, NULL);
            // remove node from list
            remove_entity_node_from_list(e);
            e = next;
            continue;
        }

        if (entity_state_check(e->entity, ENTITY_STATE_DO_PHYSICS))
            physics_integrate(&e->entity->phys, dt);
    }
}

void entity_handle_all_events() {
    for (struct entity_list_node *node = entity_list_head; node != NULL; node = node->next) {
        entity_handle_pending_events(node->entity);
    }
}

void game_camera_screen_to_world(int screen_x, int screen_y, struct vec2 *world_pos) {
    struct vec2 screen_pos;
    screen_pos.x = screen_x - screen_center.x;
    screen_pos.y = screen_y - screen_center.y;

    vec2_transform(&screen_pos, &game_camera.inv_transform, world_pos);

    vec2_add(world_pos, &game_camera.position, world_pos);
}

void game_camera_world_to_screen(const struct vec2 *world_pos, int *screen_x, int *screen_y) {
    struct vec2 diff_from_camera, diff_transformed;
    vec2_sub(world_pos, &game_camera.position, &diff_from_camera);

    vec2_transform(&diff_from_camera, &game_camera.transform, &diff_transformed);

    *screen_x = pal_round(diff_transformed.x + screen_center.x);
    *screen_y = pal_round(diff_transformed.y + screen_center.y);
}

bool game_is_point_on_screen(const struct vec2 *world_pos) {
    int screen_x, screen_y;

    game_camera_world_to_screen(world_pos, &screen_x, &screen_y);

    return screen_x >= 0 && screen_x <= PAL_SCREEN_WIDTH &&
           screen_y >= 0 && screen_y <= PAL_SCREEN_HEIGHT;
}

void game_camera_set_velocity(const struct vec2 *velocity) {
    game_camera.velocity = *velocity;
}

void game_camera_set_position(const struct vec2 *position) {
    game_camera.position = *position;
}

const struct vec2 *game_camera_get_position() {
    return (const struct vec2 *) &game_camera.position;
}

void game_camera_set_transform(const struct mat2 *transform) {
    struct mat2 inv_transform;

    if (!mat2_inv(transform, &inv_transform))
        return;

    // if matrix has inverse then set transform and inverse transform
    game_camera.transform = *transform;
    game_camera.inv_transform = inv_transform;
}

const struct mat2 *game_camera_get_transform() {
    return (const struct mat2 *) &game_camera.transform;
}

const struct mat2 *game_camera_get_inv_transform() {
    return (const struct mat2 *) &game_camera.inv_transform;
}

void game_camera_set_pointer_control(enum camera_pointer_control control) {
    game_camera.pointer_control = control;
}

void game_loop_run() {
    running = true;

    audio_start();

    screen_center.x = PAL_SCREEN_WIDTH / 2;
    screen_center.y = PAL_SCREEN_HEIGHT / 2;

    camera_calculate_transform();

    // fill in translated bounds first
    for (struct entity_list_node *e = entity_list_head; e != NULL; e = e->next)
        physics_compute_translated_bounds(&e->entity->phys);

    while (running) {
        // get frame start timestamp
        frame_start = pal_get_time();
        pal_screen_clear((struct color) { 0xff, 0xff, 0xff });

        emit_events();

        entity_handle_all_events();

        update_all(DT);

        // be sure entity bounds are up to date
        for (struct entity_list_node *e = entity_list_head; e != NULL; e = e->next)
            physics_compute_translated_bounds(&e->entity->phys);

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