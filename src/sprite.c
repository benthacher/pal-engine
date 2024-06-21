#include "sprite.h"

#include "graphics.h"

void sprite_update(struct sprite *sprite) {
    if (sprite->sprite_def->num_frames == 1)
        return;

    pal_float_t current_time = pal_get_time();

    if (sprite->previous_time == 0.0) {
        sprite->previous_time = current_time;
        return;
    }

    sprite->frame_elapsed_time += current_time - sprite->previous_time;
    sprite->previous_time = current_time;

    while (sprite->frame_elapsed_time > sprite->sprite_def->frames[sprite->current_frame]->duration) {
        sprite->frame_elapsed_time -= sprite->sprite_def->frames[sprite->current_frame]->duration;

        // next frame
        if (sprite->current_frame < sprite->sprite_def->num_frames - 1) {
            sprite->current_frame++;
            sprite->finished = false;
        } else if (sprite->sprite_def->loop) {
            sprite->current_frame = 0;
            sprite->finished = true;
        }

        if (sprite->current_frame == sprite->sprite_def->num_frames - 1 && !sprite->sprite_def->loop)
            sprite->finished = true;
    }
}

void sprite_init(struct sprite *sprite, struct sprite_def *sprite_def) {
    sprite->finished = false;
    sprite->current_frame = 0;
    sprite->previous_time = 0.0;
    sprite->frame_elapsed_time = 0.0;
    sprite->sprite_def = sprite_def;
}

void sprite_draw(struct sprite *sprite, int x, int y, pal_float_t angle, pal_float_t scale) {
    graphics_draw_image(sprite->sprite_def->frames[sprite->current_frame]->image, x, y, angle, scale);
}