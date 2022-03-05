#include "huc.h"

#include "..\define.h"

#incbin(Palette, "res/PaletteBG.bin");
#incbin(Pattern, "res/PatternBG.bin");
#incbin(PatternPalette, "res/PatternBG.pal.bin");
#incbin(Map, "res/MapBG.bin");

#define MAP_TILE_W 32
#define MAP_TILE_H 16

#define SCR_TILE_W (SCREEN_WIDTH>>4)
#define SCR_TILE_H (SCREEN_HEIGHT>>4)

#define TILE_VRAM 0x5000

/* Set convert result here */
#define BG_MAP_W 4
#define BG_MAP_H 4
#define BG_PAT_COUNT 16
#define BG_PAL_COUNT 1

main()
{  
  int x, y;
  int MapX, MapY;
  u8 KeyState;
  x = y = 0;
  MapX = MapY = 0;

  /*set_map_data(Map, MAP_TILE_W, MAP_TILE_H);  */
  set_map_data(Map, BG_MAP_W, BG_MAP_H);  
  set_tile_data(Pattern, BG_PAT_COUNT, PatternPalette);

  load_tile(TILE_VRAM);
  load_palette(0, Palette, BG_PAL_COUNT);
  load_map(x >> 4, y >> 4, MapX, MapY, SCR_TILE_W, SCR_TILE_H);

  while(1) {
    KeyState = joy(JOY_PAD0);
    if(KeyState & JOY_UP) { y = (y - 1) & 0xff; }

    if(!(y & 15)) {
      --MapY;
      if(MapY < 0) { MapY += MAP_TILE_H; }
      load_map(x >> 4, (y >> 4) - 1, MapX, MapY - 1, SCR_TILE_W, 1);
    }

    scroll(WIN0, x, y, 0, SCREEN_HEIGHT - 1, SCR_BG_ON);
    
    vsync();
  }
}