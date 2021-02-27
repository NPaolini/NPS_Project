#include "lighting.h"
//---------------------------------
#define ON_LIGHT LOW
#define OFF_LIGHT HIGH
//---------------------------------
#define BIT_SOFT 7
//---------------------------------
lighting::lighting(pin_t pinOut, pin_t* pinInput, byte num) :
    PinOut(pinOut), PinInput(pinInput), Num(num), status(0)
{
}
//---------------------------------
void lighting::begin()
{
  for(byte i = 0; i < Num; ++i) {
    G_pinMode(PinInput[i], INPUT);
    }
  G_pinMode(PinOut, OUTPUT);
  G_writeOut(PinOut, OFF_LIGHT);
}
//---------------------------------
void lighting::loop()
{
  byte old = status;
  // ricarica il bit software
  status = old & (1 << BIT_SOFT);
  bool active = status;
  for(byte i = 0; i < Num; ++i) {
    bool t = G_getInput(PinInput[i], old & (1 << i));
    active ^= t;
    status |= t << i;
    }
  G_writeOut(PinOut, active ? ON_LIGHT : OFF_LIGHT);
}
//---------------------------------
void lighting::toggleSoft()
{
  status ^= 1 << BIT_SOFT;
}
//---------------------------------
bool lighting::getSoftStatus()
{
  return status & (1 << BIT_SOFT);
}
//---------------------------------
bool lighting::getStatus()
{
  bool curr = 0;
  for(byte i = 0; i < Num; ++i)
    curr ^= (status >> i) & 1;
  return curr ^ getSoftStatus();
}
