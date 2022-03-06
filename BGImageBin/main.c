#include "huc.h"

#include "..\define.h"

#incbin(Pattern, "res/PatternIMAGE.bin");
#incbin(Palette, "res/PaletteIMAGE.bin");
#incbin(BAT, "res/BAT.bin");

main()
{
  /* Not suit for scroll, suit for still image */
  load_background(Pattern, Palette, BAT, SCREEN_CELLS_W, SCREEN_CELLS_H);

  while(1){
    vsync();  
  }
}
