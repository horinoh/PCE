#include "huc.h"
#include "..\define.h"

const u16 Palette0[] = {
  COLOR_BLACK, /* BackGround(BG), Transparent(SPR)*/
  COLOR_WHITE, 
  COLOR_MAGENTA, 
  COLOR_LIGHTBLUE, 
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

#define B 0 /* Index of COLOR_BLACK */
#define W 1 /* Index of COLOR_WHITE */
#define M 2 /* Index of COLOR_MAGENTA */
#define L 3 /* Index of COLOR_LIGHTBLUE */
/* Normal index color picture data */
/*
const u16 Picture[] = {
  B, B, B, M, 
  W, B, W, W, 
  W, W, W, W, 
  B, B, B, B,

  B, B, B, B, 
  B, W, W, W, 
  W, W, W, W, 
  W, B, B, B,

  B, B, B, B, 
  W, W, W, M, 
  M, B, M, B, 
  M, B, B, B,

  B, B, B, B,
  W, W, W, M,
  M, B, M, B,
  M, B, B, B,

  B, B, B, B,
  B, W, W, W,
  W, W, W, W,
  W, B, B, B,

  B, B, B, B,
  B, L, W, W,
  W, W, W, W,
  B, B, B, B,

  B, B, B, B,
  B, L, L, L,
  L, L, L, B,
  B, B, B, B,

  B, B, B, B,
  L, L, W, W,
  W, L, B, M,
  M, B, B, B,

  B, B, M, M,
  L, L, W, W,
  W, W, W, M,
  M, B, B, B,

  B, B, M, M,
  L, L, L, W,
  W, W, W, B,
  B, B, B, B,

  B, B, B, B,
  L, L, L, L,
  L, L, B, B,
  M, B, B, B,

  B, B, B, B,
  L, L, W, L,
  L, L, W, M,
  M, B, B, B,

  B, B, B, W,
  W, W, W, L,
  L, W, W, M,
  M, B, B, B,

  B, B, M, M,
  W, W, W, B,
  L, L, W, M,
  M, B, B, B,

  B, B, B, M,
  M, B, B, B,
  B, B, B, B,
  B, B, B, B,

  B, B, B, B,
  M, M, B, B,
  B, B, B, B,
  B, B, B, B
};
*/

/*
Plane0 is masked part of color index with 1 -> color index & 1
Plane1 is masked part of color index with 2 -> color index & 2
Plane2 is masked part of color index with 4 -> color index & 4
Plane3 is masked part of color index with 8 -> color index & 8
*/
#define B0 0 /* Index of COLOR_BLACK & 1 */
#define B1 0 /* Index of COLOR_BLACK & 2 */
#define B2 0 /* Index of COLOR_BLACK & 4 */
#define B3 0 /* Index of COLOR_BLACK & 8 */

#define W0 1 /* Index of COLOR_WHITE & 1 */
#define W1 0 /* Index of COLOR_WHITE & 2 */
#define W2 0 /* Index of COLOR_WHITE & 4 */
#define W3 0 /* Index of COLOR_WHITE & 8 */

#define M0 0 /* Index of COLOR_MAGENTA & 1 */
#define M1 2 /* Index of COLOR_MAGENTA & 2 */
#define M2 0 /* Index of COLOR_MAGENTA & 4 */
#define M3 0 /* Index of COLOR_MAGENTA & 8 */

#define L0 1 /* Index of COLOR_LIGHTBLUE & 1 */
#define L1 2 /* Index of COLOR_LIGHTBLUE & 2 */
#define L2 0 /* Index of COLOR_LIGHTBLUE & 4 */
#define L3 0 /* Index of COLOR_LIGHTBLUE & 8 */
const u16 Pattern0[] = {
  /* Plane 0 */
  B0<<15|B0<<14|B0<<13|M0<<12|W0<<11|B0<<10|W0<<9|W0<<8|W0<<7|W0<<6|W0<<5|W0<<4|B0<<3|B0<<2|B0<<1|B0<<0,
  B0<<15|B0<<14|B0<<13|B0<<12|B0<<11|W0<<10|W0<<9|W0<<8|W0<<7|W0<<6|W0<<5|W0<<4|W0<<3|B0<<2|B0<<1|B0<<0,
  B0<<15|B0<<14|B0<<13|B0<<12|W0<<11|W0<<10|W0<<9|M0<<8|M0<<7|B0<<6|M0<<5|B0<<4|M0<<3|B0<<2|B0<<1|B0<<0,
  B0<<15|B0<<14|B0<<13|B0<<12|W0<<11|W0<<10|W0<<9|M0<<8|M0<<7|B0<<6|M0<<5|B0<<4|M0<<3|B0<<2|B0<<1|B0<<0,
  
  B0<<15|B0<<14|B0<<13|B0<<12|B0<<11|W0<<10|W0<<9|W0<<8|W0<<7|W0<<6|W0<<5|W0<<4|W0<<3|B0<<2|B0<<1|B0<<0,
  B0<<15|B0<<14|B0<<13|B0<<12|B0<<11|L0<<10|W0<<9|W0<<8|W0<<7|W0<<6|W0<<5|W0<<4|B0<<3|B0<<2|B0<<1|B0<<0,
  B0<<15|B0<<14|B0<<13|B0<<12|B0<<11|L0<<10|L0<<9|L0<<8|L0<<7|L0<<6|L0<<5|B0<<4|B0<<3|B0<<2|B0<<1|B0<<0,
  B0<<15|B0<<14|B0<<13|B0<<12|L0<<11|L0<<10|W0<<9|W0<<8|W0<<7|L0<<6|B0<<5|M0<<4|M0<<3|B0<<2|B0<<1|B0<<0,
  
  B0<<15|B0<<14|M0<<13|M0<<12|L0<<11|L0<<10|W0<<9|W0<<8|W0<<7|W0<<6|W0<<5|M0<<4|M0<<3|B0<<2|B0<<1|B0<<0,
  B0<<15|B0<<14|M0<<13|M0<<12|L0<<11|L0<<10|L0<<9|W0<<8|W0<<7|W0<<6|W0<<5|B0<<4|B0<<3|B0<<2|B0<<1|B0<<0,
  B0<<15|B0<<14|B0<<13|B0<<12|L0<<11|L0<<10|L0<<9|L0<<8|L0<<7|L0<<6|B0<<5|B0<<4|M0<<3|B0<<2|B0<<1|B0<<0,
  B0<<15|B0<<14|B0<<13|B0<<12|L0<<11|L0<<10|W0<<9|L0<<8|L0<<7|L0<<6|W0<<5|M0<<4|M0<<3|B0<<2|B0<<1|B0<<0,

  B0<<15|B0<<14|B0<<13|W0<<12|W0<<11|W0<<10|W0<<9|L0<<8|L0<<7|W0<<6|W0<<5|M0<<4|M0<<3|B0<<2|B0<<1|B0<<0,
  B0<<15|B0<<14|M0<<13|M0<<12|W0<<11|W0<<10|W0<<9|B0<<8|L0<<7|L0<<6|W0<<5|M0<<4|M0<<3|B0<<2|B0<<1|B0<<0,
  B0<<15|B0<<14|B0<<13|M0<<12|M0<<11|B0<<10|B0<<9|B0<<8|B0<<7|B0<<6|B0<<5|B0<<4|B0<<3|B0<<2|B0<<1|B0<<0,
  B0<<15|B0<<14|B0<<13|B0<<12|M0<<11|M0<<10|B0<<9|B0<<8|B0<<7|B0<<6|B0<<5|B0<<4|B0<<3|B0<<2|B0<<1|B0<<0,

  /* Plane 1 */
  B1<<15|B1<<14|B1<<13|M1<<12|W1<<11|B1<<10|W1<<9|W1<<8|W1<<7|W1<<6|W1<<5|W1<<4|B1<<3|B1<<2|B1<<1|B1<<0,
  B1<<15|B1<<14|B1<<13|B1<<12|B1<<11|W1<<10|W1<<9|W1<<8|W1<<7|W1<<6|W1<<5|W1<<4|W1<<3|B1<<2|B1<<1|B1<<0,
  B1<<15|B1<<14|B1<<13|B1<<12|W1<<11|W1<<10|W1<<9|M1<<8|M1<<7|B1<<6|M1<<5|B1<<4|M1<<3|B1<<2|B1<<1|B1<<0,
  B1<<15|B1<<14|B1<<13|B1<<12|W1<<11|W1<<10|W1<<9|M1<<8|M1<<7|B1<<6|M1<<5|B1<<4|M1<<3|B1<<2|B1<<1|B1<<0,

  B1<<15|B1<<14|B1<<13|B1<<12|B1<<11|W1<<10|W1<<9|W1<<8|W1<<7|W1<<6|W1<<5|W1<<4|W1<<3|B1<<2|B1<<1|B1<<0,
  B1<<15|B1<<14|B1<<13|B1<<12|B1<<11|L1<<10|W1<<9|W1<<8|W1<<7|W1<<6|W1<<5|W1<<4|B1<<3|B1<<2|B1<<1|B1<<0,
  B1<<15|B1<<14|B1<<13|B1<<12|B1<<11|L1<<10|L1<<9|L1<<8|L1<<7|L1<<6|L1<<5|B1<<4|B1<<3|B1<<2|B1<<1|B1<<0,
  B1<<15|B1<<14|B1<<13|B1<<12|L1<<11|L1<<10|W1<<9|W1<<8|W1<<7|L1<<6|B1<<5|M1<<4|M1<<3|B1<<2|B1<<1|B1<<0,
  
  B1<<15|B1<<14|M1<<13|M1<<12|L1<<11|L1<<10|W1<<9|W1<<8|W1<<7|W1<<6|W1<<5|M1<<4|M1<<3|B1<<2|B1<<1|B1<<0,
  B1<<15|B1<<14|M1<<13|M1<<12|L1<<11|L1<<10|L1<<9|W1<<8|W1<<7|W1<<6|W1<<5|B1<<4|B1<<3|B1<<2|B1<<1|B1<<0,
  B1<<15|B1<<14|B1<<13|B1<<12|L1<<11|L1<<10|L1<<9|L1<<8|L1<<7|L1<<6|B1<<5|B1<<4|M1<<3|B1<<2|B1<<1|B1<<0,
  B1<<15|B1<<14|B1<<13|B1<<12|L1<<11|L1<<10|W1<<9|L1<<8|L1<<7|L1<<6|W1<<5|M1<<4|M1<<3|B1<<2|B1<<1|B1<<0,

  B1<<15|B1<<14|B1<<13|W1<<12|W1<<11|W1<<10|W1<<9|L1<<8|L1<<7|W1<<6|W1<<5|M1<<4|M1<<3|B1<<2|B1<<1|B1<<0,
  B1<<15|B1<<14|M1<<13|M1<<12|W1<<11|W1<<10|W1<<9|B1<<8|L1<<7|L1<<6|W1<<5|M1<<4|M1<<3|B1<<2|B1<<1|B1<<0,
  B1<<15|B1<<14|B1<<13|M1<<12|M1<<11|B1<<10|B1<<9|B1<<8|B1<<7|B1<<6|B1<<5|B1<<4|B1<<3|B1<<2|B1<<1|B1<<0,
  B1<<15|B1<<14|B1<<13|B1<<12|M1<<11|M1<<10|B1<<9|B1<<8|B1<<7|B1<<6|B1<<5|B1<<4|B1<<3|B1<<2|B1<<1|B1<<0,

  /* Plane 2 */
  B2<<15|B2<<14|B2<<13|M2<<12|W2<<11|B2<<10|W2<<9|W2<<8|W2<<7|W2<<6|W2<<5|W2<<4|B2<<3|B2<<2|B2<<1|B2<<0,
  B2<<15|B2<<14|B2<<13|B2<<12|B2<<11|W2<<10|W2<<9|W2<<8|W2<<7|W2<<6|W2<<5|W2<<4|W2<<3|B2<<2|B2<<1|B2<<0,
  B2<<15|B2<<14|B2<<13|B2<<12|W2<<11|W2<<10|W2<<9|M2<<8|M2<<7|B2<<6|M2<<5|B2<<4|M2<<3|B2<<2|B2<<1|B2<<0,
  B2<<15|B2<<14|B2<<13|B2<<12|W2<<11|W2<<10|W2<<9|M2<<8|M2<<7|B2<<6|M2<<5|B2<<4|M2<<3|B2<<2|B2<<1|B2<<0,

  B2<<15|B2<<14|B2<<13|B2<<12|B2<<11|W2<<10|W2<<9|W2<<8|W2<<7|W2<<6|W2<<5|W2<<4|W2<<3|B2<<2|B2<<1|B2<<0,
  B2<<15|B2<<14|B2<<13|B2<<12|B2<<11|L2<<10|W2<<9|W2<<8|W2<<7|W2<<6|W2<<5|W2<<4|B2<<3|B2<<2|B2<<1|B2<<0,
  B2<<15|B2<<14|B2<<13|B2<<12|B2<<11|L2<<10|L2<<9|L2<<8|L2<<7|L2<<6|L2<<5|B2<<4|B2<<3|B2<<2|B2<<1|B2<<0,
  B2<<15|B2<<14|B2<<13|B2<<12|L2<<11|L2<<10|W2<<9|W2<<8|W2<<7|L2<<6|B2<<5|M2<<4|M2<<3|B2<<2|B2<<1|B2<<0,
  
  B2<<15|B2<<14|M2<<13|M2<<12|L2<<11|L2<<10|W2<<9|W2<<8|W2<<7|W2<<6|W2<<5|M2<<4|M2<<3|B2<<2|B2<<1|B2<<0,
  B2<<15|B2<<14|M2<<13|M2<<12|L2<<11|L2<<10|L2<<9|W2<<8|W2<<7|W2<<6|W2<<5|B2<<4|B2<<3|B2<<2|B2<<1|B2<<0,
  B2<<15|B2<<14|B2<<13|B2<<12|L2<<11|L2<<10|L2<<9|L2<<8|L2<<7|L2<<6|B2<<5|B2<<4|M2<<3|B2<<2|B2<<1|B2<<0,
  B2<<15|B2<<14|B2<<13|B2<<12|L2<<11|L2<<10|W2<<9|L2<<8|L2<<7|L2<<6|W2<<5|M2<<4|M2<<3|B2<<2|B2<<1|B2<<0,

  B2<<15|B2<<14|B2<<13|W2<<12|W2<<11|W2<<10|W2<<9|L2<<8|L2<<7|W2<<6|W2<<5|M2<<4|M2<<3|B2<<2|B2<<1|B2<<0,
  B2<<15|B2<<14|M2<<13|M2<<12|W2<<11|W2<<10|W2<<9|B2<<8|L2<<7|L2<<6|W2<<5|M2<<4|M2<<3|B2<<2|B2<<1|B2<<0,
  B2<<15|B2<<14|B2<<13|M2<<12|M2<<11|B2<<10|B2<<9|B2<<8|B2<<7|B2<<6|B2<<5|B2<<4|B2<<3|B2<<2|B2<<1|B2<<0,
  B2<<15|B2<<14|B2<<13|B2<<12|M2<<11|M2<<10|B2<<9|B2<<8|B2<<7|B2<<6|B2<<5|B2<<4|B2<<3|B2<<2|B2<<1|B2<<0,
  
  /* Plane 3 */
  B3<<15|B3<<14|B3<<13|M3<<12|W3<<11|B3<<10|W3<<9|W3<<8|W3<<7|W3<<6|W3<<5|W3<<4|B3<<3|B3<<2|B3<<1|B3<<0,
  B3<<15|B3<<14|B3<<13|B3<<12|B3<<11|W3<<10|W3<<9|W3<<8|W3<<7|W3<<6|W3<<5|W3<<4|W3<<3|B3<<2|B3<<1|B3<<0,
  B3<<15|B3<<14|B3<<13|B3<<12|W3<<11|W3<<10|W3<<9|M3<<8|M3<<7|B3<<6|M3<<5|B3<<4|M3<<3|B3<<2|B3<<1|B3<<0,
  B3<<15|B3<<14|B3<<13|B3<<12|W3<<11|W3<<10|W3<<9|M3<<8|M3<<7|B3<<6|M3<<5|B3<<4|M3<<3|B3<<2|B3<<1|B3<<0,

  B3<<15|B3<<14|B3<<13|B3<<12|B3<<11|W3<<10|W3<<9|W3<<8|W3<<7|W3<<6|W3<<5|W3<<4|W3<<3|B3<<2|B3<<1|B3<<0,
  B3<<15|B3<<14|B3<<13|B3<<12|B3<<11|L3<<10|W3<<9|W3<<8|W3<<7|W3<<6|W3<<5|W3<<4|B3<<3|B3<<2|B3<<1|B3<<0,
  B3<<15|B3<<14|B3<<13|B3<<12|B3<<11|L3<<10|L3<<9|L3<<8|L3<<7|L3<<6|L3<<5|B3<<4|B3<<3|B3<<2|B3<<1|B3<<0,
  B3<<15|B3<<14|B3<<13|B3<<12|L3<<11|L3<<10|W3<<9|W3<<8|W3<<7|L3<<6|B3<<5|M3<<4|M3<<3|B3<<2|B3<<1|B3<<0,
  
  B3<<15|B3<<14|M3<<13|M3<<12|L3<<11|L3<<10|W3<<9|W3<<8|W3<<7|W3<<6|W3<<5|M3<<4|M3<<3|B3<<2|B3<<1|B3<<0,
  B3<<15|B3<<14|M3<<13|M3<<12|L3<<11|L3<<10|L3<<9|W3<<8|W3<<7|W3<<6|W3<<5|B3<<4|B3<<3|B3<<2|B3<<1|B3<<0,
  B3<<15|B3<<14|B3<<13|B3<<12|L3<<11|L3<<10|L3<<9|L3<<8|L3<<7|L3<<6|B3<<5|B3<<4|M3<<3|B3<<2|B3<<1|B3<<0,
  B3<<15|B3<<14|B3<<13|B3<<12|L3<<11|L3<<10|W3<<9|L3<<8|L3<<7|L3<<6|W3<<5|M3<<4|M3<<3|B3<<2|B3<<1|B3<<0,

  B3<<15|B3<<14|B3<<13|W3<<12|W3<<11|W3<<10|W3<<9|L3<<8|L3<<7|W3<<6|W3<<5|M3<<4|M3<<3|B3<<2|B3<<1|B3<<0,
  B3<<15|B3<<14|M3<<13|M3<<12|W3<<11|W3<<10|W3<<9|B<3<8|L3<<7|L3<<6|W3<<5|M3<<4|M3<<3|B3<<2|B3<<1|B3<<0,
  B3<<15|B3<<14|B3<<13|M3<<12|M3<<11|B3<<10|B3<<9|B3<<8|B3<<7|B3<<6|B3<<5|B3<<4|B3<<3|B3<<2|B3<<1|B3<<0,
  B3<<15|B3<<14|B3<<13|B3<<12|M3<<11|M3<<10|B3<<9|B3<<8|B3<<7|B3<<6|B3<<5|B3<<4|B3<<3|B3<<2|B3<<1|B3<<0
 };


/* To ROM */
#incbin(Palette1, "SPR_Palette.bin")
#incbin(Pattern1, "SPR_Pattern.bin")

PutSprites()
{
  u8 i;
  for(i = 0;i < SPR_MAX;++i){
      spr_set(i);
      spr_x((rand() % SCREEN_WIDTH) - 8);
      spr_y((rand() % SCREEN_HEIGHT) - 8);
  }
}

#define SPR_VRAM 0x4000/*0x6000*/
#define SPR_PLANES 4

#define SPR_WSIZE_16X16 (16 * 16 * SPR_PLANES / sizeof(u16))
main()
{
  u8 KeyState;
  u8 i;

  /* To VRAM */
  load_vram(SPR_VRAM, Pattern0, SPR_WSIZE_16X16);
  load_vram(SPR_VRAM + SPR_WSIZE_16X16, Pattern1, SPR_WSIZE_16X16);

  /*load_palette(PALETTE_SPR00, Palette, 1);*/
  set_sprpal(0, Palette0, 1);
  set_sprpal(1, Palette1, 1);

  /* Initialize Sprite Attribute Table */
  init_satb();

  for(i = 0;i < SPR_MAX;++i) {
    /* Sprite no [0, 63] */
    spr_set(i);
    /* Size, Flip settings */
    /*
    SZ_16x16, SZ_16x32, SZ_16x64, SZ_32x16, SZ_32x32, SZ_32x64
    */
    spr_ctrl(SIZE_MAS | FLIP_MAS, SZ_16x16 | NO_FLIP);
    /*spr_ctrl(SIZE_MAS | FLIP_MAS, SZ_16x16 | FLIP_X | FLIP_Y);*/
    /* Pattern (VRAM) */
    spr_pattern(SPR_VRAM + (i & 1) * SPR_WSIZE_16X16);
    /* Palette [0, 15] */
    spr_pal((i & 1));
    /* Priority [0, 1] */
    /* 0 : Behind of BG, 1 : Front of BG */
    spr_pri(1);
    /* Hide, show */
    spr_hide();
    spr_show();
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
   
    /* Sprite Attribute Table update */
    satb_update();
  }
}