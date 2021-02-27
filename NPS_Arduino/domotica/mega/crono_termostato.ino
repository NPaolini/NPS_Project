#include "pid.h"
#include "crono_termostato.h"
//----------------------------------
#define HOUR_ITEMS 24
#define TOT_ITEMS (HOUR_ITEMS * 2 + 1)
#define DIM_ITEMS (TOT_ITEMS + 1)
//----------------------------------
#define CT_ON LOW
#define CT_OFF HIGH
//----------------------------------
#define LEV(s) (LOW == (s) ? CT_OFF : CT_ON)
//----------------------------------
extern float getCurrTemper();
extern int16_t getIxTime(); // nel formato hh*100 + mm
//----------------------------------
// per fare una media delle temperature
#define MAX_COUNT 10
//----------------------------------
crono_termostato::crono_termostato(pin_t pin, unsigned long delayStep) : pinOut(pin), DelayStep(delayStep), lastTime(0),
   pinStatus(0)
{
  memset(items, 0, sizeof(items));
  G_pinMode(pinOut, OUTPUT);
}
//----------------------------------
void crono_termostato::setItems(const byte* buff, byte num, byte offs)
{
  if(num + offs > sizeof(items))
    return;
  memcpy(items + offs, buff, num * sizeof(*buff));
  lastTime = 0;
}
//----------------------------------
void crono_termostato::getItems(byte* buff, byte num, byte offs)
{
  if(num + offs > sizeof(items))
    return;
  memcpy(buff, items + offs, num * sizeof(*buff));
}
//----------------------------------
byte crono_termostato::getIx(bool forceAuto)
{
  byte action = items[0];
  byte ix = 1;
  if(forceAuto || !(action & 2)) { // automatico
    int16_t tm = getIxTime();
    byte h = tm / 100;
    byte m = tm % 100;
    ix = h * 2 + m / 30 + (forceAuto ? 0 : 2);
    }
  return ix;
}
//----------------------------------
void crono_termostato::begin()
{
  byte ix = getIx(false);
  pid.set_setPoint(items[ix]);
}
//----------------------------------
bool crono_termostato::Loop()
{
  byte action = items[0];
  if(!(action & 1))
    return false;
  unsigned long t = millis();
  if(t - lastTime < DelayStep)
    return false;
  byte ix = getIx(false);
  float tmp = items[ix];
  tmp /= 4;
  pid.set_setPoint(tmp);
  byte old = pinStatus;
  pinStatus = LOW;
  lastTime = t;
  float currTemper = getCurrTemper();
  float curr = pid.getResult(currTemper);
  if(curr >= 0)
    pinStatus = HIGH;
  if(pinStatus != old)
    G_writeOut(pinOut, LEV(pinStatus));
  if(useDebug & D_BIT_CRONO) {
    Serial.print(F("Stato crono-termostato = temperatura attuale: "));
    Serial.print(currTemper);
    Serial.print(F(" - temperatura impostata: "));

    Serial.print(tmp);
    if(ix >= 2) {
      Serial.print(F(" - indice: "));
      Serial.print(ix - 2);
      byte h = (ix - 2) / 2;
      const char* m = ((ix - 2) & 1) ? ".30" : ".00";
      Serial.print(F(" - orario: "));
      Serial.print(h);
      Serial.print(m);
      }
    else
      Serial.print(F(" - manuale"));
    Serial.print(F(" - stato: "));
    Serial.println(pinStatus);
    }
  return true;
};
//----------------------------------
extern void convert4Output(int8_t val, char* buff);
//----------------------------------
void crono_termostato::getImpTemp(char* buff)
{
  byte ix = getIx(false);
  convert4Output(items[ix], buff);
  buff[strlen(buff) - 1] = 0;
}
//----------------------------------
void crono_termostato::fillImpData(char* buff, int len)
{
  static int8_t oldIx = -1;
  int8_t ix = getIx(true);
  if(ix != oldIx) {
    oldIx = ix;
    needUpdateCrono = true;
    }
  char t[20];
  float f = items[ix + 2];
  f /= 4;
  dtostrf (f, 2, 2, t);
  char t2[20];
  dtostrf (getCurrTemper(), 2, 2, t2);
  sprintf(buff, "3|i|%d|%s|%s|", ix, t, t2);
}
//----------------------------------
void crono_termostato::setValue(byte ix, byte val)
{
  if(ix < SIZE_A(items))
    items[ix] = val;
}
//----------------------------------
void crono_termostato::toggleOnOff()
{
  items[0] ^= 1;
  if(!(items[0] & 1)) {
    pinStatus = LOW;
    G_writeOut(pinOut, LEV(pinStatus));
    }
  else
    forceRefresh();
}
//----------------------------------
void crono_termostato::toggleAuto()
{
  items[0] ^= 2;
  forceRefresh();
}
//----------------------------------
