#define BG_WIDTH 512
#define BG_HEIGHT 256

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 224

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

/* Sprite */
#define SPR_MAX 64

/* Colors */
#define COLOR_BLUE 0x7
#define COLOR_RED (0x7 << 3)
#define COLOR_GREEN (0x7 << 6)
#define COLOR_BLACK 0x0
#define COLOR_WHITE (COLOR_GREEN | COLOR_RED | COLOR_BLUE)
#define COLOR_YELLOW (COLOR_GREEN | COLOR_RED)
#define COLOR_MAGENTA (COLOR_RED | COLOR_BLUE)
#define COLOR_GRAY ((0x3 << 6) | (0x3 << 3) | (0x3))
#define COLOR_LIGHTBLUE (COLOR_GREEN | COLOR_BLUE)

/* Joypad */
#define JOY_PAD0 0
#define JOY_PAD1 1
#define JOY_PAD2 2
#define JOY_PAD3 3
#define JOY_PAD4 4

/* typedef */
#define u8 char
#define u16 int
