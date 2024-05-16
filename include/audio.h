#pragma once

#include <stdint.h>
#include <limits.h>

#include "pal.h"
#include "midi_parse.h"

// oscillator period is power of 2 so wrapping isn't an issue
#define OSC_PERIOD (1 << 20)
#define OSC_PERIOD_MASK (OSC_PERIOD - 1)
#define OSC_AMPLITUDE INT16_MAX
#define WAVE_SAMPLER_MAX_SAMPLES 20

struct effect_node;
struct filter_node;
struct oscillator;

/**
 * @brief Function to get sample from oscillator based on time t in the oscillator's period
 * NOTE: t must be in the range 0 to OSC_PERIOD
 * NOTE: the returned int value be between -OSC_AMPLITUDE and +OSC_AMPLITUDE
 *
 */
typedef int32_t (*oscillator_waveform_func_t)(uint32_t t);

/**
 * @brief Effect node update function, called every sample period
 * Used to perform operation on oscillator, things like changing frequency based on lfo
 *
 */
typedef void (*effect_node_update_func_t)(struct oscillator *, struct effect_node *);

/**
 * @brief Filters input and returns output to next filter in chain
 *
 */
typedef int32_t (*filter_process_func_t)(struct filter_node *, int32_t input_sample);

enum oscillator_voice_num {
    OSC_VOICE_NONE = -1,
    OSC_VOICE_0 = 0,
    OSC_VOICE_1,
    OSC_VOICE_2,
    OSC_VOICE_3,
    OSC_VOICE_4,
    OSC_VOICE_5,
    OSC_VOICE_6,
    OSC_MAX_VOICES
};

enum adsr_state {
    ADSR_STATE_ATTACK,
    ADSR_STATE_DECAY,
    ADSR_STATE_SUSTAIN,
    ADSR_STATE_RELEASE,
    ADSR_STATE_OFF
};

struct adsr_envelope {
    int32_t envelope_value;
    int16_t sustain;
    uint32_t attack_step;
    uint32_t decay_step;
    uint32_t release_step;
    enum adsr_state state;
};

struct oscillator_voice {
    int16_t amplitude;
    int32_t t;
    int32_t t_increment;
    struct adsr_envelope adsr;
};

struct effect_node {
    effect_node_update_func_t update;
    struct effect_node *next;
};

struct filter_node {
    filter_process_func_t process;
    struct filter_node *next;
};

struct oscillator {
    struct oscillator_voice voices[OSC_MAX_VOICES];

    struct effect_node *effect_list_head;
    struct filter_node *filter_list_head;
    oscillator_waveform_func_t waveform;
};

struct wave_data {
    const int16_t *data;
    uint32_t length;
};

struct wave_sample {
    const struct wave_data *wave_data;
    uint32_t pointer;
    uint16_t amplitude;
    bool playing;
};

struct wave_sampler {
    struct wave_sample samples[WAVE_SAMPLER_MAX_SAMPLES];
    struct filter_node *filter_list_head;
};

enum midi_channel_type {
    MIDI_CHANNEL_OSC,
    MIDI_CHANNEL_SAMPLER
};

struct midi_player {
    // one less oscillator channel because of the dedicated drum channel
    struct oscillator oscillators[MIDI_NUM_CHANNELS - 1];
    struct wave_sampler drums;
    int8_t channel_voice_to_note_mapping[MIDI_NUM_CHANNELS - 1][OSC_MAX_VOICES];
    int8_t note_to_drum_sample[128];

    struct midi_parser parser;
};

/**
 * @brief Initializes oscillator with given ADSR parameters and waveform function
 *
 * @param osc
 * @param attack_ms
 * @param decay_ms
 * @param sustain_value
 * @param release_ms
 * @param waveform
 */
void oscillator_init(struct oscillator *osc, uint32_t attack_ms, uint32_t decay_ms, int16_t sustain_value, uint32_t release_ms, oscillator_waveform_func_t waveform);

/**
 * @brief Deletes oscillator from active oscillators
 *
 * @param osc
 */
void oscillator_delete(struct oscillator *osc);

/**
 * @brief Plays tone on oscillator, returning the voice the tone is played on
 *
 * @param osc
 * @param amplitude
 * @param frequency
 * @return enum oscillator_voice_num
 */
enum oscillator_voice_num oscillator_play_voice(struct oscillator *osc, uint16_t amplitude, float frequency);

/**
 * @brief Stops given oscillator voice
 *
 * @param osc
 * @param voice
 */
void oscillator_stop_voice(struct oscillator *osc, enum oscillator_voice_num voice);

/**
 * @brief Changes frequency of oscillator voice to given frequency
 *
 * @param osc
 * @param voice
 * @param frequency
 */
void oscillator_change_voice_frequency(struct oscillator *osc, enum oscillator_voice_num voice, float frequency);

/**
 * @brief Adds filter to filter chain of oscillator
 *
 * @param osc
 * @param filter
 */
void oscillator_add_filter(struct oscillator *osc, struct filter_node *filter);

/**
 * @brief Adds effect node to effect chain on oscillator
 *
 * @param osc
 * @param effect
 */
void oscillator_add_effect(struct oscillator *osc, struct effect_node *effect);

/**
 * @brief Initializes midi player
 *
 * @param player
 */
void midi_player_init(struct midi_player *player);

/**
 * @brief Loads midi player with given midi source data
 *
 * @param player
 * @param midi_data_source
 */
void midi_player_load_midi(struct midi_player *player, uint8_t *midi_data_source);

/**
 * @brief Requests audio subsystem to stop outputing sound
 *
 */
void audio_request_stop();

/**
 * @brief Starts audio subsystem
 *
 */
void audio_start();

/**
 * @brief Sets master volume
 *
 * @param gain
 */
void audio_set_master_volume(float gain);