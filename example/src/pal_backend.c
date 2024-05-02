#include "pal.h"

const screen_dim_t PAL_SCREEN_WIDTH = 0;
const screen_dim_t PAL_SCREEN_HEIGHT = 0;
uint32_t PAL_AUDIO_SAMPLE_RATE = 0;
const uint32_t PAL_RAND_MAX = 0;

static pal_audio_callback_t pal_audio_callback;

void pal_screen_clear(struct color c) {

}

void pal_screen_render() {

}

void pal_screen_draw_pixel(int x, int y, struct color c) {

}

double pal_get_time() {
    return 0.0f;
}

void pal_set_audio_callback(pal_audio_callback_t audio_callback) {
    pal_audio_callback = audio_callback;
}

bool pal_poll_event(struct pal_event *event) {
    return false;
}

float pal_sin(float a) {
    return 0.0f;
}

float pal_cos(float a) {
    return 0.0f;
}

uint32_t pal_rand() {
    return 0;
}

bool pal_init() {
    return true;
}