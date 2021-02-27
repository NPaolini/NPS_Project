#include "mplex.h"
#include "domotica.h"
//---------------------------------
mplex::mplex(pin_t pinSig, pin_t* pinSel) :
    pinSig(pinSig), useShReg(false)
{
  for(byte i = 0; i < MAX_MPX_SEL; ++i) {
    PinSel[i] = pinSel[i];
    if(pinSel[i] >= OFFS_SR_1)
      useShReg = true;
    }
}
//---------------------------------
void mplex::begin()
{
  for(byte i = 0; i < MAX_MPX_SEL; ++i) {
    G_pinMode(PinSel[i], OUTPUT);
    }
  pinMode(pinSig, INPUT_PULLUP);
}
//---------------------------------
void mplex::prepare(pin_t pin)
{
  for(byte i = 0; i < MAX_MPX_SEL; ++i) {
    G_prepareWriteOut(PinSel[i], pin & 1);
    pin >>= 1;
    }
  if(useShReg)
    G_finalizeWriteOut();
}
//---------------------------------
bool mplex::getStatus(pin_t pin)
{
  prepare(pin);
  bool result = digitalRead(pinSig);
  return result;
}
//---------------------------------
void manageMPlex::begin()
{
  for(int8_t i = Num - 1; i >= 0; --i) {
    if(EnaPin[i]) {
      G_pinMode(EnaPin[i], OUTPUT);
      G_writeOut(EnaPin[i], true);
      if(EnaPin[i] >= OFFS_SR_1)
        useShReg = true;
      }
    }
  oldEnabled = 255;
  MPlex1.begin();
  MPlex2.begin();
}
//---------------------------------
void manageMPlex::enable(byte ix)
{
  if(!EnaPin[ix] || oldEnabled == ix)
    return;
  oldEnabled = ix;
  for(int8_t i = Num - 1; i >= 0; --i)
    if(i != ix)
      G_prepareWriteOut(EnaPin[i], HIGH);
  G_prepareWriteOut(EnaPin[ix], LOW);
  if(useShReg)
    G_finalizeWriteOut();
}
//---------------------------------
bool manageMPlex::digRead(pin_t pin)
{
  if(OFFS_MPX_1 > pin)
    return digitalRead(pin);
  else if(OFFS_MPX_2 > pin) {
    enable(0);
    return MPlex1.getStatus(REM_MPX_1(pin));
    }
  else if(OFFS_MPX_3 > pin) {
    enable(1);
    return MPlex2.getStatus(REM_MPX_2(pin));
    }
  return false;
}
//---------------------------------
//---------------------------------
sh_reg::sh_reg(pin_t clockPin, pin_t latchPin, pin_t dataPin) :
  ClockPin(clockPin), LatchPin(latchPin), DataPin(dataPin)
{
  // i rele generalmente hanno lo stato di riposo a +5v
  for(int i = 0; i < SR_CASCADE; ++i)
    Values[i] = (uint8_t)-1;
}
//---------------------------------
void sh_reg::begin()
{
  for(int i = 0; i < SR_CASCADE; ++i)
    Values[i] = (uint8_t)-1;
}
//---------------------------------
void sh_reg::setValues()
{
  pin_t old = lastPinSPI;
  if(PIN_SHIFT_REG_LATCH != old)
    enableSHIFT_REG();
  else
    digitalWrite(LatchPin, LOW);
  for(int i = SR_CASCADE - 1; i >= 0; --i)
    SPI.transfer(Values[i]);

  if(PIN_SHIFT_REG_LATCH != old)
    enableGen(old);
  else
    digitalWrite(LatchPin, HIGH);
}
//---------------------------------
void sh_reg::setPinValue(pin_t pin, bool set)
{
  if(preparePinValue(pin, set))
    setValues();
}
//---------------------------------
bool sh_reg::preparePinValue(pin_t pin, bool set)
{
#if 1
  pin_t ix = pin >> 3;
  pin &= 7;
#else
  pin_t ix = pin / 8;
  pin %= 8;
#endif
  if(ix >= SR_CASCADE)
    return false;
  Values[ix] &= ~(1 << pin);
  Values[ix] |= set << pin;
  return true;
}
//---------------------------------
void sh_reg::finalizeValue()
{
  setValues();
}
//---------------------------------
