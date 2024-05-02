#include "game.h"

#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "entity.h"
#include "audio.h"

struct entity_list_node {
    struct entity *entity;
    struct entity_list_node *next;
    struct entity_list_node *prev;
};

static double frame_start, frame_duration;
static bool running = false;
static struct entity_list_node *entity_list_head = NULL;

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

    while (pal_poll_event(&e)) {
        switch (e.type) {
            case PAL_EVENT_TYPE_BUTTON:
                for (struct entity_list_node *node = entity_list_head; node != NULL; node = node->next) {
                    entity_event_emit(node->entity, ENTITY_EVENT_BUTTON, &e.button, sizeof(struct button_event));
                }
                break;

            case PAL_EVENT_TYPE_POINTER:
                for (struct entity_list_node *node = entity_list_head; node != NULL; node = node->next) {
                    // check pointer collisions
                    // entity_event_emit(node->entity, ENTITY_EVENT_BUTTON, &e.button, sizeof(struct button_event));
                }
                break;

            case PAL_EVENT_TYPE_IMU:

                break;
        }
    }

    for (struct entity_list_node *node = entity_list_head; node != NULL; node = node->next) {
        entity_event_emit(node->entity, ENTITY_EVENT_UPDATE, NULL, 0);
    }
}

static void render_all() {
    // render entities
    for (struct entity_list_node *e = entity_list_head; e != NULL; e = e->next) {
        entity_render(e->entity);
    }
}

static void update_all(float dt) {
    for (struct entity_list_node *e = entity_list_head; e != NULL; e = e->next) {
        if (e->entity->config_flags.do_physics)
            physics_integrate(&e->entity->phys, dt);
    }
}

void entity_handle_all_events() {
    for (struct entity_list_node *node = entity_list_head; node != NULL; node = node->next) {
        for (int e = 0; e < NUM_ENTITY_EVENTS; e++) {
            if (entity_event_check(node->entity, e))
                entity_call_event_handler(node->entity, (enum entity_event) e);
        }
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

        update_all(DT);

        entity_handle_all_events();

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