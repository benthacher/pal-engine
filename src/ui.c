#include "ui.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "pal.h"

// element magic field must equal this number
#define ELEMENT_MAGIC_FIELD (0xab392cfe)

element_t *first_element = NULL;
static struct vec2 drag_elem_relative_pos = {
    .x = -1,
    .y = -1,
};

static bool inline element_is_valid(element_t *elem) {
    return elem != NULL && elem->magic == ELEMENT_MAGIC_FIELD;
}

static void element_init(element_t *element) {
    // fill in element fields
    element->magic = ELEMENT_MAGIC_FIELD;
    element->parent = NULL;
    element->first_child = NULL;
    element->next_sibling = NULL;
    element->event_state = EVENT_STATE_NONE;
    element->render = NULL;
    element->click = NULL;

    // add element to list
    if (first_element != NULL)
        element->next = first_element;

    first_element = element;
}

void element_remove(element_t *element_to_remove) {
    element_t *elem = first_element;

    if (elem == NULL || !element_is_valid(element_to_remove))
        return;

    while (elem->next != element_to_remove && elem->next != NULL)
        elem = elem->next;

    if (elem->next == element_to_remove)
        elem->next = elem->next->next;
}

void ui_element_add_child(element_t *parent, element_t *child) {
    if (!element_is_valid(parent) || !element_is_valid(child) || child->parent != NULL)
        return;

    child->parent = parent;
    child->next_sibling = parent->first_child;
    parent->first_child = child;
}

void ui_render_elements() {
    // get pointer before rendering so we can use it in collision checking
    // pointer_data_t *pointer = screen_get_pointer();

    // for (element_t *elem = first_element; elem != NULL; elem = elem->next) {
    //     // only check for hover event if the element's not clicked
    //     if (elem->event_state & EVENT_STATE_CLICKED)
    //         continue;

    //     // check for collisions
    //     if (pointer->position.x >= elem->x && pointer->position.x <= elem->x + elem->width &&
    //         pointer->position.y >= elem->y && pointer->position.y <= elem->y + elem->height) {

    //         // bubble event up
    //         for (element_t *bubble = elem; bubble != NULL; bubble = bubble->parent) {
    //             // don't check for collision, parents always contain child
    //             bubble->event_state |= EVENT_STATE_HOVERED;

    //             if (pointer->state == POINTER_DOWN) {
    //                 bubble->event_state |= EVENT_STATE_CLICKED;
    //             }
    //         }
    //     } else {
    //         elem->event_state &= ~EVENT_STATE_HOVERED;
    //     }
    // }

    // // execute clicks, render elements
    // for (element_t *elem = first_element; elem != NULL; elem = elem->next) {
    //     if (pointer->state == POINTER_UP && elem->event_state & EVENT_STATE_CLICKED && elem->click != NULL) {
    //         // if the element was clicked and the pointer's now released, call click function if it exists
    //         elem->click(elem);
    //         elem->event_state &= ~(EVENT_STATE_CLICKED);
    //     }

    //     // render element visually
    //     if (elem->render != NULL)
    //         elem->render(elem);
    // }
}

static void box_render(element_t *box) {
    int draw_x, draw_y;
    struct color color = { box->event_state & EVENT_STATE_CLICKED ? 0x00 : 0xff, box->event_state & EVENT_STATE_HOVERED ? 0x00 : 0xff, 0xff, 0xff };

    for (draw_x = box->x; draw_x < box->x + box->width; draw_x++) {
        for (draw_y = box->y; draw_y < box->y + box->height; draw_y++) {
            pal_screen_draw_pixel(draw_x, draw_y, color);
        }
    }
}

static void box_click(element_t *box) {
    printf("clicked box!\n");
}

void ui_create_box(element_t *box, int x, int y, int width, int height) {
    element_init(box);

    box->x = x;
    box->y = y;
    box->width = width;
    box->height = height;
    box->text = "box";
    box->type = ELEM_TYPE_BOX;
    box->render = &box_render;
    box->click = &box_click;
}