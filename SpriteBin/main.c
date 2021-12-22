#include "huc.h"
#include "..\define.h"

#incbin(Palette, "SPR_Palette.bin")
#incbin(Pattern, "SPR_Pattern.bin")

PutSprites()
{
  u8 i;
  for(i = 0;i < SPR_MAX;++i){
      spr_set(i);
      spr_x((rand() % SCREEN_WIDTH) - 8);
      spr_y((rand() % SCREEN_HEIGHT) - 8);
  }
}

#define SPR_VRAM 0x4000

main()
{
  u8 KeyState;
  u8 i;

  load_vram(SPR_VRAM, Pattern, SPR_WORDSIZE_16x16);
  set_sprpal(0, Palette, 1);

  init_satb();

  for(i = 0;i < SPR_MAX;++i) {
    spr_set(i);
    spr_ctrl(SIZE_MAS | FLIP_MAS, SZ_16x16 | NO_FLIP);
    spr_pattern(SPR_VRAM);
    spr_pal(0);
    spr_pri(1);
  }

  PutSprites(); 
  while(1){
    vsync();

    KeyState = joy(JOY_PAD0);
    for(i = 0;i < SPR_MAX;++i){
      spr_set(i);
      if(KeyState & JOY_LEFT) spr_x(spr_get_x() - 1);
      if(KeyState & JOY_RGHT) spr_x(spr_get_x() + 1);
      if(KeyState & JOY_UP) spr_y(spr_get_y() - 1);
      if(KeyState & JOY_DOWN) spr_y(spr_get_y() + 1);
    }

    if(joytrg(JOY_PAD0) & (JOY_A | JOY_B)){
      PutSprites();
    }
   
    satb_update();
  }
}