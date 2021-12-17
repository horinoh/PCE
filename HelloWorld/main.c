#include "huc.h"

#include "..\define.h"

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

  /*load_palette(PALETTE_BG00, Palette, 1);*/
  /*set_bgpal(0, Palette, 1);*/  
  
  put_string("Hello World", 0, SCREEN_HEIGHT / 8 - 1);
  
  /* Character code [32, 255]*/
  for(i = 0; i < 256 - 32;i++){
    put_char(i + 32, i % 16, i / 16);
  }

  while(1){
    vsync();
  }
}