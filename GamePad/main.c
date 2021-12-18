#include "huc.h"

#include "..\define.h"

onStartPressed()
{
  static u8 StartCount;
  put_string("START", 10, StartCount);
  StartCount = ++StartCount % (SCREEN_HEIGHT / 8);

  /*
  do {
		vsync();
	} while (!(joytrg(JOY_PAD0) & JOY_STRT));

  clear_joy_events(JOY_PAD0_MAS);
  */
 }

main()
{
  u8 KeyState;
  u8 KeyTrig;
  u16 x, y;
  x = 0; y = 1;
  
  set_joy_callback(0, JOY_PAD0_MAS, JOY_STRT, onStartPressed);

  while(1){
    vsync();

    get_joy_events(JOY_PAD0);

    KeyState = joy(JOY_PAD0);
    KeyTrig = joytrg(JOY_PAD0);
    
    put_number(KeyState, 3, 0, 0);

    put_string("DEFAULT KEY MAPPING", 0, SCREEN_HEIGHT / 8 - 7); 
    put_string("URDL : Arrow ", 0, SCREEN_HEIGHT / 8 - 5); 
    put_string("SEL  : Tab", 0, SCREEN_HEIGHT / 8 - 4);
    put_string("RUN  : Enter", 0, SCREEN_HEIGHT / 8 - 3);
    put_string("II   : Space", 0, SCREEN_HEIGHT / 8 - 2);
    put_string("I    : N", 0, SCREEN_HEIGHT / 8 - 1);

    /* Up, Right, Down, Left : Arrow key */
    (KeyState & JOY_UP) ? put_char('+', 1 + x, 0 + y) : put_char('-', 1 + x, 0 + y);
    (KeyState & JOY_RGHT) ? put_char('+', 2 + x, 1 + y) : put_char('-', 2 + x, 1 + y);
    (KeyState & JOY_DOWN) ? put_char('+', 1 + x, 2 + y) : put_char('-', 1 + x, 2 + y);
    (KeyState & JOY_LEFT) ? put_char('+', 0 + x, 1 + y) : put_char('-', 0 + x, 1 + y);
    if(KeyTrig & JOY_UP) put_char('@', 1 + x, 0 + y) ;
    if(KeyTrig & JOY_RGHT) put_char('@', 2 + x, 1 + y);
    if(KeyTrig & JOY_DOWN) put_char('@', 1 + x, 2 + y);
    if(KeyTrig & JOY_LEFT) put_char('@', 0 + x, 1 + y);

    /* Select, Run : Tab, Enter */
    (KeyState & JOY_SLCT) ? put_char('+', 4 + x, 2 + y) : put_char('-', 4 + x, 2 + y);
    (KeyState & JOY_STRT) ? put_char('+', 5 + x, 2 + y) : put_char('-', 5 + x, 2 + y);
    if(KeyTrig & JOY_SLCT) put_char('@', 4 + x, 2 + y);
    if(KeyTrig & JOY_STRT) put_char('@', 5 + x, 2 + y);
  
    /* II, I : Space, N */
    (KeyState & JOY_B) ? put_char('+', 7 + x, 2 + y) : put_char('-', 7 + x, 2 + y);
    (KeyState & JOY_A) ? put_char('+', 8 + x, 2 + y) : put_char('-', 8 + x, 2 + y);
    if(KeyTrig & JOY_B) put_char('@', 7 + x, 2 + y);
    if(KeyTrig & JOY_A) put_char('@', 8 + x, 2 + y);
  }
}