#pragma once
/**
 * @file pal.h
 * @author Ben Thacher (benisadeveloper@gmail.com)
 * @brief
 * @version 0.1
 * @date 2024-04-20
 *
 * @copyright Copyright (c) 2024
 *
 * Palygon Abstraction Layer
 * This file shouldn't need to change for different platforms
 *
 */

#include <stdint.h>
#include <stdbool.h>

// Set this flag in the CMakeLists.txt file depending on the desired platform precision
#if defined PAL_USE_FLOAT32 && PAL_USE_FLOAT32 == 1
typedef float pal_float_t;
#else
typedef double pal_float_t;
#endif

#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

typedef uint16_t screen_dim_t;
typedef uint16_t audio_sample_t;
#define AUDIO_SAMPLE_MAX ((1 << (sizeof(audio_sample_t) << 3)) - 1)

typedef void (*pal_audio_callback_t)(audio_sample_t *samples, int num_samples);

/* Screen stuff */
// Screen size must be set in pal.c depending on actual/window screen dimensions
extern const screen_dim_t PAL_SCREEN_WIDTH;
extern const screen_dim_t PAL_SCREEN_HEIGHT;
extern uint32_t PAL_AUDIO_SAMPLE_RATE;
extern const uint32_t PAL_RAND_MAX;


/**
 * @brief 2D Vector
 *
 */
struct vec2 {
    pal_float_t x;
    pal_float_t y;
};

/**
 * @brief 3D Vector
 *
 */
struct vec3 {
    pal_float_t x;
    pal_float_t y;
    pal_float_t z;
};

/**
 * @brief 2D Matrix
 *
 */
struct mat2 {
    pal_float_t a;
    pal_float_t b;
    pal_float_t c;
    pal_float_t d;
};

struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

/* Input event stuff */
enum pointer_state {
    POINTER_STATE_DOWN,
    POINTER_STATE_UP,
};

struct pointer_event {
    struct vec2 position;
    enum pointer_state state;
};

enum button_state {
    BUTTON_STATE_DOWN,
    BUTTON_STATE_UP,
};

enum button {
    BUTTON_LEFT = 0,
    BUTTON_RIGHT,
    BUTTON_LEFT_TRIGGER,
    BUTTON_RIGHT_TRIGGER,
    NUM_BUTTONS
};

struct button_event {
    enum button which;
    enum button_state state;
};

struct imu_event {
    struct vec3 acceleration;
    struct vec3 gyro;
};

enum pal_event_type {
    PAL_EVENT_TYPE_POINTER,
    PAL_EVENT_TYPE_BUTTON,
    PAL_EVENT_TYPE_IMU
};

struct pal_event {
    enum pal_event_type type;
    union {
        struct pointer_event pointer;
        struct button_event button;
        struct imu_event imu;
    };
};

/**
 * @brief Initializes PAL
 *
 * @return true successful initialization
 * @return false failure
 */
bool pal_init();

/**
 * @brief Clears screen with given color
 *
 */
void pal_screen_clear(struct color c);

/**
 * @brief Renders written pixels to screen
 *
 */
void pal_screen_render();

/**
 * @brief Draws pixel at position with color
 *
 * @param x
 * @param y
 * @param c
 */
void pal_screen_draw_pixel(int x, int y, struct color c);

/**
 * @brief Checks if event has occurred, placing event data into event pointer
 *
 * @param event
 * @return true if event occurred
 * @return false if not
 */
bool pal_poll_event(struct pal_event *event);

/**
 * @brief Get time in seconds (resolution platform dependent)
 *
 * @return double
 */
double pal_get_time();

/**
 * @brief Sets audio callback, called when the audio subsystem is ready for samples
 *
 * @param audio_callback
 */
void pal_set_audio_callback(pal_audio_callback_t audio_callback);

/**
 * @brief Sine function
 *
 * @param a
 * @return pal_float_t
 */
pal_float_t pal_sin(pal_float_t a);

/**
 * @brief Cosine function
 *
 * @param a
 * @return pal_float_t
 */
pal_float_t pal_cos(pal_float_t a);

/**
 * @brief Random number between 0 and PAL_RAND_MAX
 *
 * @return uint32_t
 */
uint32_t pal_rand();