#include "audio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "pal.h"
#include "mathutils.h"

#define MAX_NUM_MIDI_PLAYERS 10
#define MAX_NUM_OSCILLATORS 100
// #define PRINT_MIDI_LYRICS

#define ENVELOPE_VALUE_SCALING 10

uint64_t current_sample_num = 0;
struct polyphonic_wave_sampler {
    const struct wave_data *wave_data;
    struct channel {
        uint32_t pointer;
        uint16_t amplitude;
        bool playing;
    } channels[MAX_CONCURRENT_SAMPLE_VOICES];
};


static struct midi_player *midi_players[MAX_NUM_MIDI_PLAYERS];
static struct oscillator *oscillators[MAX_NUM_OSCILLATORS];
static struct polyphonic_wave_sampler wave_samplers[MAX_POLYPHONIC_WAVE_SAMPLERS];
struct filter_node *wave_samplers_filter_list_head;
struct filter_node *master_filter_head;

static int num_midi_players = 0;
static int num_oscillators = 0;
static bool running = false;

static pal_float_t master_gain = 1.0;
static pal_float_t dynamic_mixer_gain = 1.0;
static int32_t dynamic_mixer_timer = 0;

static void add_oscillator(struct oscillator *osc_to_add) {
    if (num_oscillators < MAX_NUM_OSCILLATORS)
        oscillators[num_oscillators++] = osc_to_add;
}

void oscillator_delete(struct oscillator *osc) {
    for (int i = 0; i < num_oscillators; i++) {
        if (oscillators[i] == osc) {
            // set current oscillator to the last oscillator in the list, decrement number of oscillators
            oscillators[i] = oscillators[num_oscillators--];
            return;
        }
    }
    // oscillator not found, who cares
}

wave_sample_t wave_sample_register(const struct wave_data *wave_data) {
    for (int i = 0; i < MAX_POLYPHONIC_WAVE_SAMPLERS; i++) {
        if (wave_samplers[i].wave_data == NULL) {
            wave_samplers[i].wave_data = wave_data;

            for (int j = 0; j < MAX_CONCURRENT_SAMPLE_VOICES; j++) {
                wave_samplers[i].channels[j].pointer = 0;
                wave_samplers[i].channels[j].playing = false;
            }

            return (wave_sample_t) i;
        }
    }

    return (wave_sample_t) -1;
}

void wave_sample_play(wave_sample_t sample, uint16_t amplitude) {
    if (sample == -1 || wave_samplers[sample].wave_data == NULL)
        return;

    // find free sample voice and play it
    for (int8_t i = 0; i < MAX_CONCURRENT_SAMPLE_VOICES; i++) {
        if (!wave_samplers[sample].channels[i].playing) {
            wave_samplers[sample].channels[i].playing = true;
            wave_samplers[sample].channels[i].pointer = 0;
            wave_samplers[sample].channels[i].amplitude = amplitude;
            return;
        }
    }
}

void oscillator_change_voice_frequency(struct oscillator *osc, enum oscillator_voice_num voice, pal_float_t frequency) {
    osc->voices[voice].t_increment = OSC_PERIOD * frequency / PAL_AUDIO_SAMPLE_RATE;
}

enum oscillator_voice_num oscillator_play_voice(struct oscillator *osc, uint16_t amplitude, pal_float_t frequency) {
    // find voice to play
    for (enum oscillator_voice_num v = OSC_VOICE_0; v < OSC_MAX_VOICES; v++) {
        if (osc->voices[v].adsr.state == ADSR_STATE_OFF) {
            // start playing this voice
            osc->voices[v].adsr.state = ADSR_STATE_ATTACK;
            osc->voices[v].amplitude = amplitude;
            oscillator_change_voice_frequency(osc, v, frequency);
            return v;
        }
    }

    return OSC_VOICE_NONE;
}

void oscillator_stop_voice(struct oscillator *osc, enum oscillator_voice_num voice) {
    if (osc->voices[voice].adsr.state != ADSR_STATE_OFF)
        osc->voices[voice].adsr.state = ADSR_STATE_RELEASE;
}

