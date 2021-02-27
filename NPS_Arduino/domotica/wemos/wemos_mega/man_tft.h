//------ man_tft.h ---------------------------
//--------------------------------------------
#ifndef man_tft_H_
#define man_tft_H_
//--------------------------------------------
#define bkgColorText ST77XX_BLACK
//---------------------------------------
void tft_print_common(uint8_t size, uint16_t color, int16_t x, int16_t y);
void tft_print_common(uint8_t size, uint16_t color);
//---------------------------------------
template<typename T>void tft_print(T txt, uint8_t size, uint16_t color, int16_t x, int16_t y)
{
  tft_print_common(size, color, x, y);
  tft.print(txt);
}
//---------------------------------------
template<typename T>void tft_print(T txt, uint8_t size, uint16_t color)
{
  tft_print_common(size, color);
  tft.print(txt);
}
//---------------------------------------
template<typename T>void tft_print(T txt, uint16_t color)
{
  tft.setTextColor(color, bkgColorText);
  tft.print(txt);
}
//---------------------------------------
template<typename T>void tft_print(T txt, uint8_t size)
{
  tft.setTextSize(size);
  tft.print(txt);
}
//---------------------------------------
template<typename T>void tft_print(T txt)
{
  tft.print(txt);
}
//---------------------------------------
template<typename T>void tft_println(T txt, uint8_t size, uint16_t color, int16_t x, int16_t y)
{
  tft_print_common(size, color, x, y);
  tft.println(txt);
}
//---------------------------------------
template<typename T>void tft_println(T txt, uint8_t size, uint16_t color)
{
  tft_print_common(size, color);
  tft.println(txt);
}
//---------------------------------------
template<typename T>void tft_println(T txt, uint16_t color)
{
  tft.setTextColor(color, bkgColorText);
  tft.println(txt);
}
//---------------------------------------
template<typename T>void tft_println(T txt, uint8_t size)
{
  tft.setTextSize(size);
  tft.println(txt);
}
//---------------------------------------
template<typename T>void tft_println(T txt)
{
  tft.println(txt);
}
//---------------------------------------
#endif
