#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "pal.h"

#define MIDI_MAX_TRACKS 16
#define MIDI_NUM_CHANNELS 16
#define MIDI_DRUM_CHANNEL 9

struct midi_chunk_header {
    char type[4];
    // length field is big endian (_be)
    uint32_t length_be;
};

enum midi_status_code {
    MIDI_STATUS_NOTE_OFF =                0b1000,
    MIDI_STATUS_NOTE_ON =                 0b1001,
    MIDI_STATUS_POLYPHONIC_KEY_PRESSURE = 0b1010,
    MIDI_STATUS_CONTROL_CHANGE =          0b1011,
    MIDI_STATUS_PROGRAM_CHANGE =          0b1100,
    MIDI_STATUS_CHANNEL_PRESSURE =        0b1101,
    MIDI_STATUS_PITCH_WHEEL_CHANGE =      0b1110,
    MIDI_STATUS_SYSTEM =                  0b1111,
};

enum midi_track_state {
    TRACK_STATE_READ_DELTA,
    TRACK_STATE_WAIT_FOR_TIMER,
    TRACK_STATE_EVENT_PENDING,
};

enum midi_system_code {
    MIDI_SYSTEM_EXCLUSIVE =             0b0000,
    MIDI_SYSTEM_SONG_POSITION_POINTER = 0b0010,
    MIDI_SYSTEM_SONG_SELECT =           0b0011,
    MIDI_SYSTEM_TUNE_REQUEST =          0b0110,
    MIDI_SYSTEM_TIMING_CLOCK =          0b1000,
    MIDI_SYSTEM_START =                 0b1010,
    MIDI_SYSTEM_CONTINUE =              0b1011,
    MIDI_SYSTEM_STOP =                  0b1100,
    MIDI_SYSTEM_ACTIVE_SENSING =        0b1110,
    MIDI_SYSTEM_META_ESCAPE =           0b1111,
};

enum midi_meta_event_code {
    MIDI_META_EVENT_SEQ_NUM =                0x00,
    MIDI_META_EVENT_TEXT =                   0x01,
    MIDI_META_EVENT_COPYRIGHT =              0x02,
    MIDI_META_EVENT_SEQUENCE_TRACK_NAME =    0x03,
    MIDI_META_EVENT_INSTRUMENT_NAME =        0x04,
    MIDI_META_EVENT_LYRIC =                  0x05,
    MIDI_META_EVENT_MARKER =                 0x06,
    MIDI_META_EVENT_CUE_POINT =              0x07,
    MIDI_META_EVENT_CHANNEL_PREFIX =         0x20,
    MIDI_META_EVENT_END_OF_TRACK =           0x2F,
    MIDI_META_EVENT_SET_TEMPO =              0x51,
    MIDI_META_EVENT_SMPTE_OFFSET =           0x54,
    MIDI_META_EVENT_TIME_SIGNATURE =         0x58,
    MIDI_META_EVENT_KEY_SIGNATURE =          0x59,
    MIDI_META_EVENT_SEQUENCER_SPECIFIC =     0x7F,
};

enum midi_division_format {
    MIDI_FORMAT_TICKS_PER_QUARTER_NOTE = 0,
    MIDI_FORMAT_SMPTE = 1
};

enum midi_SMPTE_format {
    MIDI_SMPTE_24 =    0b00,
    MIDI_SMPTE_25 =    0b01,
    MIDI_SMPTE_29_97 = 0b10,
    MIDI_SMPTE_30 =    0b11
};

union status_byte {
    struct {
        enum midi_system_code midi_system_code : 4;
        enum midi_status_code status_code : 4;
    };
    struct {
        uint8_t channel : 4;
    };
};

struct track {
    uint8_t *pointer;
    int64_t timer; // nanosecond down counter until next event
    bool ended;
    union status_byte previous_status;
    uint8_t channel_prefix;
    enum midi_track_state state;
};

struct midi_event {
    union status_byte status;
    union {
        struct {
            uint8_t note;
            union {
                uint8_t velocity;
                uint8_t pressure;
            };
        };
        uint8_t program;
        struct {
            uint8_t number;
            uint8_t value;
        } controller;
        uint16_t pitch_wheel;
        uint16_t raw;
    };
    struct {
        uint8_t code;
        uint8_t length;
        uint8_t *data;
    } meta;
};

struct midi_parser {
    bool loop; // whether or not to loop midi playback when end is reached
    uint16_t num_tracks;
    uint32_t tempo_us_per_quarter_note;
    uint32_t ns_per_tick;
    union {
        struct {
            uint16_t ticks_per_frame : 8;
            uint16_t : 5;
            enum midi_SMPTE_format midi_SMPTE_format : 2;
        };
        struct {
            uint16_t ticks_per_quarter_note : 15;
        };
        struct {
            uint16_t : 15;
            enum midi_division_format format : 1;
        };
        uint16_t raw;
    } division;
    struct midi_chunk_header *header;
    struct midi_chunk_header *track_headers[MIDI_MAX_TRACKS];
    struct track tracks[MIDI_MAX_TRACKS];
};

/**
 * @brief Initializes midi parser
 *
 * @param parser
 * @param buffer pointer to midi file contents
 * @return true
 * @return false
 */
bool midi_parser_init(struct midi_parser *parser, void *buffer);

/**
 * @brief Restarts playback of midi parser
 *
 * @param parser
 */
void midi_parser_restart(struct midi_parser *parser);

/**
 * @brief Advances parser by given time delta in nanoseconds
 *
 * @param parser
 * @param delta_time_ns
 * @return true event needs to be read
 * @return false no events pending
 */
bool midi_parser_advance(struct midi_parser *parser, uint32_t delta_time_ns);

/**
 * @brief Gets pending event from parser
 *
 * @param parser
 * @return true more events need to be read
 * @return false
 */
bool midi_parser_next_event(struct midi_parser *parser, struct midi_event *event);

/**
 * @brief Function to convert note number to frequency in Hz
 *
 * @param note_number Note number from midi event
 * @return pal_float_t frequency [Hz]
 */
pal_float_t midi_parser_note_frequency(uint8_t note_number);

/**
 * @brief Check if the parser has completed (all tracks have ended)
 *
 * @param parser
 * @return true if all tracks have ended
 * @return false
 */
bool midi_parser_ended(struct midi_parser *parser);

/**
 * @brief Sets track offset to given initial time offset value in seconds
 *
 * @param parser
 * @param track_num
 * @param initial_value initial time offset in seconds
 */
void midi_parser_set_track_offset(struct midi_parser *parser, int track_num, pal_float_t initial_value);
