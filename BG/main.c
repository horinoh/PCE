#include "huc.h"

#include "..\define.h"

const u16 Palette[] = {
  COLOR_BACKGROUND,
  COLOR_WHITE, 
  COLOR_MAGENTA, 
  COLOR_LIGHTBLUE, 
  COLOR_YELLOW,
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

/* Index of COLOR_BLACK, COLOR_WHITE, COLOR_MAGENTA, COLOR_LIGHTBLUE */
#define T 0
#define W 1 
#define M 2 
#define L 3
#define Y 4

/* Plane masked values */
#define T0 0 /* (Index of COLOR_TRANSPARENT & 1) ? 1 : 0 */
#define T1 0 /* (Index of COLOR_TRANSPARENT & 2) ? 1 : 0 */
#define T2 0 /* (Index of COLOR_TRANSPARENT & 4) ? 1 : 0 */
#define T3 0 /* (Index of COLOR_TRANSPARENT & 8) ? 1 : 0 */

#define W0 1
#define W1 0 
#define W2 0 
#define W3 0

#define M0 0 
#define M1 1 
#define M2 0 
#define M3 0 

#define L0 1 
#define L1 1 
#define L2 0 
#define L3 0

#define Y0 0
#define Y1 0 
#define Y2 1 
#define Y3 0

/*
 8 x 8 : 4 プレーン
  最初の 8 要素の下位、上位 8 ビットで、プレーン 0, 1　を表す
  続く 8 要素の下位、上位 8 ビットで、プレーン 2, 3 を表す

11111111 00000000
11111111 00000000
11111111 00000000
11111111 00000000
11111111 00000000
11111111 00000000
11111111 00000000
11111111 00000000
                    x 4
33333333 22222222
33333333 22222222
33333333 22222222
33333333 22222222
33333333 22222222
33333333 22222222
33333333 22222222
33333333 22222222

8 x 8 が 4 つ一組でタイル (16 x 16) となる
マップにはタイル単位で配置していくことになる
*/
const u16 Pattern[] = {
  /* Tile0(LT) Plane 0, 1 */
  T1<<15|T1<<14|T1<<13|W1<<12|W1<<11|T1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|T0<<5|W0<<4|W0<<3|T0<<2|T0<<1|T0<<0,
  T1<<15|T1<<14|W1<<13|W1<<12|W1<<11|W1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|W0<<5|W0<<4|W0<<3|W0<<2|T0<<1|T0<<0,
  T1<<15|W1<<14|W1<<13|W1<<12|W1<<11|W1<<10|W1<<9|T1<<8 | T0<<7|W0<<6|W0<<5|W0<<4|W0<<3|W0<<2|W0<<1|T0<<0,
  W1<<15|W1<<14|T1<<13|W1<<12|W1<<11|T1<<10|W1<<9|W1<<8 | W0<<7|W0<<6|T0<<5|W0<<4|W0<<3|T0<<2|W0<<1|W0<<0,
  W1<<15|W1<<14|W1<<13|W1<<12|W1<<11|W1<<10|W1<<9|W1<<8 | W0<<7|W0<<6|W0<<5|W0<<4|W0<<3|W0<<2|W0<<1|W0<<0,
  T1<<15|T1<<14|W1<<13|T1<<12|T1<<11|W1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|W0<<5|T0<<4|T0<<3|W0<<2|T0<<1|T0<<0,
  T1<<15|W1<<14|T1<<13|W1<<12|W1<<11|T1<<10|W1<<9|T1<<8 | T0<<7|W0<<6|T0<<5|W0<<4|W0<<3|T0<<2|W0<<1|T0<<0,
  W1<<15|T1<<14|W1<<13|T1<<12|T1<<11|W1<<10|T1<<9|W1<<8 | W0<<7|T0<<6|W0<<5|T0<<4|T0<<3|W0<<2|T0<<1|W0<<0,
  /* Tile0(LT) Plane 2, 3 */
  T3<<15|T3<<14|T3<<13|W3<<12|W3<<11|T3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|T2<<5|W2<<4|W2<<3|T2<<2|T2<<1|T2<<0,
  T3<<15|T3<<14|W3<<13|W3<<12|W3<<11|W3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|W2<<5|W2<<4|W2<<3|W2<<2|T2<<1|T2<<0,
  T3<<15|W3<<14|W3<<13|W3<<12|W3<<11|W3<<10|W3<<9|T3<<8 | T2<<7|W2<<6|W2<<5|W2<<4|W2<<3|W2<<2|W2<<1|T2<<0,
  W3<<15|W3<<14|T3<<13|W3<<12|W3<<11|T3<<10|W3<<9|W3<<8 | W2<<7|W2<<6|T2<<5|W2<<4|W2<<3|T2<<2|W2<<1|W2<<0,
  W3<<15|W3<<14|W3<<13|W3<<12|W3<<11|W3<<10|W3<<9|W3<<8 | W2<<7|W2<<6|W2<<5|W2<<4|W2<<3|W2<<2|W2<<1|W2<<0,
  T3<<15|T3<<14|W3<<13|T3<<12|T3<<11|W3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|W2<<5|T2<<4|T2<<3|W2<<2|T2<<1|T2<<0,
  T3<<15|W3<<14|T3<<13|W3<<12|W3<<11|T3<<10|W3<<9|T3<<8 | T2<<7|W2<<6|T2<<5|W2<<4|W2<<3|T2<<2|W2<<1|T2<<0,
  W3<<15|T3<<14|W3<<13|T3<<12|T3<<11|W3<<10|T3<<9|W3<<8 | W2<<7|T2<<6|W2<<5|T2<<4|T2<<3|W2<<2|T2<<1|W2<<0,

  /* Tile0(RT) Plane 0, 1 */
  T1<<15|T1<<14|T1<<13|T1<<12|M1<<11|M1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|T0<<5|T0<<4|M0<<3|M0<<2|T0<<1|T0<<0,
  T1<<15|T1<<14|T1<<13|T1<<12|M1<<11|T1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|T0<<5|T0<<4|M0<<3|T0<<2|T0<<1|T0<<0,
  T1<<15|M1<<14|M1<<13|M1<<12|M1<<11|M1<<10|M1<<9|T1<<8 | T0<<7|M0<<6|M0<<5|M0<<4|M0<<3|M0<<2|M0<<1|T0<<0,
  M1<<15|M1<<14|M1<<13|M1<<12|M1<<11|M1<<10|M1<<9|M1<<8 | M0<<7|M0<<6|M0<<5|M0<<4|M0<<3|M0<<2|M0<<1|M0<<0,
  M1<<15|T1<<14|M1<<13|M1<<12|M1<<11|M1<<10|M1<<9|M1<<8 | M0<<7|T0<<6|M0<<5|M0<<4|M0<<3|M0<<2|M0<<1|M0<<0,
  M1<<15|T1<<14|M1<<13|M1<<12|M1<<11|M1<<10|M1<<9|M1<<8 | M0<<7|T0<<6|M0<<5|M0<<4|M0<<3|M0<<2|M0<<1|M0<<0,
  M1<<15|M1<<14|M1<<13|M1<<12|M1<<11|M1<<10|M1<<9|M1<<8 | M0<<7|M0<<6|M0<<5|M0<<4|M0<<3|M0<<2|M0<<1|M0<<0,
  T1<<15|M1<<14|M1<<13|M1<<12|M1<<11|M1<<10|M1<<9|T1<<8 | T0<<7|M0<<6|M0<<5|M0<<4|M0<<3|M0<<2|M0<<1|T0<<0,
  /* Tile0(RT) Plane 2, 3 */
  T3<<15|T3<<14|T3<<13|T3<<12|M3<<11|M3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|T2<<5|T2<<4|M2<<3|M2<<2|T2<<1|T2<<0,
  T3<<15|T3<<14|T3<<13|T3<<12|M3<<11|T3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|T2<<5|T2<<4|M2<<3|T2<<2|T2<<1|T2<<0,
  T3<<15|M3<<14|M3<<13|M3<<12|M3<<11|M3<<10|M3<<9|T3<<8 | T2<<7|M2<<6|M2<<5|M2<<4|M2<<3|M2<<2|M2<<1|T2<<0,
  M3<<15|M3<<14|M3<<13|M3<<12|M3<<11|M3<<10|M3<<9|M3<<8 | M2<<7|M2<<6|M2<<5|M2<<4|M2<<3|M2<<2|M2<<1|M2<<0,
  M3<<15|T3<<14|M3<<13|M3<<12|M3<<11|M3<<10|M3<<9|M3<<8 | M2<<7|T2<<6|M2<<5|M2<<4|M2<<3|M2<<2|M2<<1|M2<<0,
  M3<<15|T3<<14|M3<<13|M3<<12|M3<<11|M3<<10|M3<<9|M3<<8 | M2<<7|T2<<6|M2<<5|M2<<4|M2<<3|M2<<2|M2<<1|M2<<0,
  M3<<15|M3<<14|M3<<13|M3<<12|M3<<11|M3<<10|M3<<9|M3<<8 | M2<<7|M2<<6|M2<<5|M2<<4|M2<<3|M2<<2|M2<<1|M2<<0,
  T3<<15|M3<<14|M3<<13|M3<<12|M3<<11|M3<<10|M3<<9|T3<<8 | T2<<7|M2<<6|M2<<5|M2<<4|M2<<3|M2<<2|M2<<1|T2<<0,

  /* Tile0(LB) Plane 0, 1 */
  T1<<15|T1<<14|T1<<13|T1<<12|M1<<11|M1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|T0<<5|T0<<4|M0<<3|M0<<2|T0<<1|T0<<0,
  T1<<15|T1<<14|T1<<13|T1<<12|M1<<11|T1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|T0<<5|T0<<4|M0<<3|T0<<2|T0<<1|T0<<0,
  T1<<15|M1<<14|M1<<13|M1<<12|M1<<11|M1<<10|M1<<9|T1<<8 | T0<<7|M0<<6|M0<<5|M0<<4|M0<<3|M0<<2|M0<<1|T0<<0,
  M1<<15|M1<<14|M1<<13|M1<<12|M1<<11|M1<<10|M1<<9|M1<<8 | M0<<7|M0<<6|M0<<5|M0<<4|M0<<3|M0<<2|M0<<1|M0<<0,
  M1<<15|T1<<14|M1<<13|M1<<12|M1<<11|M1<<10|M1<<9|M1<<8 | M0<<7|T0<<6|M0<<5|M0<<4|M0<<3|M0<<2|M0<<1|M0<<0,
  M1<<15|T1<<14|M1<<13|M1<<12|M1<<11|M1<<10|M1<<9|M1<<8 | M0<<7|T0<<6|M0<<5|M0<<4|M0<<3|M0<<2|M0<<1|M0<<0,
  M1<<15|M1<<14|M1<<13|M1<<12|M1<<11|M1<<10|M1<<9|M1<<8 | M0<<7|M0<<6|M0<<5|M0<<4|M0<<3|M0<<2|M0<<1|M0<<0,
  T1<<15|M1<<14|M1<<13|M1<<12|M1<<11|M1<<10|M1<<9|T1<<8 | T0<<7|M0<<6|M0<<5|M0<<4|M0<<3|M0<<2|M0<<1|T0<<0,
  /* Tile0(LB) Plane 2, 3 */
  T3<<15|T3<<14|T3<<13|T3<<12|M3<<11|M3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|T2<<5|T2<<4|M2<<3|M2<<2|T2<<1|T2<<0,
  T3<<15|T3<<14|T3<<13|T3<<12|M3<<11|T3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|T2<<5|T2<<4|M2<<3|T2<<2|T2<<1|T2<<0,
  T3<<15|M3<<14|M3<<13|M3<<12|M3<<11|M3<<10|M3<<9|T3<<8 | T2<<7|M2<<6|M2<<5|M2<<4|M2<<3|M2<<2|M2<<1|T2<<0,
  M3<<15|M3<<14|M3<<13|M3<<12|M3<<11|M3<<10|M3<<9|M3<<8 | M2<<7|M2<<6|M2<<5|M2<<4|M2<<3|M2<<2|M2<<1|M2<<0,
  M3<<15|T3<<14|M3<<13|M3<<12|M3<<11|M3<<10|M3<<9|M3<<8 | M2<<7|T2<<6|M2<<5|M2<<4|M2<<3|M2<<2|M2<<1|M2<<0,
  M3<<15|T3<<14|M3<<13|M3<<12|M3<<11|M3<<10|M3<<9|M3<<8 | M2<<7|T2<<6|M2<<5|M2<<4|M2<<3|M2<<2|M2<<1|M2<<0,
  M3<<15|M3<<14|M3<<13|M3<<12|M3<<11|M3<<10|M3<<9|M3<<8 | M2<<7|M2<<6|M2<<5|M2<<4|M2<<3|M2<<2|M2<<1|M2<<0,
  T3<<15|M3<<14|M3<<13|M3<<12|M3<<11|M3<<10|M3<<9|T3<<8 | T2<<7|M2<<6|M2<<5|M2<<4|M2<<3|M2<<2|M2<<1|T2<<0,

  /* Tile0(RB) Plane 0, 1 */
  T1<<15|T1<<14|T1<<13|W1<<12|W1<<11|T1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|T0<<5|W0<<4|W0<<3|T0<<2|T0<<1|T0<<0,
  T1<<15|T1<<14|W1<<13|W1<<12|W1<<11|W1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|W0<<5|W0<<4|W0<<3|W0<<2|T0<<1|T0<<0,
  T1<<15|W1<<14|W1<<13|W1<<12|W1<<11|W1<<10|W1<<9|T1<<8 | T0<<7|W0<<6|W0<<5|W0<<4|W0<<3|W0<<2|W0<<1|T0<<0,
  W1<<15|W1<<14|T1<<13|W1<<12|W1<<11|T1<<10|W1<<9|W1<<8 | W0<<7|W0<<6|T0<<5|W0<<4|W0<<3|T0<<2|W0<<1|W0<<0,
  W1<<15|W1<<14|W1<<13|W1<<12|W1<<11|W1<<10|W1<<9|W1<<8 | W0<<7|W0<<6|W0<<5|W0<<4|W0<<3|W0<<2|W0<<1|W0<<0,
  T1<<15|T1<<14|W1<<13|T1<<12|T1<<11|W1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|W0<<5|T0<<4|T0<<3|W0<<2|T0<<1|T0<<0,
  T1<<15|W1<<14|T1<<13|W1<<12|W1<<11|T1<<10|W1<<9|T1<<8 | T0<<7|W0<<6|T0<<5|W0<<4|W0<<3|T0<<2|W0<<1|T0<<0,
  W1<<15|T1<<14|W1<<13|T1<<12|T1<<11|W1<<10|T1<<9|W1<<8 | W0<<7|T0<<6|W0<<5|T0<<4|T0<<3|W0<<2|T0<<1|W0<<0,
  /* Tile0(RB) Plane 2, 3 */
  T3<<15|T3<<14|T3<<13|W3<<12|W3<<11|T3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|T2<<5|W2<<4|W2<<3|T2<<2|T2<<1|T2<<0,
  T3<<15|T3<<14|W3<<13|W3<<12|W3<<11|W3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|W2<<5|W2<<4|W2<<3|W2<<2|T2<<1|T2<<0,
  T3<<15|W3<<14|W3<<13|W3<<12|W3<<11|W3<<10|W3<<9|T3<<8 | T2<<7|W2<<6|W2<<5|W2<<4|W2<<3|W2<<2|W2<<1|T2<<0,
  W3<<15|W3<<14|T3<<13|W3<<12|W3<<11|T3<<10|W3<<9|W3<<8 | W2<<7|W2<<6|T2<<5|W2<<4|W2<<3|T2<<2|W2<<1|W2<<0,
  W3<<15|W3<<14|W3<<13|W3<<12|W3<<11|W3<<10|W3<<9|W3<<8 | W2<<7|W2<<6|W2<<5|W2<<4|W2<<3|W2<<2|W2<<1|W2<<0,
  T3<<15|T3<<14|W3<<13|T3<<12|T3<<11|W3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|W2<<5|T2<<4|T2<<3|W2<<2|T2<<1|T2<<0,
  T3<<15|W3<<14|T3<<13|W3<<12|W3<<11|T3<<10|W3<<9|T3<<8 | T2<<7|W2<<6|T2<<5|W2<<4|W2<<3|T2<<2|W2<<1|T2<<0,
  W3<<15|T3<<14|W3<<13|T3<<12|T3<<11|W3<<10|T3<<9|W3<<8 | W2<<7|T2<<6|W2<<5|T2<<4|T2<<3|W2<<2|T2<<1|W2<<0,

  /* --------------------------------------------------------------------------------------------------------- */

  /* Tile1(LT) Plane 0, 1 */
  T1<<15|T1<<14|T1<<13|L1<<12|L1<<11|T1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|T0<<5|L0<<4|L0<<3|T0<<2|T0<<1|T0<<0,
  T1<<15|T1<<14|L1<<13|L1<<12|L1<<11|L1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|L0<<5|L0<<4|L0<<3|L0<<2|T0<<1|T0<<0,
  T1<<15|L1<<14|L1<<13|L1<<12|L1<<11|L1<<10|L1<<9|T1<<8 | T0<<7|L0<<6|L0<<5|L0<<4|L0<<3|L0<<2|L0<<1|T0<<0,
  L1<<15|L1<<14|T1<<13|L1<<12|L1<<11|T1<<10|L1<<9|L1<<8 | L0<<7|L0<<6|T0<<5|L0<<4|L0<<3|T0<<2|L0<<1|L0<<0,
  L1<<15|L1<<14|L1<<13|L1<<12|L1<<11|L1<<10|L1<<9|L1<<8 | L0<<7|L0<<6|L0<<5|L0<<4|L0<<3|L0<<2|L0<<1|L0<<0,
  T1<<15|T1<<14|L1<<13|T1<<12|T1<<11|L1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|L0<<5|T0<<4|T0<<3|L0<<2|T0<<1|T0<<0,
  T1<<15|L1<<14|T1<<13|L1<<12|L1<<11|T1<<10|L1<<9|T1<<8 | T0<<7|L0<<6|T0<<5|L0<<4|L0<<3|T0<<2|L0<<1|T0<<0,
  L1<<15|T1<<14|L1<<13|T1<<12|T1<<11|L1<<10|T1<<9|L1<<8 | L0<<7|T0<<6|L0<<5|T0<<4|T0<<3|L0<<2|T0<<1|L0<<0,
  /* Tile1(LT) Plane 2, 3 */
  T3<<15|T3<<14|T3<<13|L3<<12|L3<<11|T3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|T2<<5|L2<<4|L2<<3|T2<<2|T2<<1|T2<<0,
  T3<<15|T3<<14|L3<<13|L3<<12|L3<<11|L3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|L2<<5|L2<<4|L2<<3|L2<<2|T2<<1|T2<<0,
  T3<<15|L3<<14|L3<<13|L3<<12|L3<<11|L3<<10|L3<<9|T3<<8 | T2<<7|L2<<6|L2<<5|L2<<4|L2<<3|L2<<2|L2<<1|T2<<0,
  L3<<15|L3<<14|T3<<13|L3<<12|L3<<11|T3<<10|L3<<9|L3<<8 | L2<<7|L2<<6|T2<<5|L2<<4|L2<<3|T2<<2|L2<<1|L2<<0,
  L3<<15|L3<<14|L3<<13|L3<<12|L3<<11|L3<<10|L3<<9|L3<<8 | L2<<7|L2<<6|L2<<5|L2<<4|L2<<3|L2<<2|L2<<1|L2<<0,
  T3<<15|T3<<14|L3<<13|T3<<12|T3<<11|L3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|L2<<5|T2<<4|T2<<3|L2<<2|T2<<1|T2<<0,
  T3<<15|L3<<14|T3<<13|L3<<12|L3<<11|T3<<10|L3<<9|T3<<8 | T2<<7|L2<<6|T2<<5|L2<<4|L2<<3|T2<<2|L2<<1|T2<<0,
  L3<<15|T3<<14|L3<<13|T3<<12|T3<<11|L3<<10|T3<<9|L3<<8 | L2<<7|T2<<6|L2<<5|T2<<4|T2<<3|L2<<2|T2<<1|L2<<0,

  /* Tile1(RT) Plane 0, 1 */
  T1<<15|T1<<14|T1<<13|T1<<12|Y1<<11|Y1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|T0<<5|T0<<4|Y0<<3|Y0<<2|T0<<1|T0<<0,
  T1<<15|T1<<14|T1<<13|T1<<12|Y1<<11|T1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|T0<<5|T0<<4|Y0<<3|T0<<2|T0<<1|T0<<0,
  T1<<15|Y1<<14|Y1<<13|Y1<<12|Y1<<11|Y1<<10|Y1<<9|T1<<8 | T0<<7|Y0<<6|Y0<<5|Y0<<4|Y0<<3|Y0<<2|Y0<<1|T0<<0,
  Y1<<15|Y1<<14|Y1<<13|Y1<<12|Y1<<11|Y1<<10|Y1<<9|Y1<<8 | Y0<<7|Y0<<6|Y0<<5|Y0<<4|Y0<<3|Y0<<2|Y0<<1|Y0<<0,
  Y1<<15|T1<<14|Y1<<13|Y1<<12|Y1<<11|Y1<<10|Y1<<9|Y1<<8 | Y0<<7|T0<<6|Y0<<5|Y0<<4|Y0<<3|Y0<<2|Y0<<1|Y0<<0,
  Y1<<15|T1<<14|Y1<<13|Y1<<12|Y1<<11|Y1<<10|Y1<<9|Y1<<8 | Y0<<7|T0<<6|Y0<<5|Y0<<4|Y0<<3|Y0<<2|Y0<<1|Y0<<0,
  Y1<<15|Y1<<14|Y1<<13|Y1<<12|Y1<<11|Y1<<10|Y1<<9|Y1<<8 | Y0<<7|Y0<<6|Y0<<5|Y0<<4|Y0<<3|Y0<<2|Y0<<1|Y0<<0,
  T1<<15|Y1<<14|Y1<<13|Y1<<12|Y1<<11|Y1<<10|Y1<<9|T1<<8 | T0<<7|Y0<<6|Y0<<5|Y0<<4|Y0<<3|Y0<<2|Y0<<1|T0<<0,
  /* Tile1(RT) Plane 2, 3 */
  T3<<15|T3<<14|T3<<13|T3<<12|Y3<<11|Y3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|T2<<5|T2<<4|Y2<<3|Y2<<2|T2<<1|T2<<0,
  T3<<15|T3<<14|T3<<13|T3<<12|Y3<<11|T3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|T2<<5|T2<<4|Y2<<3|T2<<2|T2<<1|T2<<0,
  T3<<15|Y3<<14|Y3<<13|Y3<<12|Y3<<11|Y3<<10|Y3<<9|T3<<8 | T2<<7|Y2<<6|Y2<<5|Y2<<4|Y2<<3|Y2<<2|Y2<<1|T2<<0,
  Y3<<15|Y3<<14|Y3<<13|Y3<<12|Y3<<11|Y3<<10|Y3<<9|Y3<<8 | Y2<<7|Y2<<6|Y2<<5|Y2<<4|Y2<<3|Y2<<2|Y2<<1|Y2<<0,
  Y3<<15|T3<<14|Y3<<13|Y3<<12|Y3<<11|Y3<<10|Y3<<9|Y3<<8 | Y2<<7|T2<<6|Y2<<5|Y2<<4|Y2<<3|Y2<<2|Y2<<1|Y2<<0,
  Y3<<15|T3<<14|Y3<<13|Y3<<12|Y3<<11|Y3<<10|Y3<<9|Y3<<8 | Y2<<7|T2<<6|Y2<<5|Y2<<4|Y2<<3|Y2<<2|Y2<<1|Y2<<0,
  Y3<<15|Y3<<14|Y3<<13|Y3<<12|Y3<<11|Y3<<10|Y3<<9|Y3<<8 | Y2<<7|Y2<<6|Y2<<5|Y2<<4|Y2<<3|Y2<<2|Y2<<1|Y2<<0,
  T3<<15|Y3<<14|Y3<<13|Y3<<12|Y3<<11|Y3<<10|Y3<<9|T3<<8 | T2<<7|Y2<<6|Y2<<5|Y2<<4|Y2<<3|Y2<<2|Y2<<1|T2<<0,

  /* Tile1(LB) Plane 0, 1 */
  T1<<15|T1<<14|T1<<13|T1<<12|Y1<<11|Y1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|T0<<5|T0<<4|Y0<<3|Y0<<2|T0<<1|T0<<0,
  T1<<15|T1<<14|T1<<13|T1<<12|Y1<<11|T1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|T0<<5|T0<<4|Y0<<3|T0<<2|T0<<1|T0<<0,
  T1<<15|Y1<<14|Y1<<13|Y1<<12|Y1<<11|Y1<<10|Y1<<9|T1<<8 | T0<<7|Y0<<6|Y0<<5|Y0<<4|Y0<<3|Y0<<2|Y0<<1|T0<<0,
  Y1<<15|Y1<<14|Y1<<13|Y1<<12|Y1<<11|Y1<<10|Y1<<9|Y1<<8 | Y0<<7|Y0<<6|Y0<<5|Y0<<4|Y0<<3|Y0<<2|Y0<<1|Y0<<0,
  Y1<<15|T1<<14|Y1<<13|Y1<<12|Y1<<11|Y1<<10|Y1<<9|Y1<<8 | Y0<<7|T0<<6|Y0<<5|Y0<<4|Y0<<3|Y0<<2|Y0<<1|Y0<<0,
  Y1<<15|T1<<14|Y1<<13|Y1<<12|Y1<<11|Y1<<10|Y1<<9|Y1<<8 | Y0<<7|T0<<6|Y0<<5|Y0<<4|Y0<<3|Y0<<2|Y0<<1|Y0<<0,
  Y1<<15|Y1<<14|Y1<<13|Y1<<12|Y1<<11|Y1<<10|Y1<<9|Y1<<8 | Y0<<7|Y0<<6|Y0<<5|Y0<<4|Y0<<3|Y0<<2|Y0<<1|Y0<<0,
  T1<<15|Y1<<14|Y1<<13|Y1<<12|Y1<<11|Y1<<10|Y1<<9|T1<<8 | T0<<7|Y0<<6|Y0<<5|Y0<<4|Y0<<3|Y0<<2|Y0<<1|T0<<0,
  /* Tile1(LB) Plane 2, 3 */
  T3<<15|T3<<14|T3<<13|T3<<12|Y3<<11|Y3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|T2<<5|T2<<4|Y2<<3|Y2<<2|T2<<1|T2<<0,
  T3<<15|T3<<14|T3<<13|T3<<12|Y3<<11|T3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|T2<<5|T2<<4|Y2<<3|T2<<2|T2<<1|T2<<0,
  T3<<15|Y3<<14|Y3<<13|Y3<<12|Y3<<11|Y3<<10|Y3<<9|T3<<8 | T2<<7|Y2<<6|Y2<<5|Y2<<4|Y2<<3|Y2<<2|Y2<<1|T2<<0,
  Y3<<15|Y3<<14|Y3<<13|Y3<<12|Y3<<11|Y3<<10|Y3<<9|Y3<<8 | Y2<<7|Y2<<6|Y2<<5|Y2<<4|Y2<<3|Y2<<2|Y2<<1|Y2<<0,
  Y3<<15|T3<<14|Y3<<13|Y3<<12|Y3<<11|Y3<<10|Y3<<9|Y3<<8 | Y2<<7|T2<<6|Y2<<5|Y2<<4|Y2<<3|Y2<<2|Y2<<1|Y2<<0,
  Y3<<15|T3<<14|Y3<<13|Y3<<12|Y3<<11|Y3<<10|Y3<<9|Y3<<8 | Y2<<7|T2<<6|Y2<<5|Y2<<4|Y2<<3|Y2<<2|Y2<<1|Y2<<0,
  Y3<<15|Y3<<14|Y3<<13|Y3<<12|Y3<<11|Y3<<10|Y3<<9|Y3<<8 | Y2<<7|Y2<<6|Y2<<5|Y2<<4|Y2<<3|Y2<<2|Y2<<1|Y2<<0,
  T3<<15|Y3<<14|Y3<<13|Y3<<12|Y3<<11|Y3<<10|Y3<<9|T3<<8 | T2<<7|Y2<<6|Y2<<5|Y2<<4|Y2<<3|Y2<<2|Y2<<1|T2<<0,

  /* Tile1(RB) Plane 0, 1 */
  T1<<15|T1<<14|T1<<13|L1<<12|L1<<11|T1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|T0<<5|L0<<4|L0<<3|T0<<2|T0<<1|T0<<0,
  T1<<15|T1<<14|L1<<13|L1<<12|L1<<11|L1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|L0<<5|L0<<4|L0<<3|L0<<2|T0<<1|T0<<0,
  T1<<15|L1<<14|L1<<13|L1<<12|L1<<11|L1<<10|L1<<9|T1<<8 | T0<<7|L0<<6|L0<<5|L0<<4|L0<<3|L0<<2|L0<<1|T0<<0,
  L1<<15|L1<<14|T1<<13|L1<<12|L1<<11|T1<<10|L1<<9|L1<<8 | L0<<7|L0<<6|T0<<5|L0<<4|L0<<3|T0<<2|L0<<1|L0<<0,
  L1<<15|L1<<14|L1<<13|L1<<12|L1<<11|L1<<10|L1<<9|L1<<8 | L0<<7|L0<<6|L0<<5|L0<<4|L0<<3|L0<<2|L0<<1|L0<<0,
  T1<<15|T1<<14|L1<<13|T1<<12|T1<<11|L1<<10|T1<<9|T1<<8 | T0<<7|T0<<6|L0<<5|T0<<4|T0<<3|L0<<2|T0<<1|T0<<0,
  T1<<15|L1<<14|T1<<13|L1<<12|L1<<11|T1<<10|L1<<9|T1<<8 | T0<<7|L0<<6|T0<<5|L0<<4|L0<<3|T0<<2|L0<<1|T0<<0,
  L1<<15|T1<<14|L1<<13|T1<<12|T1<<11|L1<<10|T1<<9|L1<<8 | L0<<7|T0<<6|L0<<5|T0<<4|T0<<3|L0<<2|T0<<1|L0<<0,
  /* Tile1(RB) Plane 2, 3 */
  T3<<15|T3<<14|T3<<13|L3<<12|L3<<11|T3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|T2<<5|L2<<4|L2<<3|T2<<2|T2<<1|T2<<0,
  T3<<15|T3<<14|L3<<13|L3<<12|L3<<11|L3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|L2<<5|L2<<4|L2<<3|L2<<2|T2<<1|T2<<0,
  T3<<15|L3<<14|L3<<13|L3<<12|L3<<11|L3<<10|L3<<9|T3<<8 | T2<<7|L2<<6|L2<<5|L2<<4|L2<<3|L2<<2|L2<<1|T2<<0,
  L3<<15|L3<<14|T3<<13|L3<<12|L3<<11|T3<<10|L3<<9|L3<<8 | L2<<7|L2<<6|T2<<5|L2<<4|L2<<3|T2<<2|L2<<1|L2<<0,
  L3<<15|L3<<14|L3<<13|L3<<12|L3<<11|L3<<10|L3<<9|L3<<8 | L2<<7|L2<<6|L2<<5|L2<<4|L2<<3|L2<<2|L2<<1|L2<<0,
  T3<<15|T3<<14|L3<<13|T3<<12|T3<<11|L3<<10|T3<<9|T3<<8 | T2<<7|T2<<6|L2<<5|T2<<4|T2<<3|L2<<2|T2<<1|T2<<0,
  T3<<15|L3<<14|T3<<13|L3<<12|L3<<11|T3<<10|L3<<9|T3<<8 | T2<<7|L2<<6|T2<<5|L2<<4|L2<<3|T2<<2|L2<<1|T2<<0,
  L3<<15|T3<<14|L3<<13|T3<<12|T3<<11|L3<<10|T3<<9|L3<<8 | L2<<7|T2<<6|L2<<5|T2<<4|T2<<3|L2<<2|T2<<1|L2<<0
};

/*
タイルで使用するパレット 
  タイル数分だけ用意する
  4 ビットシフトする必要がある
*/
const u8 PatternPallets[] = {
	0<<4,
  /* ---------------*/
  0<<4
};

/*
16 x 28 tiles (256 x 448 pixel)
*/
const u8 Map[] = {
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
  0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

#define MAP_TILE_W 16 /* 256 pixel */
#define MAP_TILE_H 31 /* 496 pixel */

#define SCR_TILE_W (SCREEN_WIDTH>>4)
#define SCR_TILE_H (SCREEN_HEIGHT>>4)

  /* Free memory range is [0x800, 0xfff] and [0x5000, 0x7eff], here use latter */
#define TILE_VRAM 0x5000

main()
{  
  int x, y;
  int MapX, MapY;
  u8 KeyState;
  x = y = 0;
  MapX = MapY = 0;

  /* 16 x 28 tiles (256 x 448 pixel) */
  set_map_data(Map, MAP_TILE_W, MAP_TILE_H);  
  set_tile_data(Pattern, 2, PatternPallets);

  /* Free memory range is [0x800, 0xfff] and [0x5000, 0x7eff], here use latter */
  load_tile(TILE_VRAM);
  /* Palette [0, 15] */
  load_palette(0, Palette, 1);
  /*
  第1, 2引数 : 描画先 LT 座標(16ピクセルで1) 
  第3, 4引数 : 読み取るマップデータの LT 座標(16ピクセルで1)
  第5, 6引数 : マップの幅と高さ (ちょうど画面に映る分だけ読み込んでいる)
  */
	load_map(x >> 4, y >> 4, MapX, MapY, SCR_TILE_W, SCR_TILE_H);

  while(1) {
    KeyState = joy(JOY_PAD0);    
    if(KeyState & JOY_UP) { y = (y - 1) & 0xff; }

    if(!(y & 15)) {
      --MapY;
      if(MapY < 0) { MapY += MAP_TILE_H; }
      load_map(x >> 4, (y >> 4) - 1, MapX, MapY - 1, SCR_TILE_W, 1);
    }

    scroll(WIN0, x, y, 0, SCREEN_HEIGHT - 1, /*SCR_SPR_ON |*/SCR_BG_ON);
    
    vsync();
  }
}