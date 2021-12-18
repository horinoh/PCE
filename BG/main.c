#include "huc.h"

#include "..\define.h"

#define MAP_WIDTH  48
#define MAP_HEIGHT 12
const char Map[] = {
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,6,5,5,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,5,5,4,5,4,5,4,5,4,5,4, 4,4,5,5,4,5,5,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,5,5,4,6,4,5,4,5,4,5,4, 5,5,5,5,4,5,5,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,5,5,4,4,4,5,4,5,4,5,4, 5,5,5,5,4,5,5,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,5,5,4,5,4,5,4,5,4,5,4, 6,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,5,5,4,5,4,5,4,4,4,5,4, 4,4,5,5,4,5,6,5,5,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5, 5,5,5,5,5,0,8,2,2,2,2,2,2,2,2,2, 2,2,2,0,0,0,2,2,0,5,5,5,5,5,5,5,
	5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,6, 5,7,7,7,7,0,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,0,5,5,5,5,5,5,6,
	2,5,5,5,5,5,6,5,5,5,5,5,5,5,5,8, 2,2,2,2,2,2,1,1,1,1,1,1,1,1,1,1, 1,1,1,1,1,1,1,1,0,9,9,9,9,9,8,2,
	1,5,7,7,2,2,2,0,0,8,9,9,9,9,9,0, 1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0,0,3,3,3,3,3,0,1,
	1,2,2,2,1,1,1,1,1,0,3,3,3,3,3,0, 1,1,1,1,1,1,1,1,1,0,3,3,3,3,3,3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,1,
	1,1,1,1,1,1,1,1,1,0,3,3,3,3,3,0, 1,1,1,1,1,1,1,1,1,0,3,3,3,3,3,3, 3,3,3,3,3,3,3,3,3,3,3,3,3,3,0,1
};

#define WIN_H SCREEN_HEIGHT / 4

main()
{
  u16 i;

  /*
  Virtual screnn size
  SCR_SIZE_32x32, SCR_SIZE_64x32, SCR_SIZE_128x32, SCR_SIZE_32x64, SCR_SIZE_64x64, SCR_SIZE_128x64
  (Default is SCR_SIZE_64x32)
  */
  set_screen_size(SCR_SIZE_64x32);

  /*
  set_xres(256, XRES_SHARP);
  gfx_line(0, 0, 10, 10, 1);
  gfx_plot(10, 10, 1); 
   */
 
 /*
  set_bgpal(0, Palette, 1);
  
  set_map_data(Map, MAP_WIDTH, MAP_HEIGHT);
  {
	  set_tile_data(Tile, 12, tile_pal_ref);
  	load_tile(0x1000);
  }
	load_map(0, 0, 0, 0, 17, 12);
*/

  while(1){
    vsync();
  }
}