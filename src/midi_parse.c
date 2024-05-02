#include "midi_parse.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define MIDI_DEFAULT_TEMPO_BPM (120)
#define BPM_TO_US_PER_QUARTER_NOTE(bpm) ((60 * 1000000) / bpm)

// uncomment to print track information
// #define PRINT_EVENTS

#ifdef PRINT_EVENTS
#define TRACK_PRINT(track_num, fmt, ...) printf("[Track%d]: " fmt, track_num, ##__VA_ARGS__)
#else
#define TRACK_PRINT(track_num, fmt, ...)
#endif

static const float SMPTE_frames_per_second_map[] = {
    [MIDI_SMPTE_24] =    24.0f,
    [MIDI_SMPTE_25] =    25.0f,
    [MIDI_SMPTE_29_97] = 29.97f,
    [MIDI_SMPTE_30] =    30.0f
};

// note frequency lookup table
static const float note_frequencies[] = {
    [127] = 12543.85,
    [126] = 11839.82,
    [125] = 11175.30,
    [124] = 10548.08,
    [123] = 9956.06,
    [122] = 9397.27,
    [121] = 8869.84,
    [120] = 8372.02,
    [119] = 7902.13,
    [118] = 7458.62,
    [117] = 7040.00,
    [116] = 6644.88,
    [115] = 6271.93,
    [114] = 5919.91,
    [113] = 5587.65,
    [112] = 5274.04,
    [111] = 4978.03,
    [110] = 4698.64,
    [109] = 4434.92,
    [108] = 4186.01,
    [107] = 3951.07,
    [106] = 3729.31,
    [105] = 3520.00,
    [104] = 3322.44,
    [103] = 3135.96,
    [102] = 2959.96,
    [101] = 2793.83,
    [100] = 2637.02,
    [99] = 	2489.02,
    [98] = 	2349.32,
    [97] = 	2217.46,
    [96] = 	2093.00,
    [95] = 	1975.53,
    [94] = 	1864.66,
    [93] = 	1760.00,
    [92] = 	1661.22,
    [91] = 	1567.98,
    [90] = 	1479.98,
    [89] = 	1396.91,
    [88] = 	1318.51,
    [87] = 	1244.51,
    [86] = 	1174.66,
    [85] = 	1108.73,
    [84] = 	1046.50,
    [83] = 	987.77,
    [82] = 	932.33,
    [81] = 	880.00,
    [80] = 	830.61,
    [79] = 	783.99,
    [78] = 	739.99,
    [77] = 	698.46,
    [76] = 	659.26,
    [75] = 	622.25,
    [74] = 	587.33,
    [73] = 	554.37,
    [72] = 	523.25,
    [71] = 	493.88,
    [70] = 	466.16,
    [69] =  440.00,
    [68] = 	415.30,
    [67] = 	392.00,
    [66] = 	369.99,
    [65] = 	349.23,
    [64] = 	329.63,
    [63] = 	311.13,
    [62] = 	293.66,
    [61] = 	277.18,
    [60] = 	261.63,
    [59] = 	246.94,
    [58] = 	233.08,
    [57] = 	220.00,
    [56] = 	207.65,
    [55] = 	196.00,
    [54] = 	185.00,
    [53] = 	174.61,
    [52] = 	164.81,
    [51] = 	155.56,
    [50] = 	146.83,
    [49] = 	138.59,
    [48] = 	130.81,
    [47] = 	123.47,
    [46] = 	116.54,
    [45] = 	110.00,
    [44] = 	103.83,
    [43] = 	98.00,
    [42] = 	92.50,
    [41] = 	87.31,
    [40] = 	82.41,
    [39] = 	77.78,
    [38] = 	73.42,
    [37] = 	69.30,
    [36] = 	65.41,
    [35] = 	61.74,
    [34] = 	58.27,
    [33] = 	55.00,
    [32] = 	51.91,
    [31] = 	49.00,
    [30] = 	46.25,
    [29] = 	43.65,
    [28] = 	41.20,
    [27] = 	38.89,
    [26] = 	36.71,
    [25] = 	34.65,
    [24] = 	32.70,
    [23] = 	30.87,
    [22] = 	29.14,
    [21] = 	27.50,
    [20] = 	25.96,
    [19] = 	24.50,
    [18] = 	23.12,
    [17] = 	21.83,
    [16] = 	20.60,
    [15] = 	19.45,
    [14] = 	18.35,
    [13] = 	17.32,
    [12] = 	16.35,
    [11] = 	15.43,
    [10] = 	14.57,
    [9] = 	13.75,
    [8] = 	12.98,
    [7] = 	12.25,
    [6] = 	11.56,
    [5] = 	10.91,
    [4] = 	10.30,
    [3] = 	9.72,
    [2] = 	9.18,
    [1] = 	8.66,
    [0] = 	8.18,
};

