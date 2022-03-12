#include "huc.h"

#include "..\define.h"

/* Convert data */
#incbin(Palette, "res/32x16mapchip_20190721_PAL.bin");
#incbin(Pattern, "res/32x16mapchip_20190721_PAT.bin");
#incbin(PatternPalette, "res/32x16mapchip_20190721_PAT.pal.bin");
#incbin(Map, "res/32x16mapchip_20190721_MAP.bin");
#define BG_PAL_COUNT 4
#define BG_PAT_COUNT 168
#define BG_MAP_W 32
#define BG_MAP_H 16
/* Convert data */

#define SCR_TILE_W (SCREEN_WIDTH>>4)
#define SCR_TILE_H (SCREEN_HEIGHT>>4)

#define TILE_VRAM 0x5000

main()
{  
  int x, y;
  int MapX, MapY;
  u8 KeyState;
  x = y = 0;
  MapX = MapY = 0;

  set_map_data(Map, BG_MAP_W, BG_MAP_H);  
  set_tile_data(Pattern, BG_PAT_COUNT, PatternPalette);

  load_tile(TILE_VRAM);
  load_palette(0, Palette, BG_PAL_COUNT);
  load_map(x >> 4, y >> 4, MapX, MapY, SCR_TILE_W, SCR_TILE_H);

  while(1) {
    KeyState = joy(JOY_PAD0);

    if(KeyState & JOY_UP) { y = (y - 1) & 0xff; }
    /*
    if(KeyState & JOY_RGHT) { ++x; }
    if(KeyState & JOY_LEFT) { --x; }
    */

    if(!(y & 15)) {
      --MapY;
      if(MapY < 0) { MapY += BG_MAP_H; }
      load_map(x >> 4, (y >> 4) - 1, MapX, MapY - 1, SCR_TILE_W, 1);
    }

    scroll(WIN0, x, y, 0, SCREEN_HEIGHT - 1, SCR_BG_ON);
    
    vsync();
  }
}