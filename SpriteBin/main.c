#include "huc.h"
#include "..\define.h"

/* Convert data */
/*
#incbin(Palette, "res/bomber_PAL.bin")
#incbin(Pattern, "res/bomber_PAT.bin")
#include "res\bomber_PAT.pal.txt"
#define GET_SPR_PAL bomber_PAT_PAL
#define SPR_PAL_COUNT 1
#define SPR_PAT_COUNT 2
*/
#incbin(Palette, "res/move_obj1_PAL.bin")
#incbin(Pattern, "res/move_obj1_PAT.bin")
#include "res\move_obj1_PAT.pal.txt"
#define GET_SPR_PAL move_obj1_PAT_PAL
#define SPR_PAL_COUNT 12
#define SPR_PAT_COUNT 24
/*
#incbin(Palette, "res/goblin_PAL.bin")
#incbin(Pattern, "res/goblin_PAT.bin")
#include "res\goblin_PAT.pal.txt"
#define GET_SPR_PAL goblin_PAT_PAL
#define SPR_PAL_COUNT 1
#define SPR_PAT_COUNT 12
*/
/* Convert data */

PutSpritesRandom()
{
  u8 i;
  for(i = 0;i < SPR_MAX;++i){
      spr_set(i);
      spr_x((rand() % SCREEN_WIDTH) - 8);
      spr_y((rand() % SCREEN_HEIGHT) - 8);
  }
}

#define SPR_VRAM 0x6000

main()
{
  u8 KeyState;
  u8 i;
  u8 SpId;

  load_vram(SPR_VRAM, Pattern, SPR_WORDSIZE_16x16 * SPR_PAT_COUNT);
  set_sprpal(0, Palette, SPR_PAL_COUNT);

  init_satb();

  for(i = 0;i < SPR_MAX;++i) {
    SpId = i % SPR_PAT_COUNT;
    spr_set(i);
    spr_ctrl(SIZE_MAS | FLIP_MAS, SZ_16x16 | NO_FLIP);
    spr_pattern(SPR_VRAM + (SPR_SIZE_16x16 / 16) * SpId);
    spr_pal(GET_SPR_PAL[SpId]);
    spr_pri(1);
  }

  PutSpritesRandom(); 
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
      PutSpritesRandom();
    }
   
    satb_update();
  }
}