static const char HEADER_TYPE[4] = { 'M', 'T', 'h', 'd' };
static const uint32_t HEADER_LENGTH = 6;
static const char TRACK_TYPE[4] = { 'M', 'T', 'r', 'k' };

#define STATUS_BYTE_END_EXCLUSIVE (0b11110111)

static inline uint32_t endian_swap_32(uint32_t v) {
    return (v << 24) | ((v & 0xFF00) << 8) | ((v & 0xFF0000) >> 8) | (v >> 24);
}
static inline uint16_t endian_swap_16(uint16_t v) {
    return (v << 8) | (v >> 8);
}

/**
 * @brief Returns the next byte from a given track and advances the track pointer
 *
 * @param parser
 * @param track_num
 */
static inline uint8_t next_byte(struct midi_parser *parser, int track_num) {
    return *(parser->tracks[track_num].pointer++);
}

static inline char *status_string(enum midi_status_code status_code) {
    switch (status_code) {
        case MIDI_STATUS_NOTE_OFF:
            return "NOTE_OFF";
        case MIDI_STATUS_NOTE_ON:
            return "NOTE_ON";
        case MIDI_STATUS_POLYPHONIC_KEY_PRESSURE:
            return "POLYPHONIC_KEY_PRESSURE";
        case MIDI_STATUS_CONTROL_CHANGE:
            return "CONTROL_CHANGE";
        case MIDI_STATUS_PROGRAM_CHANGE:
            return "PROGRAM_CHANGE";
        case MIDI_STATUS_CHANNEL_PRESSURE:
            return "CHANNEL_PRESSURE";
        case MIDI_STATUS_PITCH_WHEEL_CHANGE:
            return "PITCH_WHEEL_CHANGE";
        case MIDI_STATUS_SYSTEM:
            return "SYSTEM";
    }
}

float midi_parser_note_frequency(uint8_t note_number) {
    return note_frequencies[note_number];
}

static void calculate_us_per_tick(struct midi_parser *parser) {
    // depending on division format, delta time ticks means a different thing
    if (parser->division.format == MIDI_FORMAT_TICKS_PER_QUARTER_NOTE) {
        // time deltas are always in ticks, if we're operating on ticks per quarter note then we evaluate based on tempo
        parser->us_per_tick = parser->tempo_us_per_quarter_note / parser->division.ticks_per_quarter_note;
    } else if (parser->division.format == MIDI_FORMAT_SMPTE) {
        parser->us_per_tick = 1000000 / (SMPTE_frames_per_second_map[parser->division.midi_SMPTE_format] * parser->division.ticks_per_frame);
    }
}

/**
 * @brief Processes meta event on given track, moves track pointer and processes data
 * NOTE: this function must be called AFTER a track encounters a 0xFF byte (denoting a meta event)
 * This means the next data must not be 0xFF
 *
 * @param parser
 * @param track_num
 */
