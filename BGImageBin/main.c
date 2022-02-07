#include "huc.h"

#include "..\define.h"

#incbin(Pattern, "Pattern.bin");
#incbin(Palette, "Palette.bin");
#incbin(BAT, "BAT.bin");

main()
{
  /* Not suit for scroll, suit for still image */
  /* BG display range is 32 x 28 cell (256 x 224 pixel) */
  load_background(Pattern, Palette, BAT, 32, 28);

  while(1){
    vsync();
  }
}
