#include "huc.h"

#define BG_WIDTH 512
#define BG_HEIGHT 256

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 224

#define PALETTE_BG 0
#define PALETTE_SPR 16

#define COLOR_BLUE 0x7
#define COLOR_RED (0x7 << 3)
#define COLOR_GREEN (0x7 << 6)
#define COLOR_BLACK 0x0
#define COLOR_WHITE (COLOR_GREEN | COLOR_RED | COLOR_BLUE)
#define COLOR_YELLOW (COLOR_GREEN | COLOR_RED)
#define COLOR_MAGENTA (COLOR_RED | COLOR_BLUE)
#define COLOR_GRAY ((0x3 << 6) | (0x3 << 3) | (0x3))

#define u8 char
#define u16 int

const u16 Palette[] = {
  COLOR_GRAY, /* BackGround(BG), Transparent(SPR)*/
  COLOR_WHITE, 
  COLOR_BLACK, 
  COLOR_BLACK, 
  COLOR_BLACK, 
  COLOR_BLACK, 
  COLOR_BLACK,
  COLOR_BLACK,
  COLOR_BLACK, 
  COLOR_BLACK,
  COLOR_BLACK, 
  COLOR_BLACK, 
  COLOR_BLACK, 
  COLOR_BLACK, 
  COLOR_BLACK, 
  COLOR_BLACK
};

main()
{
  u8 KeyState;
  u16 x, y;
  x = 0; y = 1;

  load_palette(PALETTE_BG + 0, Palette, sizeof(Palette) / sizeof(u16));
  
  while(1){
    vsync();

    KeyState = joy(0);
    put_number(KeyState, 3, 0, 0);

    /* Up, Right, Down, Left : Arrow key */
    (KeyState & JOY_UP) ? put_char('+', 1 + x, 0 + y) : put_char('-', 1 + x, 0 + y);
    (KeyState & JOY_RGHT) ? put_char('+', 2 + x, 1 + y) : put_char('-', 2 + x, 1 + y);
    (KeyState & JOY_DOWN) ? put_char('+', 1 + x, 2 + y) : put_char('-', 1 + x, 2 + y);
    (KeyState & JOY_LEFT) ? put_char('+', 0 + x, 1 + y) : put_char('-', 0 + x, 1 + y);

    /* Select, Run : Tab, Enter */
    (KeyState & JOY_SLCT) ? put_char('+', 4 + x, 2 + y) : put_char('-', 4 + x, 2 + y);
    (KeyState & JOY_STRT) ? put_char('+', 5 + x, 2 + y) : put_char('-', 5 + x, 2 + y);

    /* II, I : Space, N */
    (KeyState & JOY_B) ? put_char('+', 7 + x, 2 + y) : put_char('-', 7 + x, 2 + y);;
    (KeyState & JOY_A) ? put_char('+', 8 + x, 2 + y) : put_char('-', 8 + x, 2 + y);
  }
}