//------ man_tft.ino --------------------
#include "man_tft.h"
//---------------------------------------
//---------------------------------------
void tft_print_common(uint8_t size, uint16_t color, int16_t x, int16_t y)
{
  tft.setCursor(x, y);
  tft.setTextColor(color, bkgColorText);
  tft.setTextSize(size);
}
//---------------------------------------
void tft_print_common(uint8_t size, uint16_t color)
{
  tft.setTextColor(color, bkgColorText);
  tft.setTextSize(size);
}
//---------------------------------------