void oscillator_init(struct oscillator *osc, uint32_t attack_ms, uint32_t decay_ms, int16_t sustain_value, uint32_t release_ms, oscillator_waveform_func_t waveform) {
    for (enum oscillator_voice_num v = OSC_VOICE_0; v < OSC_MAX_VOICES; v++) {
        osc->voices[v].adsr.state = ADSR_STATE_OFF;
        osc->voices[v].adsr.envelope_value = 0;
        osc->voices[v].adsr.attack_step = ENVELOPE_VALUE_SCALING * (OSC_AMPLITUDE * 1000) / (attack_ms * PAL_AUDIO_SAMPLE_RATE);
        osc->voices[v].adsr.decay_step = ENVELOPE_VALUE_SCALING * ((OSC_AMPLITUDE - sustain_value) * 1000) / (decay_ms * PAL_AUDIO_SAMPLE_RATE);
        osc->voices[v].adsr.sustain = ENVELOPE_VALUE_SCALING * sustain_value;
        osc->voices[v].adsr.release_step = ENVELOPE_VALUE_SCALING * (sustain_value * 1000) / (release_ms * PAL_AUDIO_SAMPLE_RATE);
    }

    osc->waveform = waveform;
    osc->effect_list_head = NULL;
    osc->filter_list_head = NULL;

    add_oscillator(osc);
}

void wave_sampler_init() {
    for (int i = 0; i < MAX_POLYPHONIC_WAVE_SAMPLERS; i++) {
        wave_samplers[i].wave_data == NULL;

        for (int j = 0; j < MAX_CONCURRENT_SAMPLE_VOICES; j++) {
            wave_samplers[i].channels[j].amplitude = 0;
            wave_samplers[i].channels[j].pointer = 0;
            wave_samplers[i].channels[j].playing = false;
        }
    }
}

void oscillator_add_effect(struct oscillator *osc, struct effect_node *effect) {
    if (osc->effect_list_head == NULL) {
        osc->effect_list_head = effect;
    } else {
        // add effect on the end of list
        struct effect_node *node = osc->effect_list_head;
        while (node->next != NULL) node = node->next;
        // node now points to the tail
        node->next = effect;
    }
}

void oscillator_add_filter(struct oscillator *osc, struct filter_node *filter) {
    if (osc->filter_list_head == NULL) {
        osc->filter_list_head = filter;
    } else {
        // add filter on the end of list
        struct filter_node *node = osc->filter_list_head;
        while (node->next != NULL) node = node->next;
        // node now points to the tail
        node->next = filter;
    }
}

static void advance_adsr_envelope(struct adsr_envelope *adsr) {
    switch (adsr->state) {
        case ADSR_STATE_ATTACK:
            adsr->envelope_value += adsr->attack_step;
            if (adsr->envelope_value >= OSC_AMPLITUDE) {
                adsr->envelope_value = OSC_AMPLITUDE;
                adsr->state = ADSR_STATE_DECAY;
            }
            break;
        case ADSR_STATE_DECAY:
            adsr->envelope_value -= adsr->decay_step;
            if (adsr->envelope_value <= adsr->sustain) {
                adsr->envelope_value = adsr->sustain;
                adsr->state = ADSR_STATE_SUSTAIN;
            }
            break;
        case ADSR_STATE_SUSTAIN:
            adsr->envelope_value = adsr->sustain;
            break;
        case ADSR_STATE_RELEASE:
            adsr->envelope_value -= adsr->release_step;
            if (adsr->envelope_value <= 0) {
                adsr->envelope_value = 0;
                adsr->state = ADSR_STATE_OFF;
            }
            break;
        case ADSR_STATE_OFF:
        default:
            break;
    }
}

static int32_t run_filter_chain(struct filter_node *filter_list_head, int32_t input_sample) {
    struct filter_node *filter = filter_list_head;
    int32_t sample = input_sample;

    while (filter != NULL) {
        sample = filter->process(filter, sample);
        filter = filter->next;
    }

    return sample;
}

static int32_t oscillator_get_sample_and_advance(struct oscillator *osc) {
    int32_t sample = 0;

    // run effects
    struct effect_node *effect = osc->effect_list_head;
    while (effect != NULL) {
        effect->update(osc, effect);
        effect = effect->next;
    }

    for (enum oscillator_voice_num v = OSC_VOICE_0; v < OSC_MAX_VOICES; v++) {
        if (osc->voices[v].adsr.state == ADSR_STATE_OFF)
            continue;

        sample += (osc->waveform(osc->voices[v].t) * osc->voices[v].adsr.envelope_value / ENVELOPE_VALUE_SCALING) / OSC_AMPLITUDE * osc->voices[v].amplitude / OSC_AMPLITUDE;
        osc->voices[v].t = (osc->voices[v].t + osc->voices[v].t_increment) & OSC_PERIOD_MASK;

        advance_adsr_envelope(&osc->voices[v].adsr);
    }

    sample = run_filter_chain(osc->filter_list_head, sample);

    return sample;
}

