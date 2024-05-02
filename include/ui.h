#pragma once

#include <stdint.h>
#include <stdbool.h>

#include "graphics.h"

typedef enum element_type {
    ELEM_TYPE_CONTAINER,
    ELEM_TYPE_TEXTBOX,
    ELEM_TYPE_BOX,
    ELEM_TYPE_BUTTON,
    ELEM_TYPE_IMAGE,
} element_type_t;

typedef enum event_state {
    EVENT_STATE_NONE =      0,
    EVENT_STATE_CLICKED =   1 << 0,
    EVENT_STATE_RELEASED =  1 << 1,
    EVENT_STATE_HOVERED =   1 << 2,
} event_state_t;

typedef struct element {
    int x;
    int y;
    int width;
    int height;
    struct element *parent;
    struct element *next_sibling;
    struct element *first_child;
    void *data;
    char *name;
    char *text;
    element_type_t type;
    event_state_t event_state;

    void (*render)(struct element *);
    void (*click)(struct element *);

    uint32_t magic; // must equal ELEMENT_MAGIC_FIELD
    struct element *next;
} element_t;

void ui_render_elements();
void ui_element_add_child(element_t *parent, element_t *child);
void ui_create_box(element_t *box, int x, int y, int width, int height);