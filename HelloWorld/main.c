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
  u16 i;

  load_palette(PALETTE_BG + 0, Palette, sizeof(Palette) / sizeof(u16));

  put_string("Hello World", 0, SCREEN_HEIGHT / 8 - 1);
  
  /* Character code [32, 255]*/
  for(i = 0; i < 256 - 32;i++){
    put_char(i + 32, i % 16, i / 16);
  }

  while(1){
    vsync();
  }
}