static void process_meta_event(struct midi_parser *parser, int track_num, struct midi_event *event) {
    if (parser->tracks[track_num].ended)
        return;

    uint8_t meta_code = next_byte(parser, track_num);
    uint8_t length = next_byte(parser, track_num);
    uint8_t *data = parser->tracks[track_num].pointer;
    parser->tracks[track_num].pointer += length; // advance past data

    event->meta.length = length;
    event->meta.data = data;
    event->meta.code = meta_code;

    switch (meta_code) {
        case MIDI_META_EVENT_TEXT:
            TRACK_PRINT(track_num, "\"%.*s\"\n", length, (char *) data);
            break;
        case MIDI_META_EVENT_COPYRIGHT:
            TRACK_PRINT(track_num, "Copyright: \"%.*s\"\n", length, (char *) data);
            break;
        case MIDI_META_EVENT_SEQUENCE_TRACK_NAME:
            TRACK_PRINT(track_num, "Sequence/Track Name: \"%.*s\"\n", length, (char *) data);
            break;
        case MIDI_META_EVENT_INSTRUMENT_NAME:
            TRACK_PRINT(track_num, "Instrument Name: \"%.*s\"\n", length, (char *) data);
            break;
        case MIDI_META_EVENT_LYRIC:
            TRACK_PRINT(track_num, "Lyric: \"%.*s\"\n", length, (char *) data);
            break;
        case MIDI_META_EVENT_MARKER:
            TRACK_PRINT(track_num, "Marker: \"%.*s\"\n", length, (char *) data);
            break;
        case MIDI_META_EVENT_CUE_POINT:
            TRACK_PRINT(track_num, "Cue Point: \"%.*s\"\n", length, (char *) data);
            break;
        case MIDI_META_EVENT_CHANNEL_PREFIX:
            TRACK_PRINT(track_num, "Channel prefix: %d\n", *data);
            parser->tracks[track_num].channel_prefix = *data;
            break;
        case MIDI_META_EVENT_END_OF_TRACK:
            TRACK_PRINT(track_num, "Ended!\n");
            parser->tracks[track_num].ended = true;

            break;
        case MIDI_META_EVENT_SET_TEMPO:
            parser->tempo_us_per_quarter_note = data[0] << 16 | data[1] << 8 | data[2];

            TRACK_PRINT(track_num, "Set tempo: %d us/qt\n", parser->tempo_us_per_quarter_note);

            calculate_us_per_tick(parser);

            break;
        case MIDI_META_EVENT_TIME_SIGNATURE:
            // i do not give a fuck about no dang stink time signature
            TRACK_PRINT(track_num, "Time signature set: %d/%d\n", data[0], 1 << data[1]);
            break;
        case MIDI_META_EVENT_KEY_SIGNATURE:
        case MIDI_META_EVENT_SEQUENCER_SPECIFIC:
        case MIDI_META_EVENT_SEQ_NUM:
        default:
            // don't do anything
            break;
    }
}

static void process_event(struct midi_parser *parser, int track_num, struct midi_event *event) {
    if (parser->tracks[track_num].ended)
        return;

    union status_byte *status;

    // if next status byte isn't valid, use previous status
    if (!(*parser->tracks[track_num].pointer & (1 << 7))) {
        // use previous status byte
        status = &parser->tracks[track_num].previous_status;
    } else {
        // valid status byte, grab it
        status = (union status_byte *) parser->tracks[track_num].pointer;
        parser->tracks[track_num].previous_status = *status;
        parser->tracks[track_num].pointer++;
    }

    event->status = *status;

    if (status->status_code == MIDI_STATUS_SYSTEM) {
        // deal with system codes
        switch (status->midi_system_code) {
            case MIDI_SYSTEM_EXCLUSIVE:
                // parse until the end exclusive byte is found
                while (next_byte(parser, track_num) != STATUS_BYTE_END_EXCLUSIVE);
                break;
            case MIDI_SYSTEM_SONG_POSITION_POINTER:
                // skip past song position pointer
                // maybe process this later
                (void) next_byte(parser, track_num);
                (void) next_byte(parser, track_num);
                break;
            case MIDI_SYSTEM_SONG_SELECT:
                (void) next_byte(parser, track_num);
                break;
            case MIDI_SYSTEM_TUNE_REQUEST:
            case MIDI_SYSTEM_TIMING_CLOCK:
            case MIDI_SYSTEM_START:
            case MIDI_SYSTEM_CONTINUE:
            case MIDI_SYSTEM_STOP:
            case MIDI_SYSTEM_ACTIVE_SENSING:
                break;
            case MIDI_SYSTEM_META_ESCAPE:
                process_meta_event(parser, track_num, event);
                break;
        }
    } else {
        // some status commands only use one byte
        if (event->status.status_code == MIDI_STATUS_PROGRAM_CHANGE || event->status.status_code == MIDI_STATUS_CHANNEL_PRESSURE)
            event->raw = next_byte(parser, track_num);
        else
            event->raw = next_byte(parser, track_num) | next_byte(parser, track_num) << 8;

        // change note on events with zero velocity to a note off event
        if (event->status.status_code == MIDI_STATUS_NOTE_ON && event->velocity == 0)
            event->status.status_code = MIDI_STATUS_NOTE_OFF;

        TRACK_PRINT(track_num, "event: status: %s, channel: %d\n", status_string(event->status.status_code), event->status.channel);
    }
}

