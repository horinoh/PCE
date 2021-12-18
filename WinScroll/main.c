#include "huc.h"

#include "..\define.h"

#define WIN_H SCREEN_HEIGHT / 4

main()
{
  u16 i, x, y;
  x = y = 0;

  /*
  Virtual screnn size
  SCR_SIZE_32x32, SCR_SIZE_64x32, SCR_SIZE_128x32, SCR_SIZE_32x64, SCR_SIZE_64x64, SCR_SIZE_128x64
  (Default is SCR_SIZE_64x32)
  */
  set_screen_size(SCR_SIZE_64x32);

  /* Here SCR_SIZE_64x32 */
  for(i = 0;i < 32;++i) {
    put_string("[  ]ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz01234567", 0, i); /* 4 + 26 + 26 + 8 = 64 */
    put_number(i, 2, 1, i);
  }

  while(1){
    vsync();

    scroll(WIN0,  x,  y, 0 * WIN_H, 1 * WIN_H - 1, SCR_SPR_ON | SCR_BG_ON);
    scroll(WIN1,  x, -y, 1 * WIN_H, 2 * WIN_H - 1, SCR_SPR_ON | SCR_BG_ON); 
    scroll(WIN2, -x,  y, 2 * WIN_H, 3 * WIN_H - 1, SCR_SPR_ON | SCR_BG_ON); 
    scroll(WIN3, -x, -y, 3 * WIN_H, 4 * WIN_H - 1, SCR_SPR_ON | SCR_BG_ON); 

    if(joy(JOY_PAD0)){
      x++;
    } else {
      y++;
    }
  }
}