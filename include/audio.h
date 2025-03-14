#pragma once

#include <stdint.h>
#include <limits.h>

#include "pal.h"
#include "midi_parse.h"

// oscillator period is power of 2 so wrapping isn't an issue
#define OSC_PERIOD (1 << 18)
#define OSC_PERIOD_MASK (OSC_PERIOD - 1)
#define OSC_AMPLITUDE INT16_MAX
#define MAX_POLYPHONIC_WAVE_SAMPLERS (64)
#define MAX_CONCURRENT_SAMPLE_VOICES (4)
#define NUM_DRUM_NOTES (128)

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

typedef int8_t wave_sample_t;
#define WAVE_SAMPLE_INVALID ((wave_sample_t) -1)

_Static_assert(MAX_POLYPHONIC_WAVE_SAMPLERS <= ((1 << ((sizeof(wave_sample_t) << 3) - 1)) - 1), "MAX_POLYPHONIC_WAVE_SAMPLERS must fit into wave_sample_t");

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

enum midi_channel_type {
    MIDI_CHANNEL_OSC,
    MIDI_CHANNEL_SAMPLER
};

struct midi_player {
    // one less oscillator channel because of the dedicated drum channel
    struct oscillator oscillators[MIDI_NUM_CHANNELS - 1];
    int8_t channel_voice_to_note_mapping[MIDI_NUM_CHANNELS - 1][OSC_MAX_VOICES];
    wave_sample_t drum_samples[NUM_DRUM_NOTES];
    int8_t channel_transpose[MIDI_NUM_CHANNELS - 1];

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
enum oscillator_voice_num oscillator_play_voice(struct oscillator *osc, uint16_t amplitude, pal_float_t frequency);

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
void oscillator_change_voice_frequency(struct oscillator *osc, enum oscillator_voice_num voice, pal_float_t frequency);

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
 * @brief Adds filter to master audio filter chain
 *
 * This filter is run on the final output of audio synthesis
 *
 * @param filter
 */
void audio_add_filter(struct filter_node *filter);

/**
 * @brief Registers wave sample with global wave sampler to be played later
 * with wave_sample_play
 *
 * @param wave_data Pointer to wave file from assets directory
 * @return wave_sample_t identifier to use with play/pause functions
 */
wave_sample_t wave_sample_register(const struct wave_data *wave_data);

/**
 * @brief Plays registered wave sample
 *
 * @param sample
 * @param amplitude Amplitude of wave sample
 * @param speed Speed multiplier of sample playback
 */
void wave_sample_play(wave_sample_t sample, uint16_t amplitude, pal_float_t speed);

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
 * @brief Assigns drum wave sample data to drum note number in midi player
 *
 * @param player
 * @param sample Sample ID returned from wave_sample_register
 * @param note_number MIDI note number to map sample to
 */
void midi_player_assign_drum_sample_to_note(struct midi_player *player, wave_sample_t sample, int note_number);

/**
 * @brief Sets transpose (in note number/half steps) of given midi channel
 *
 * @param player
 * @param channel
 * @param transpose
 * @return true
 * @return false
 */
void midi_player_set_channel_transpose(struct midi_player *player, int channel, int8_t transpose);

/**
 * @brief Gets channel oscillator from midi_player for given channel
 *
 * @param player
 * @param channel
 * @return struct oscillator*
 */
struct oscillator *midi_player_get_channel_oscillator(struct midi_player *player, uint8_t channel);

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
void audio_set_master_volume(pal_float_t gain);