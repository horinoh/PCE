#define BG_WIDTH 512
#define BG_HEIGHT 256

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 224
#define SCREEN_CELLS_W 32 /* SCREEN_WIDTH >> 3 */
#define SCREEN_CELLS_H 28 /* SCREEN_HEIGHT >> 3 */

/* Window */
#define WIN0 0
#define WIN1 1
#define WIN2 2
#define WIN3 3

#define SCR_SPR_ON (1 << 6)
#define SCR_BG_ON (1 << 7)

/* Palette */
#define PALETTE_BG00 0
#define PALETTE_BG01 1
#define PALETTE_BG02 2
#define PALETTE_BG03 3
#define PALETTE_BG04 4
#define PALETTE_BG05 5
#define PALETTE_BG06 6
#define PALETTE_BG07 7
#define PALETTE_BG08 8
#define PALETTE_BG09 9
#define PALETTE_BG10 10
#define PALETTE_BG11 11
#define PALETTE_BG12 12
#define PALETTE_BG13 13
#define PALETTE_BG14 14
#define PALETTE_BG15 15

#define PALETTE_SPR00 16
#define PALETTE_SPR01 17
#define PALETTE_SPR02 18
#define PALETTE_SPR03 19
#define PALETTE_SPR04 20
#define PALETTE_SPR05 21
#define PALETTE_SPR06 22
#define PALETTE_SPR07 23
#define PALETTE_SPR08 24
#define PALETTE_SPR09 25
#define PALETTE_SPR10 26
#define PALETTE_SPR11 27
#define PALETTE_SPR12 28
#define PALETTE_SPR13 29
#define PALETTE_SPR14 30
#define PALETTE_SPR15 31

/* Sprite 16x16, 16x32, 16x64, 32x16, 32x32, 32x64 */
#define SPR_MAX 64

#define SPR_PLANES 4

#define SPR_SIZE_16x16 (16 * 16 * SPR_PLANES)
#define SPR_SIZE_16x32 (16 * 32 * SPR_PLANES)
#define SPR_SIZE_16x64 (16 * 64 * SPR_PLANES)
#define SPR_SIZE_32x16 (32 * 16 * SPR_PLANES)
#define SPR_SIZE_32x32 (32 * 32 * SPR_PLANES)
#define SPR_SIZE_32x64 (32 * 64 * SPR_PLANES)

#define SPR_WORDSIZE_16x16 (SPR_SIZE_16x16 / sizeof(u16))
#define SPR_WORDSIZE_16x32 (SPR_SIZE_16x32 / sizeof(u16))
#define SPR_WORDSIZE_16x64 (SPR_SIZE_16x64 / sizeof(u16))
#define SPR_WORDSIZE_32x16 (SPR_SIZE_32x16 / sizeof(u16))
#define SPR_WORDSIZE_32x32 (SPR_SIZE_32x32 / sizeof(u16))
#define SPR_WORDSIZE_32x64 (SPR_SIZE_32x64 / sizeof(u16))

/* BG */
#define BG_TILE_MAS 0xfff
#define BG_PALETTE_MAS 0xf000
#define BG_PALETTE_SFT 12

/* Colors */
#define COLOR_TRANSPARENT 0x0
#define COLOR_BACKGROUND 0x0
#define COLOR_BLUE 0x7
#define COLOR_RED (0x7 << 3)
#define COLOR_GREEN (0x7 << 6)
#define COLOR_DARK_BLUE 0x3
#define COLOR_DARK_RED (0x3 << 3)
#define COLOR_DARK_GREEN (0x3 << 6)
#define COLOR_BLACK ((0x1 << 6) | (0x1 << 3) | 0x1)
#define COLOR_WHITE (COLOR_GREEN | COLOR_RED | COLOR_BLUE)
#define COLOR_YELLOW (COLOR_GREEN | COLOR_RED)
#define COLOR_MAGENTA (COLOR_RED | COLOR_BLUE)
#define COLOR_GRAY (COLOR_DARK_GREEN | COLOR_DARK_RED | COLOR_DARK_BLUE)
#define COLOR_LIGHTBLUE (COLOR_GREEN | COLOR_BLUE)

/* Joypad */
#define JOY_PAD0 0
#define JOY_PAD1 1
#define JOY_PAD2 2
#define JOY_PAD3 3
#define JOY_PAD4 4
#define JOY_PAD0_MAS (1 << JOY_PAD0)
#define JOY_PAD1_MAS (1 << JOY_PAD1)
#define JOY_PAD2_MAS (1 << JOY_PAD2)
#define JOY_PAD3_MAS (1 << JOY_PAD3)

/* typedef */
#define u8 char
#define u16 int
