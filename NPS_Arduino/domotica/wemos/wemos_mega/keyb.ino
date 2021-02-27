#include "keyb.h"
#include "domotica.h"
//---------------------------------
void keyb::begin()
{
  for(byte i = 0; i < MAX_PIN_KEYB_COL; ++i) {
    G_pinMode(pinWrite[i], OUTPUT);
    G_writeOut(pinWrite[i], HIGH);
    }
  for(byte i = 0; i < MAX_PIN_KEYB_ROW; ++i) {
    G_pinMode(pinRead[i], INPUT_PULLUP);
    }
}
//---------------------------------
int8_t keyb::getStatus()
{
#ifdef TKR_KEYB_12
  for(byte i = 0; i < MIN_SQUARE; ++i) {
    G_writeOut(pinWrite[i], LOW);
    for(byte j = 0; j < MIN_SQUARE; ++j) {
      bool stat = G_getInput2(pinRead[j]);
      if(!stat) {
        G_writeOut(pinWrite[i], HIGH);
        return i * 4 + j;
        }
      }
    G_writeOut(pinWrite[i], HIGH);
    }
  G_writeOut(pinWrite[MIN_SQUARE], LOW);
  pin_t t[] = { pinRead[MIN_SQUARE], pinRead[MIN_SQUARE - 2], pinRead[MIN_SQUARE + 1] };
  for(byte i = 0; i < SIZE_A(t); ++i) {
    bool stat = G_getInput2(t[i]);
    if(!stat) {
      G_writeOut(pinWrite[MIN_SQUARE], HIGH);
      return MIN_SQUARE * 4 + i;
      }
    }
  G_writeOut(pinWrite[MIN_SQUARE], HIGH);
  return -1;
#else
  for(byte i = 0; i < MAX_PIN_KEYB_COL; ++i) {
    G_writeOut(pinWrite[i], LOW);
    for(byte j = 0; j < MAX_PIN_KEYB_ROW; ++j) {
      bool stat = G_getInput2(pinRead[j]);
      if(!stat) {
        G_writeOut(pinWrite[i], HIGH);
        return i * 4 + j;
        }
      }
    G_writeOut(pinWrite[i], HIGH);
    }
  return -1;
#endif
}
//---------------------------------
// vale sia per la 4x4 che per la 4x3
static const char keyChar[] = "123A456B789C*0#D";
//---------------------------------
char keyb::codeToChar(int8_t v)
{
  if((uint8_t)v >= sizeof(keyChar))
    return '?';
  return keyChar[v];
}
//---------------------------------
int8_t keyb::charToCode(char v)
{
  for(uint8_t i = 0; i < sizeof(keyChar); ++i)
    if(v == keyChar[i])
      return i;
  return -1;
}
//---------------------------------
