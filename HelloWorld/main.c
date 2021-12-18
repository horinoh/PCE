#include "huc.h"

#include "..\define.h"

main()
{
  u16 i;

  put_string("Hello World", 0, SCREEN_HEIGHT / 8 - 1);
  
  /* Character code [32, 255]*/
  for(i = 0; i < 256 - 32;i++){
    put_char(i + 32, i % 16, i / 16);
  }

  while(1){
    vsync();
  }
}