bool midi_parser_next_event(struct midi_parser *parser, struct midi_event *event) {
    // After initialization, the track pointers are at the beginning of the track data
    for (int i = 0; i < parser->num_tracks; i++) {
        if (parser->tracks[i].ended)
            continue;

        if (parser->tracks[i].state == TRACK_STATE_EVENT_PENDING) {
            process_event(parser, i, event);
            parser->tracks[i].state = TRACK_STATE_READ_DELTA;
            return true;
        }
    }

    return false;
}

uint64_t decode_variable_length_quantity(struct midi_parser *parser, int track_num) {
    uint64_t result = 0;
    uint8_t byte;

    // loop through next bytes until bit 7 is reset
    do {
        byte = next_byte(parser, track_num);
        result <<= 7;
        result |= (byte & 0x7F);
    } while (byte & (1 << 7));

    return result;
}

bool midi_parser_advance(struct midi_parser *parser, uint32_t delta_time_us) {
    uint64_t raw_delta_ticks;
    bool events_waiting = false;

    // After initialization, the track pointers are at the beginning of the track data
    for (int i = 0; i < parser->num_tracks; i++) {
        if (parser->tracks[i].ended)
            continue;

        if (parser->tracks[i].state == TRACK_STATE_READ_DELTA) {
            raw_delta_ticks = decode_variable_length_quantity(parser, i);

            parser->tracks[i].timer = raw_delta_ticks * parser->us_per_tick;
            parser->tracks[i].state = TRACK_STATE_WAIT_FOR_TIMER;
        }

        parser->tracks[i].timer -= delta_time_us;

        // if track timer has reached zero, event needs to be parsed
        if (parser->tracks[i].timer <= 0) {
            parser->tracks[i].state = TRACK_STATE_EVENT_PENDING;
            events_waiting = true;
        }
    }

    return events_waiting;
}

void midi_parser_restart(struct midi_parser *parser) {
    for (int i = 0; i < parser->num_tracks; i++) {
        parser->tracks[i].timer = 0;
        parser->tracks[i].pointer = (uint8_t *) (parser->track_headers[i] + 1);
        parser->tracks[i].ended = false;
    }
}

bool midi_parser_ended(struct midi_parser *parser) {
    for (int i = 0; i < parser->num_tracks; i++) {
        if (!parser->tracks[i].ended)
            return false;
    }

    return true;
}

bool midi_parser_init(struct midi_parser *parser, void *buffer) {
    // midi file should start with header chunk
    parser->header = (struct midi_chunk_header *) buffer;
    parser->tempo_us_per_quarter_note = BPM_TO_US_PER_QUARTER_NOTE(MIDI_DEFAULT_TEMPO_BPM);

    // verify type, should match HEADER_TYPE and HEADER_LENGTH
    if (strncmp(parser->header->type, HEADER_TYPE, 4) != 0 || endian_swap_32(parser->header->length_be) != HEADER_LENGTH)
        return false;

    // header has been verified, read header data (uint16 data immediately after header data)
    uint16_t *header_data = (uint16_t *) (parser->header + 1);

    uint16_t format = endian_swap_16(header_data[0]); // not sure if I care about the format
    parser->num_tracks = endian_swap_16(header_data[1]);
    parser->division.raw = endian_swap_16(header_data[2]);

    // now that division is known, calculate microseconds per tick
    calculate_us_per_tick(parser);

    if (parser->num_tracks > MIDI_MAX_TRACKS)
        parser->num_tracks = MIDI_MAX_TRACKS;

    uint8_t *track_ptr = ((uint8_t *) header_data) + HEADER_LENGTH;

    // first track is right after the header, so track_ptr already points to it
    for (int i = 0; i < parser->num_tracks; i++) {
        parser->track_headers[i] = (struct midi_chunk_header *) track_ptr;

        // verify track data
        if (strncmp(parser->track_headers[i]->type, TRACK_TYPE, 4) != 0)
            return false;

        track_ptr += sizeof(struct midi_chunk_header);
        parser->tracks[i].timer = 0;
        parser->tracks[i].pointer = track_ptr;
        parser->tracks[i].ended = false;
        parser->tracks[i].state = TRACK_STATE_READ_DELTA;
        track_ptr += endian_swap_32(parser->track_headers[i]->length_be);

        TRACK_PRINT(i, "Length: %d\n", endian_swap_32(parser->track_headers[i]->length_be));
    }

    return true;
}