static int32_t wave_sampler_get_sample_and_advance() {
    int32_t sample = 0;
    int32_t voice_sample;

    for (int i = 0; i < MAX_POLYPHONIC_WAVE_SAMPLERS; i++) {
        if (wave_samplers[i].wave_data == NULL)
            continue;

        voice_sample = 0;

        for (int j = 0; j < MAX_CONCURRENT_SAMPLE_VOICES; j++) {
            if (!wave_samplers[i].channels[j].playing)
                continue;

            voice_sample += wave_samplers[i].wave_data->data[wave_samplers[i].channels[j].pointer++] * wave_samplers[i].channels[j].amplitude / OSC_AMPLITUDE;

            if (wave_samplers[i].channels[j].pointer >= wave_samplers[i].wave_data->length)
                wave_samplers[i].channels[j].playing = false;
        }

        sample += voice_sample;
    }

    return run_filter_chain(wave_samplers_filter_list_head, sample);
}

static void add_midi_player(struct midi_player *player_to_add) {
    if (num_midi_players < MAX_NUM_MIDI_PLAYERS)
        midi_players[num_midi_players++] = player_to_add;
}

void midi_player_delete(struct midi_player *player) {
    for (int i = 0; i < num_midi_players; i++) {
        if (midi_players[i] == player) {
            // set current player to the last player in the list, decrement number of players
            midi_players[i] = midi_players[num_midi_players--];
            return;
        }
    }
    // player not found, who cares
}

int32_t square_wave(uint32_t t) {
    return (t < OSC_PERIOD / 2) ? OSC_AMPLITUDE : -OSC_AMPLITUDE;
}

void midi_player_assign_drum_sample_to_note(struct midi_player *player, wave_sample_t sample, int note_number) {
    player->drum_samples[note_number] = sample;
}

void midi_player_init(struct midi_player *player) {
    for (int i = 0; i < MIDI_NUM_CHANNELS - 1; i++) {
        oscillator_init(&player->oscillators[i], 10, 100, OSC_AMPLITUDE * 0.9, 100, &square_wave);
        player->channel_transpose[i] = 0;
    }

    wave_sampler_init();

    memset(player->drum_samples, WAVE_SAMPLE_INVALID, NUM_DRUM_NOTES * sizeof(wave_sample_t));

    add_midi_player(player);
}

struct oscillator *midi_player_get_channel_oscillator(struct midi_player *player, uint8_t channel) {
    if (channel > MIDI_DRUM_CHANNEL)
        channel--;

    return &player->oscillators[channel];
}

void midi_player_load_midi(struct midi_player *player, uint8_t *midi_data_source) {
    midi_parser_init(&player->parser, midi_data_source);
}

static void midi_channel_note_on(struct midi_player *player, uint8_t channel, uint8_t note_number, uint8_t velocity) {
    uint16_t amplitude = (OSC_AMPLITUDE * velocity) / 127;

    if (channel == MIDI_DRUM_CHANNEL) {
        if (player->drum_samples[note_number] != WAVE_SAMPLE_INVALID)
            wave_sample_play(player->drum_samples[note_number], amplitude);
        else
            printf("unassigned drum sample on note %d! maybe make it hehe\n", note_number);
    } else {
        if (channel > MIDI_DRUM_CHANNEL)
            channel--;

        enum oscillator_voice_num voice = oscillator_play_voice(&player->oscillators[channel], amplitude, midi_parser_note_frequency(note_number + player->channel_transpose[channel]));

        if (voice == OSC_VOICE_NONE)
            return; // no voices left! skip note

        // keep track of what note the voice is playing
        player->channel_voice_to_note_mapping[channel][voice] = note_number;
    }
}

static void midi_channel_note_off(struct midi_player *player, uint8_t channel, uint8_t note_number) {
    if (channel == MIDI_DRUM_CHANNEL) {
        return;
    } else {
        if (channel > MIDI_DRUM_CHANNEL)
            channel--;

        for (enum oscillator_voice_num v = OSC_VOICE_0; v < OSC_MAX_VOICES; v++) {
            if (player->channel_voice_to_note_mapping[channel][v] == note_number) {
                oscillator_stop_voice(&player->oscillators[channel], v);
            }
        }
    }
}

