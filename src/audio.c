#include "audio.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

#include "pal.h"

#define MAX_NUM_MIDI_PLAYERS 10
#define MAX_NUM_OSCILLATORS 100
#define MAX_NUM_WAVE_SAMPLERS 100

static struct midi_player *midi_players[MAX_NUM_MIDI_PLAYERS];
static struct oscillator *oscillators[MAX_NUM_OSCILLATORS];
static struct wave_sampler *wave_samplers[MAX_NUM_WAVE_SAMPLERS];
static int num_midi_players = 0;
static int num_oscillators = 0;
static int num_wave_samplers = 0;
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

static void add_wave_sampler(struct wave_sampler *wav_to_add) {
    if (num_wave_samplers < MAX_NUM_WAVE_SAMPLERS)
        wave_samplers[num_wave_samplers++] = wav_to_add;
}

void wave_sampler_delete(struct wave_sampler *wav) {
    for (int i = 0; i < num_wave_samplers; i++) {
        if (wave_samplers[i] == wav) {
            // set current wave sampler to the last wave sampler in the list, decrement number of wave samplers
            wave_samplers[i] = wave_samplers[num_wave_samplers--];
            return;
        }
    }
    // wave sampler not found, who cares
}

void oscillator_change_voice_frequency(struct oscillator *osc, enum oscillator_voice_num voice, pal_float_t frequency) {
    osc->voices[voice].t_increment = OSC_PERIOD * frequency / PAL_AUDIO_SAMPLE_RATE;
}

enum oscillator_voice_num oscillator_play_voice(struct oscillator *osc, uint16_t amplitude, pal_float_t frequency) {
    // find voice to play
    for (enum oscillator_voice_num v = OSC_VOICE_0; v < OSC_MAX_VOICES; v++) {
        if (osc->voices[v].adsr.state == ADSR_STATE_OFF) {
            // start playing this voice
            osc->voices[v].t_increment = OSC_PERIOD * frequency / PAL_AUDIO_SAMPLE_RATE;
            osc->voices[v].adsr.state = ADSR_STATE_ATTACK;
            osc->voices[v].amplitude = amplitude;
            return v;
        }
    }

    return OSC_VOICE_NONE;
}

void oscillator_stop_voice(struct oscillator *osc, enum oscillator_voice_num voice) {
    if (osc->voices[voice].adsr.state != ADSR_STATE_OFF)
        osc->voices[voice].adsr.state = ADSR_STATE_RELEASE;
}

void wave_sampler_play_sample(struct wave_sampler *wav, int sample, uint16_t amplitude) {
    wav->samples[sample].amplitude = amplitude;
    wav->samples[sample].pointer = 0;
    wav->samples[sample].playing = true;
}

void wave_sampler_stop_sample(struct wave_sampler *wav, int sample) {
    wav->samples[sample].playing = false;
}

void oscillator_init(struct oscillator *osc, uint32_t attack_ms, uint32_t decay_ms, int16_t sustain_value, uint32_t release_ms, oscillator_waveform_func_t waveform) {
    for (enum oscillator_voice_num v = OSC_VOICE_0; v < OSC_MAX_VOICES; v++) {
        osc->voices[v].adsr.state = ADSR_STATE_OFF;
        osc->voices[v].adsr.envelope_value = 0;
        osc->voices[v].adsr.attack_step = (OSC_AMPLITUDE * 1000) / (attack_ms * PAL_AUDIO_SAMPLE_RATE);
        osc->voices[v].adsr.decay_step = ((OSC_AMPLITUDE - sustain_value) * 1000) / (decay_ms * PAL_AUDIO_SAMPLE_RATE);
        osc->voices[v].adsr.sustain = sustain_value;
        osc->voices[v].adsr.release_step = (sustain_value * 1000) / (release_ms * PAL_AUDIO_SAMPLE_RATE);
    }

    osc->waveform = waveform;
    osc->effect_list_head = NULL;
    osc->filter_list_head = NULL;

    add_oscillator(osc);
}

void wave_sampler_init(struct wave_sampler *wav) {
    for (int i = 0; i < WAVE_SAMPLER_MAX_SAMPLES; i++) {
        wav->samples[i].playing = false;
        wav->samples[i].wave_data == NULL;
    }

    add_wave_sampler(wav);
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

        sample += (osc->waveform(osc->voices[v].t) * osc->voices[v].adsr.envelope_value) / OSC_AMPLITUDE * osc->voices[v].amplitude / OSC_AMPLITUDE;
        osc->voices[v].t = (osc->voices[v].t + osc->voices[v].t_increment) & OSC_PERIOD_MASK;

        advance_adsr_envelope(&osc->voices[v].adsr);
    }

    sample = run_filter_chain(osc->filter_list_head, sample);

    // scale sample by active voices to leave room each voice
    return sample / OSC_MAX_VOICES / 2;
}

