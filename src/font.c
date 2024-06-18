#include "font.h"
#include <stdint.h>
#include <stddef.h>

struct character {
    uint8_t bitmap[9];
    uint8_t width;
    uint8_t height;
    int8_t offset;
};

static struct character char_0x20_def = {
    .bitmap = {

    },
    .width = 0,
    .height = 0,
    .offset = -8
};
static struct character char_0x21_def = {
    .bitmap = {
        0b110,
        0b010,
        0b010,
        0b010,
        0b011,
        0b000,
        0b011,
        0b011,
    },
    .width = 3,
    .height = 8,
    .offset = 0
};
static struct character char_0x22_def = {
    .bitmap = {
        0b11011,
        0b11011,
        0b01001,
        0b10010,
    },
    .width = 5,
    .height = 4,
    .offset = -4
};
static struct character char_0x23_def = {
    .bitmap = {
        0b110110,
        0b010010,
        0b111111,
        0b010010,
        0b010010,
        0b111111,
        0b010010,
        0b011011,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x24_def = {
    .bitmap = {
        0b00100,
        0b01110,
        0b10101,
        0b10100,
        0b01110,
        0b00101,
        0b10101,
        0b01110,
        0b00100,
    },
    .width = 5,
    .height = 9,
    .offset = 1
};
static struct character char_0x25_def = {
    .bitmap = {
        0b0100011,
        0b1010010,
        0b0100100,
        0b0001000,
        0b0001000,
        0b0010010,
        0b0100101,
        0b1100010,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x26_def = {
    .bitmap = {
        0b0110000,
        0b1001000,
        0b1010000,
        0b0101000,
        0b1001010,
        0b1000100,
        0b1001101,
        0b0110010,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x27_def = {
    .bitmap = {
        0b11,
        0b11,
        0b01,
        0b10,
    },
    .width = 2,
    .height = 4,
    .offset = -4
};
static struct character char_0x28_def = {
    .bitmap = {
        0b0011,
        0b0100,
        0b0100,
        0b1000,
        0b1000,
        0b0100,
        0b0100,
        0b0011,
    },
    .width = 4,
    .height = 8,
    .offset = 0
};
static struct character char_0x29_def = {
    .bitmap = {
        0b1100,
        0b0010,
        0b0010,
        0b0001,
        0b0001,
        0b0010,
        0b0010,
        0b1100,
    },
    .width = 4,
    .height = 8,
    .offset = 0
};
static struct character char_0x2a_def = {
    .bitmap = {
        0b00100,
        0b00100,
        0b11111,
        0b00100,
        0b01010,
    },
    .width = 5,
    .height = 5,
    .offset = -3
};
static struct character char_0x2b_def = {
    .bitmap = {
        0b00100,
        0b00100,
        0b11111,
        0b00100,
        0b00100,
    },
    .width = 5,
    .height = 5,
    .offset = -1
};
static struct character char_0x2c_def = {
    .bitmap = {
        0b11,
        0b11,
        0b01,
        0b10,
    },
    .width = 2,
    .height = 4,
    .offset = 2
};
static struct character char_0x2d_def = {
    .bitmap = {
        0b11111,
    },
    .width = 5,
    .height = 1,
    .offset = -3
};
static struct character char_0x2e_def = {
    .bitmap = {
        0b11,
        0b11,
    },
    .width = 2,
    .height = 2,
    .offset = 0
};
static struct character char_0x2f_def = {
    .bitmap = {
        0b0000011,
        0b0000010,
        0b0000100,
        0b0001000,
        0b0001000,
        0b0010000,
        0b0100000,
        0b1100000,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x30_def = {
    .bitmap = {
        0b001100,
        0b010010,
        0b100001,
        0b111001,
        0b100111,
        0b100001,
        0b010010,
        0b001100,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x31_def = {
    .bitmap = {
        0b00100,
        0b01100,
        0b10100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b11111,
    },
    .width = 5,
    .height = 8,
    .offset = 0
};
static struct character char_0x32_def = {
    .bitmap = {
        0b01110,
        0b10001,
        0b00001,
        0b00010,
        0b00100,
        0b01000,
        0b10001,
        0b11111,
    },
    .width = 5,
    .height = 8,
    .offset = 0
};
static struct character char_0x33_def = {
    .bitmap = {
        0b01110,
        0b10001,
        0b00001,
        0b00110,
        0b00001,
        0b00001,
        0b10001,
        0b01110,
    },
    .width = 5,
    .height = 8,
    .offset = 0
};
static struct character char_0x34_def = {
    .bitmap = {
        0b000110,
        0b001010,
        0b010010,
        0b100010,
        0b011111,
        0b000010,
        0b000010,
        0b000010,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x35_def = {
    .bitmap = {
        0b11111,
        0b10000,
        0b10000,
        0b01110,
        0b00001,
        0b00001,
        0b10001,
        0b01110,
    },
    .width = 5,
    .height = 8,
    .offset = 0
};
static struct character char_0x36_def = {
    .bitmap = {
        0b01110,
        0b10001,
        0b10000,
        0b10110,
        0b11001,
        0b10001,
        0b10001,
        0b01110,
    },
    .width = 5,
    .height = 8,
    .offset = 0
};
static struct character char_0x37_def = {
    .bitmap = {
        0b11111,
        0b00001,
        0b00010,
        0b00010,
        0b00100,
        0b01000,
        0b01000,
        0b10000,
    },
    .width = 5,
    .height = 8,
    .offset = 0
};
static struct character char_0x38_def = {
    .bitmap = {
        0b01110,
        0b10001,
        0b10001,
        0b01110,
        0b10001,
        0b10001,
        0b10001,
        0b01110,
    },
    .width = 5,
    .height = 8,
    .offset = 0
};
static struct character char_0x39_def = {
    .bitmap = {
        0b011110,
        0b100010,
        0b100010,
        0b100110,
        0b011010,
        0b000010,
        0b000010,
        0b000111,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x3a_def = {
    .bitmap = {
        0b11,
        0b11,
        0b00,
        0b00,
        0b11,
        0b11,
    },
    .width = 2,
    .height = 6,
    .offset = -1
};
static struct character char_0x3b_def = {
    .bitmap = {
        0b11,
        0b11,
        0b00,
        0b00,
        0b11,
        0b11,
        0b01,
        0b10,
    },
    .width = 2,
    .height = 8,
    .offset = 1
};
static struct character char_0x3c_def = {
    .bitmap = {
        0b000011,
        0b001100,
        0b010000,
        0b100000,
        0b010000,
        0b001100,
        0b000011,
    },
    .width = 6,
    .height = 7,
    .offset = 0
};
static struct character char_0x3d_def = {
    .bitmap = {
        0b11111,
        0b00000,
        0b11111,
    },
    .width = 5,
    .height = 3,
    .offset = -2
};
static struct character char_0x3e_def = {
    .bitmap = {
        0b110000,
        0b001100,
        0b000010,
        0b000001,
        0b000010,
        0b001100,
        0b110000,
    },
    .width = 6,
    .height = 7,
    .offset = 0
};
static struct character char_0x3f_def = {
    .bitmap = {
        0b01110,
        0b10001,
        0b10001,
        0b00001,
        0b00010,
        0b00100,
        0b00000,
        0b00100,
    },
    .width = 5,
    .height = 8,
    .offset = 0
};
static struct character char_0x40_def = {
    .bitmap = {
        0b00111100,
        0b01000010,
        0b10011001,
        0b10100101,
        0b10100101,
        0b10011010,
        0b01000000,
        0b00111100,
    },
    .width = 8,
    .height = 8,
    .offset = 0
};
static struct character char_0x41_def = {
    .bitmap = {
        0b0001000,
        0b0001000,
        0b0010100,
        0b0011100,
        0b0010100,
        0b0100010,
        0b0100010,
        0b1110111,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x42_def = {
    .bitmap = {
        0b111110,
        0b010001,
        0b010001,
        0b011110,
        0b010001,
        0b010001,
        0b010001,
        0b111110,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x43_def = {
    .bitmap = {
        0b00111,
        0b01001,
        0b10000,
        0b10000,
        0b10000,
        0b10000,
        0b01001,
        0b00111,
    },
    .width = 5,
    .height = 8,
    .offset = 0
};
static struct character char_0x44_def = {
    .bitmap = {
        0b111100,
        0b010010,
        0b010001,
        0b010001,
        0b010001,
        0b010001,
        0b010010,
        0b111100,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x45_def = {
    .bitmap = {
        0b111111,
        0b010001,
        0b010100,
        0b011100,
        0b010100,
        0b010000,
        0b010001,
        0b111111,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x46_def = {
    .bitmap = {
        0b111111,
        0b010001,
        0b010100,
        0b011100,
        0b010100,
        0b010000,
        0b010000,
        0b111000,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x47_def = {
    .bitmap = {
        0b001111,
        0b010001,
        0b100000,
        0b100000,
        0b100111,
        0b100001,
        0b010001,
        0b001111,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x48_def = {
    .bitmap = {
        0b1110111,
        0b0100010,
        0b0100010,
        0b0111110,
        0b0100010,
        0b0100010,
        0b0100010,
        0b1110111,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x49_def = {
    .bitmap = {
        0b11111,
        0b10101,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b10101,
        0b11111,
    },
    .width = 5,
    .height = 8,
    .offset = 0
};
static struct character char_0x4a_def = {
    .bitmap = {
        0b111111,
        0b100101,
        0b000100,
        0b000100,
        0b000100,
        0b100100,
        0b100100,
        0b011000,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x4b_def = {
    .bitmap = {
        0b1110011,
        0b0100100,
        0b0101000,
        0b0110000,
        0b0101000,
        0b0100100,
        0b0100010,
        0b1110011,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x4c_def = {
    .bitmap = {
        0b111000,
        0b010000,
        0b010000,
        0b010000,
        0b010000,
        0b010000,
        0b010001,
        0b111111,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x4d_def = {
    .bitmap = {
        0b1100011,
        0b0110110,
        0b0101010,
        0b0101010,
        0b0100010,
        0b0100010,
        0b0100010,
        0b1110111,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x4e_def = {
    .bitmap = {
        0b11000111,
        0b01100010,
        0b01010010,
        0b01010010,
        0b01001010,
        0b01001010,
        0b01000110,
        0b11100011,
    },
    .width = 8,
    .height = 8,
    .offset = 0
};
static struct character char_0x4f_def = {
    .bitmap = {
        0b0011100,
        0b0100010,
        0b1000001,
        0b1000001,
        0b1000001,
        0b1000001,
        0b0100010,
        0b0011100,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x50_def = {
    .bitmap = {
        0b111110,
        0b010001,
        0b010001,
        0b010001,
        0b011110,
        0b010000,
        0b010000,
        0b111000,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x51_def = {
    .bitmap = {
        0b0011100,
        0b0100010,
        0b1000001,
        0b1000001,
        0b1000001,
        0b1000101,
        0b0100010,
        0b0011101,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x52_def = {
    .bitmap = {
        0b111110,
        0b010001,
        0b010001,
        0b011010,
        0b010100,
        0b010010,
        0b010001,
        0b110001,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x53_def = {
    .bitmap = {
        0b01110,
        0b10001,
        0b10000,
        0b01110,
        0b00001,
        0b00001,
        0b10001,
        0b01110,
    },
    .width = 5,
    .height = 8,
    .offset = 0
};
static struct character char_0x54_def = {
    .bitmap = {
        0b11111,
        0b10101,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b00100,
        0b01110,
    },
    .width = 5,
    .height = 8,
    .offset = 0
};
static struct character char_0x55_def = {
    .bitmap = {
        0b1110111,
        0b0100010,
        0b0100010,
        0b0100010,
        0b0100010,
        0b0100010,
        0b0100010,
        0b0011100,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x56_def = {
    .bitmap = {
        0b1110111,
        0b0100010,
        0b0100010,
        0b0100010,
        0b0100010,
        0b0010100,
        0b0010100,
        0b0001000,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x57_def = {
    .bitmap = {
        0b1100011,
        0b1000001,
        0b1000001,
        0b1000001,
        0b0101010,
        0b0101010,
        0b0010100,
        0b0010100,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x58_def = {
    .bitmap = {
        0b1110111,
        0b0100010,
        0b0010100,
        0b0010100,
        0b0001000,
        0b0010100,
        0b0100010,
        0b1110111,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x59_def = {
    .bitmap = {
        0b1110111,
        0b0100010,
        0b0100010,
        0b0010100,
        0b0001000,
        0b0001000,
        0b0001000,
        0b0011100,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x5a_def = {
    .bitmap = {
        0b111111,
        0b100001,
        0b000010,
        0b000100,
        0b001000,
        0b010000,
        0b100001,
        0b111111,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x5b_def = {
    .bitmap = {
        0b111,
        0b100,
        0b100,
        0b100,
        0b100,
        0b100,
        0b100,
        0b111,
    },
    .width = 3,
    .height = 8,
    .offset = 0
};
static struct character char_0x5c_def = {
    .bitmap = {
        0b1100000,
        0b0100000,
        0b0010000,
        0b0001000,
        0b0001000,
        0b0000100,
        0b0000010,
        0b0000011,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x5d_def = {
    .bitmap = {
        0b111,
        0b001,
        0b001,
        0b001,
        0b001,
        0b001,
        0b001,
        0b111,
    },
    .width = 3,
    .height = 8,
    .offset = 0
};
static struct character char_0x5e_def = {
    .bitmap = {
        0b00100,
        0b01010,
        0b01010,
        0b10001,
    },
    .width = 5,
    .height = 4,
    .offset = -4
};
static struct character char_0x5f_def = {
    .bitmap = {
        0b11111,
    },
    .width = 5,
    .height = 1,
    .offset = 1
};
static struct character char_0x60_def = {
    .bitmap = {
        0b110,
        0b111,
        0b001,
    },
    .width = 3,
    .height = 3,
    .offset = -5
};
static struct character char_0x61_def = {
    .bitmap = {
        0b011111,
        0b100010,
        0b100010,
        0b100110,
        0b011011,
    },
    .width = 6,
    .height = 5,
    .offset = 0
};
static struct character char_0x62_def = {
    .bitmap = {
        0b110000,
        0b010000,
        0b010000,
        0b010000,
        0b011110,
        0b010001,
        0b011001,
        0b110110,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x63_def = {
    .bitmap = {
        0b01110,
        0b10001,
        0b10000,
        0b10001,
        0b01110,
    },
    .width = 5,
    .height = 5,
    .offset = 0
};
static struct character char_0x64_def = {
    .bitmap = {
        0b000011,
        0b000010,
        0b000010,
        0b011110,
        0b100010,
        0b100010,
        0b100110,
        0b011011,
    },
    .width = 6,
    .height = 8,
    .offset = 0
};
static struct character char_0x65_def = {
    .bitmap = {
        0b01110,
        0b10001,
        0b11111,
        0b10000,
        0b01110,
    },
    .width = 5,
    .height = 5,
    .offset = 0
};
static struct character char_0x66_def = {
    .bitmap = {
        0b00110,
        0b01001,
        0b01001,
        0b11100,
        0b01000,
        0b01000,
        0b01000,
        0b11100,
    },
    .width = 5,
    .height = 8,
    .offset = 0
};
static struct character char_0x67_def = {
    .bitmap = {
        0b011011,
        0b100110,
        0b100010,
        0b100010,
        0b011110,
        0b000010,
        0b100010,
        0b011100,
    },
    .width = 6,
    .height = 8,
    .offset = 3
};
static struct character char_0x68_def = {
    .bitmap = {
        0b1100000,
        0b0100000,
        0b0100000,
        0b0101100,
        0b0110010,
        0b0100010,
        0b0100010,
        0b1100111,
    },
    .width = 7,
    .height = 8,
    .offset = 0
};
static struct character char_0x69_def = {
    .bitmap = {
        0b010,
        0b000,
        0b110,
        0b010,
        0b010,
        0b010,
        0b111,
    },
    .width = 3,
    .height = 7,
    .offset = 0
};
static struct character char_0x6a_def = {
    .bitmap = {
        0b0001,
        0b0000,
        0b0011,
        0b0001,
        0b0001,
        0b0001,
        0b0001,
        0b1001,
        0b0110,
    },
    .width = 4,
    .height = 9,
    .offset = 2
};
static struct character char_0x6b_def = {
    .bitmap = {
        0b11000,
        0b01000,
        0b01001,
        0b01010,
        0b01100,
        0b01010,
        0b01001,
        0b11001,
    },
    .width = 5,
    .height = 8,
    .offset = 0
};
static struct character char_0x6c_def = {
    .bitmap = {
        0b110,
        0b010,
        0b010,
        0b010,
        0b010,
        0b010,
        0b010,
        0b111,
    },
    .width = 3,
    .height = 8,
    .offset = 0
};
static struct character char_0x6d_def = {
    .bitmap = {
        0b11010100,
        0b01101010,
        0b01001010,
        0b01001010,
        0b11011011,
    },
    .width = 8,
    .height = 5,
    .offset = 0
};
static struct character char_0x6e_def = {
    .bitmap = {
        0b1101100,
        0b0110010,
        0b0100010,
        0b0100010,
        0b1100111,
    },
    .width = 7,
    .height = 5,
    .offset = 0
};
static struct character char_0x6f_def = {
    .bitmap = {
        0b01110,
        0b10001,
        0b10001,
        0b10001,
        0b01110,
    },
    .width = 5,
    .height = 5,
    .offset = 0
};
static struct character char_0x70_def = {
    .bitmap = {
        0b110110,
        0b011001,
        0b010001,
        0b010001,
        0b011110,
        0b010000,
        0b010000,
        0b110000,
    },
    .width = 6,
    .height = 8,
    .offset = 3
};
static struct character char_0x71_def = {
    .bitmap = {
        0b0110110,
        0b1001100,
        0b1000100,
        0b1000100,
        0b0111100,
        0b0000100,
        0b0000101,
        0b0000010,
    },
    .width = 7,
    .height = 8,
    .offset = 3
};
static struct character char_0x72_def = {
    .bitmap = {
        0b110110,
        0b011001,
        0b010000,
        0b010000,
        0b110000,
    },
    .width = 6,
    .height = 5,
    .offset = 0
};
static struct character char_0x73_def = {
    .bitmap = {
        0b01111,
        0b10000,
        0b01110,
        0b00001,
        0b11110,
    },
    .width = 5,
    .height = 5,
    .offset = 0
};
static struct character char_0x74_def = {
    .bitmap = {
        0b010,
        0b010,
        0b111,
        0b010,
        0b010,
        0b010,
        0b011,
    },
    .width = 3,
    .height = 7,
    .offset = 0
};
static struct character char_0x75_def = {
    .bitmap = {
        0b1110011,
        0b0100010,
        0b0100010,
        0b0100110,
        0b0011011,
    },
    .width = 7,
    .height = 5,
    .offset = 0
};
static struct character char_0x76_def = {
    .bitmap = {
        0b1110111,
        0b0100010,
        0b0010100,
        0b0010100,
        0b0001000,
    },
    .width = 7,
    .height = 5,
    .offset = 0
};
static struct character char_0x77_def = {
    .bitmap = {
        0b1100011,
        0b1000001,
        0b0101010,
        0b0101010,
        0b0010100,
    },
    .width = 7,
    .height = 5,
    .offset = 0
};
static struct character char_0x78_def = {
    .bitmap = {
        0b1100011,
        0b0010100,
        0b0001000,
        0b0010100,
        0b1100011,
    },
    .width = 7,
    .height = 5,
    .offset = 0
};
static struct character char_0x79_def = {
    .bitmap = {
        0b1110011,
        0b0100010,
        0b0100010,
        0b0100110,
        0b0011010,
        0b0000010,
        0b0100010,
        0b0011100,
    },
    .width = 7,
    .height = 8,
    .offset = 3
};
static struct character char_0x7a_def = {
    .bitmap = {
        0b11111,
        0b10010,
        0b00100,
        0b01001,
        0b11111,
    },
    .width = 5,
    .height = 5,
    .offset = 0
};
static struct character char_0x7b_def = {
    .bitmap = {
        0b0011,
        0b0100,
        0b0100,
        0b1000,
        0b0100,
        0b0100,
        0b0100,
        0b0011,
    },
    .width = 4,
    .height = 8,
    .offset = 0
};
static struct character char_0x7c_def = {
    .bitmap = {
        0b1,
        0b1,
        0b1,
        0b1,
        0b1,
        0b1,
        0b1,
        0b1,
    },
    .width = 1,
    .height = 8,
    .offset = 0
};
static struct character char_0x7d_def = {
    .bitmap = {
        0b1100,
        0b0010,
        0b0010,
        0b0001,
        0b0010,
        0b0010,
        0b0010,
        0b1100,
    },
    .width = 4,
    .height = 8,
    .offset = 0
};
static struct character char_0x7e_def = {
    .bitmap = {
        0b011001,
        0b100110,
    },
    .width = 6,
    .height = 2,
    .offset = -4
};


static const struct character *font[] = {
    [0x20] = &char_0x20_def,
    [0x21] = &char_0x21_def,
    [0x22] = &char_0x22_def,
    [0x23] = &char_0x23_def,
    [0x24] = &char_0x24_def,
    [0x25] = &char_0x25_def,
    [0x26] = &char_0x26_def,
    [0x27] = &char_0x27_def,
    [0x28] = &char_0x28_def,
    [0x29] = &char_0x29_def,
    [0x2a] = &char_0x2a_def,
    [0x2b] = &char_0x2b_def,
    [0x2c] = &char_0x2c_def,
    [0x2d] = &char_0x2d_def,
    [0x2e] = &char_0x2e_def,
    [0x2f] = &char_0x2f_def,
    [0x30] = &char_0x30_def,
    [0x31] = &char_0x31_def,
    [0x32] = &char_0x32_def,
    [0x33] = &char_0x33_def,
    [0x34] = &char_0x34_def,
    [0x35] = &char_0x35_def,
    [0x36] = &char_0x36_def,
    [0x37] = &char_0x37_def,
    [0x38] = &char_0x38_def,
    [0x39] = &char_0x39_def,
    [0x3a] = &char_0x3a_def,
    [0x3b] = &char_0x3b_def,
    [0x3c] = &char_0x3c_def,
    [0x3d] = &char_0x3d_def,
    [0x3e] = &char_0x3e_def,
    [0x3f] = &char_0x3f_def,
    [0x40] = &char_0x40_def,
    [0x41] = &char_0x41_def,
    [0x42] = &char_0x42_def,
    [0x43] = &char_0x43_def,
    [0x44] = &char_0x44_def,
    [0x45] = &char_0x45_def,
    [0x46] = &char_0x46_def,
    [0x47] = &char_0x47_def,
    [0x48] = &char_0x48_def,
    [0x49] = &char_0x49_def,
    [0x4a] = &char_0x4a_def,
    [0x4b] = &char_0x4b_def,
    [0x4c] = &char_0x4c_def,
    [0x4d] = &char_0x4d_def,
    [0x4e] = &char_0x4e_def,
    [0x4f] = &char_0x4f_def,
    [0x50] = &char_0x50_def,
    [0x51] = &char_0x51_def,
    [0x52] = &char_0x52_def,
    [0x53] = &char_0x53_def,
    [0x54] = &char_0x54_def,
    [0x55] = &char_0x55_def,
    [0x56] = &char_0x56_def,
    [0x57] = &char_0x57_def,
    [0x58] = &char_0x58_def,
    [0x59] = &char_0x59_def,
    [0x5a] = &char_0x5a_def,
    [0x5b] = &char_0x5b_def,
    [0x5c] = &char_0x5c_def,
    [0x5d] = &char_0x5d_def,
    [0x5e] = &char_0x5e_def,
    [0x5f] = &char_0x5f_def,
    [0x60] = &char_0x60_def,
    [0x61] = &char_0x61_def,
    [0x62] = &char_0x62_def,
    [0x63] = &char_0x63_def,
    [0x64] = &char_0x64_def,
    [0x65] = &char_0x65_def,
    [0x66] = &char_0x66_def,
    [0x67] = &char_0x67_def,
    [0x68] = &char_0x68_def,
    [0x69] = &char_0x69_def,
    [0x6a] = &char_0x6a_def,
    [0x6b] = &char_0x6b_def,
    [0x6c] = &char_0x6c_def,
    [0x6d] = &char_0x6d_def,
    [0x6e] = &char_0x6e_def,
    [0x6f] = &char_0x6f_def,
    [0x70] = &char_0x70_def,
    [0x71] = &char_0x71_def,
    [0x72] = &char_0x72_def,
    [0x73] = &char_0x73_def,
    [0x74] = &char_0x74_def,
    [0x75] = &char_0x75_def,
    [0x76] = &char_0x76_def,
    [0x77] = &char_0x77_def,
    [0x78] = &char_0x78_def,
    [0x79] = &char_0x79_def,
    [0x7a] = &char_0x7a_def,
    [0x7b] = &char_0x7b_def,
    [0x7c] = &char_0x7c_def,
    [0x7d] = &char_0x7d_def,
    [0x7e] = &char_0x7e_def,
};


static void draw_char(int x, int y, const struct character *c, bool invert_color) {
    int draw_x;
    int draw_y;
    y += c->offset;
    struct color draw_color = { .a = 0xff };

    // draw pixels from bitmap
    for (int r = 0; r < c->height; r++) {
        // loop through bits starting from most significant
        for (int b = (1 << (c->width - 1)); b > 0; b >>= 1, x++) {
            if (b & c->bitmap[r]) {
                draw_color.r = draw_color.g = draw_color.b = invert_color ? 0x00 : 0xff;
                draw_x = x;
                draw_y = (y - c->height + r);

                pal_screen_draw_pixel(draw_x, draw_y, draw_color);
            }
        }

        // bring x back to the starting column
        x -= c->width;
    }
}

void draw_text(int x, int y, char *text, bool invert_color) {
    const struct character *c;
    const int letter_spacing = 1;
    const int line_height = 8 + letter_spacing;
    const int space_length = 3;
    const int start_x = x;

    while (*text != '\0') {
        if (*text == ' ') {
            x += space_length;
            text++; // advance character pointer
            continue;
        }

        c = font[(int)(*text)];

        // if (*text == '\n') {
        //     pos.y += line_height;
        //     x = start_x;
        // }

        if (c == NULL) {
            text++; // advance character pointer
            continue;
        }

        // if (x + c->width > PAL_SCREEN_WIDTH) {
        //     pos.y += line_height;
        //     x = start_x;
        // }

        draw_char(x, y, c, invert_color);

        x += c->width + letter_spacing;

        text++; // advance character pointer
    }
}