static void midi_player_advance(struct midi_player *player, uint32_t delta_time_ns) {
    struct midi_event event;
    struct oscillator *osc;

    // return if no events to parse
    if (!midi_parser_advance(&player->parser, delta_time_ns))
        return;

    while (midi_parser_next_event(&player->parser, &event)) {
        if (event.status.status_code == MIDI_STATUS_NOTE_ON) {
            midi_channel_note_on(player, event.status.channel, event.note, event.velocity);
        } else if (event.status.status_code == MIDI_STATUS_NOTE_OFF) {
            midi_channel_note_off(player, event.status.channel, event.note);
#if defined(PRINT_MIDI_LYRICS)
        } else if (event.status.midi_system_code == MIDI_SYSTEM_META_ESCAPE && event.meta.code == MIDI_META_EVENT_LYRIC) {
            printf("%*s", event.meta.length, (char *) event.meta.data);
            if (event.meta.data[event.meta.length-1] == '\r')
                putc('\n', stdout);
            fflush(stdout);
#endif
        }
    }
}

void midi_player_set_channel_transpose(struct midi_player *player, int channel, int8_t transpose) {
    if (channel > MIDI_DRUM_CHANNEL)
        channel--;

    player->channel_transpose[channel] = transpose;
}

static void advance_all_midi_players() {
    for (int i = 0; i < num_midi_players; i++) {
        midi_player_advance(midi_players[i], (uint32_t) pal_round(1000000000 / PAL_AUDIO_SAMPLE_RATE));
    }
}

void audio_set_master_volume(pal_float_t gain) {
    master_gain = gain;
}

void audio_add_filter(struct filter_node *filter) {
    if (master_filter_head == NULL) {
        master_filter_head = filter;
    } else {
        // add filter on the end of list
        struct filter_node *node = master_filter_head;
        while (node->next != NULL) node = node->next;
        // node now points to the tail
        node->next = filter;
    }
}

static void audio_fill_buffer(audio_sample_t *samples, int num_samples) {
    int32_t sample;
    int32_t current_sample;
    struct oscillator *osc;

    // add samples to buffer
    for (int i = 0; i < num_samples; i++) {
        if (running)
            advance_all_midi_players();

        current_sample = wave_sampler_get_sample_and_advance();

        for (int osc_i = 0; osc_i < num_oscillators; osc_i++) {
            current_sample += oscillator_get_sample_and_advance(oscillators[osc_i]);
        }

        // gotta figure out this mixer weirdness to balance all sounds
        current_sample = run_filter_chain(master_filter_head, current_sample) * master_gain;

        if (current_sample > OSC_AMPLITUDE)
            current_sample = OSC_AMPLITUDE;
        if (current_sample < -OSC_AMPLITUDE)
            current_sample = -OSC_AMPLITUDE;

        current_sample = current_sample * (AUDIO_SAMPLE_MAX / 2) / OSC_AMPLITUDE + (AUDIO_SAMPLE_MAX / 2);

        samples[i] = current_sample;
    }
}

void audio_request_stop() {
    bool oscillators_active = true;
    running = false;

    for (int i = 0; i < num_oscillators; i++) {
        for (enum oscillator_voice_num v = OSC_VOICE_0; v < OSC_MAX_VOICES; v++) {
            oscillators[i]->voices[v].adsr.state = ADSR_STATE_RELEASE;
        }
    }

    for (int i = 0; i < MAX_POLYPHONIC_WAVE_SAMPLERS; i++) {
        if (wave_samplers[i].wave_data == NULL)
            continue;

        for (int j = 0; j < MAX_CONCURRENT_SAMPLE_VOICES; j++)
            wave_samplers[i].channels[j].playing = false;
    }

    while (oscillators_active) {
        oscillators_active = false;

        for (int i = 0; i < num_oscillators; i++) {
            for (enum oscillator_voice_num v = OSC_VOICE_0; v < OSC_MAX_VOICES; v++) {
                if (oscillators[i]->voices[v].adsr.state != ADSR_STATE_OFF)
                    oscillators_active = true;
            }
        }
    }
}

void audio_start() {
    running = true;
    pal_set_audio_callback(&audio_fill_buffer);
}