static int32_t wave_sampler_get_sample_and_advance(struct wave_sampler *wav) {
    int32_t sample = 0;

    for (int i = 0; i < WAVE_SAMPLER_MAX_SAMPLES; i++) {
        if (!wav->samples[i].playing || wav->samples[i].wave_data == NULL)
            continue;

        sample += wav->samples[i].wave_data->data[wav->samples[i].pointer++] * wav->samples[i].amplitude / OSC_AMPLITUDE;

        if (wav->samples[i].pointer >= wav->samples[i].wave_data->length)
            wav->samples[i].playing = false;
    }

    sample = run_filter_chain(wav->filter_list_head, sample);

    return sample / 10;
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

static void midi_player_assign_drum_sample_to_note(struct midi_player *player, uint8_t note_number, int sample_number, const struct wave_data *wave_data) {
    player->note_to_drum_sample[note_number] = sample_number;
    player->drums.samples[sample_number].wave_data = wave_data;
}

void midi_player_init(struct midi_player *player) {
    for (int i = 0; i < MIDI_NUM_CHANNELS - 1; i++) {
        oscillator_init(&player->oscillators[i], 1, 100, OSC_AMPLITUDE * 0.9, 1, &square_wave);
    }

    wave_sampler_init(&player->drums);

    memset(player->note_to_drum_sample, -1, sizeof(player->note_to_drum_sample));

    // add da samples!
    // midi_player_assign_drum_sample_to_note(player, 38, 0, &snare_wav);
    // midi_player_assign_drum_sample_to_note(player, 40, 0, &snare_wav);
    // midi_player_assign_drum_sample_to_note(player, 35, 1, &kick_wav);
    // midi_player_assign_drum_sample_to_note(player, 36, 1, &kick_wav);
    // midi_player_assign_drum_sample_to_note(player, 49, 2, &crash_wav);
    // midi_player_assign_drum_sample_to_note(player, 52, 2, &crash_wav);
    // midi_player_assign_drum_sample_to_note(player, 55, 2, &crash_wav);
    // midi_player_assign_drum_sample_to_note(player, 44, 3, &closed_hat_wav);
    // midi_player_assign_drum_sample_to_note(player, 42, 3, &closed_hat_wav);
    // midi_player_assign_drum_sample_to_note(player, 46, 4, &open_hat_wav);
    // midi_player_assign_drum_sample_to_note(player, 56, 5, &cowbell_wav);
    // midi_player_assign_drum_sample_to_note(player, 62, 5, &cowbell_wav);
    // midi_player_assign_drum_sample_to_note(player, 54, 5, &cowbell_wav);

    add_midi_player(player);
}

void midi_player_load_midi(struct midi_player *player, uint8_t *midi_data_source) {
    midi_parser_init(&player->parser, midi_data_source);
}

static void midi_channel_note_on(struct midi_player *player, uint8_t channel, uint8_t note_number, uint8_t velocity) {
    uint16_t amplitude = (OSC_AMPLITUDE * velocity) / 127;

    if (channel == MIDI_DRUM_CHANNEL) {
        if (player->note_to_drum_sample[note_number] > -1)
            wave_sampler_play_sample(&player->drums, player->note_to_drum_sample[note_number], amplitude);
        else
            printf("unknown drum sample!! maybe make it hehe\n");
    } else {
        if (channel > MIDI_DRUM_CHANNEL)
            channel--;

        enum oscillator_voice_num voice = oscillator_play_voice(&player->oscillators[channel], amplitude, midi_parser_note_frequency(note_number));

        if (voice == OSC_VOICE_NONE)
            return; // no voices left! skip note

        // keep track of what note the voice is playing
        player->channel_voice_to_note_mapping[channel][voice] = note_number;
    }
}

static void midi_channel_note_off(struct midi_player *player, uint8_t channel, uint8_t note_number) {
    if (channel == MIDI_DRUM_CHANNEL) {
        if (player->note_to_drum_sample[note_number] > -1)
            wave_sampler_stop_sample(&player->drums, player->note_to_drum_sample[note_number]);
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

static void midi_player_advance(struct midi_player *player, uint32_t delta_time_us) {
    struct midi_event event;
    struct oscillator *osc;

    // return if no events to parse
    if (!midi_parser_advance(&player->parser, delta_time_us))
        return;

    while (midi_parser_next_event(&player->parser, &event)) {
        if (event.status.status_code == MIDI_STATUS_NOTE_ON) {
            midi_channel_note_on(player, event.status.channel, event.note, event.velocity);
        } else if (event.status.status_code == MIDI_STATUS_NOTE_OFF) {
            midi_channel_note_off(player, event.status.channel, event.note);
        }
    }
}

static void advance_all_midi_players() {
    for (int i = 0; i < num_midi_players; i++) {
        midi_player_advance(midi_players[i], 1000000 / PAL_AUDIO_SAMPLE_RATE);
    }
}

void audio_set_master_volume(pal_float_t gain) {
    master_gain = gain;
}

static void audio_fill_buffer(audio_sample_t *samples, int num_samples) {
    int32_t sample;
    int32_t current_sample;
    struct oscillator *osc;

    // add samples to buffer
    for (int i = 0; i < num_samples; i++) {
        if (running)
            advance_all_midi_players();

        current_sample = 0;

        for (int osc_i = 0; osc_i < num_oscillators; osc_i++) {
            sample = oscillator_get_sample_and_advance(oscillators[osc_i]);

            if (sample == 0)
                continue;

            current_sample += sample;
        }

        for (int wav_i = 0; wav_i < num_wave_samplers; wav_i++) {
            sample = wave_sampler_get_sample_and_advance(wave_samplers[wav_i]);

            if (sample == 0)
                continue;

            current_sample += sample;
        }

        // gotta figure out this mixer weirdness to balance all sounds
        current_sample *= master_gain;

        // if (amplitude * dynamic_mixer_gain > OSC_AMPLITUDE || amplitude * dynamic_mixer_gain < -OSC_AMPLITUDE) {
        //     dynamic_mixer_gain = fabs((pal_float_t) OSC_AMPLITUDE / amplitude);
        //     dynamic_mixer_timer = PAL_AUDIO_SAMPLE_RATE * 2;
        // }

        // current_sample *= dynamic_mixer_gain;

        // if (dynamic_mixer_timer <= 0) {
        //     // timer expired, increase mixer gain
        //     dynamic_mixer_gain += 0.001;
        // } else {
        //     dynamic_mixer_timer--;
        // }

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