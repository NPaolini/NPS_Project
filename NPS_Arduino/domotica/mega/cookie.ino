//-------- cookie.ino --------------
#include "cookie.h"
//----------------------------------
//#include "domotica.h"
//----------------------------------
/*
  se nella get si richiede un file html (per adesso solo index), si verifica che ci sia
  il cookie. Se c'è ed è valido si continua con la richiesta, altrimenti si richiama la pagina di login.
  Si verifica che user e psw siano validi, si genera un nuovo cookie e si riparte con l'index.
*/
//----------------------------------
bool manCookie::isValid(const char* c)
{
  if(!*c)
    return false;
  for(uint8_t i = 0; i < SIZE_A(Cookie); ++i) {
    if(!*Cookie[i].c)
      continue;
    if(!strncmp(c, Cookie[i].c, DIM_COOKIE)) {
      Cookie[i].startTime = millis();
      return true;
      }
    }
  return false;
}
//----------------------------------
#define TIME_TO_EXPIRE (1000L * 60)
//----------------------------------
void manCookie::loop()
{
  static unsigned long old;
  unsigned long curr = millis();
  if(curr - old < (1000L * 10))
    return;
  old = curr;
  for(uint8_t i = 0; i < SIZE_A(Cookie); ++i) {
    if(Cookie[i].c[0] && Cookie[i].startTime + TIME_TO_EXPIRE < curr) {
      Cookie[i].c[0] = 0;
      Cookie[i].startTime = 0;
      }
    }
}
//----------------------------------
void manCookie::get(char* c, uint8_t pos)
{
  *c = 0;
  if(pos < SIZE_A(Cookie))
    strcpy(c, Cookie[pos].c);
}
//----------------------------------
int8_t manCookie::getUser(char* c)
{
  for(uint8_t i = 0; i < SIZE_A(Cookie); ++i)
    if(!strncmp(c, Cookie[i].c, DIM_COOKIE))
      return Cookie[i].idUser;
  return -1;
}
//----------------------------------
uint8_t manCookie::add(const char* c, int8_t idUser)
{
  unsigned long curr = (unsigned long)-1;
  uint8_t pos = 0;
  for(uint8_t i = 0; i < SIZE_A(Cookie); ++i) {
    if(!Cookie[i].c[0]) {
      strcpy(Cookie[i].c, c);
      Cookie[i].startTime = millis();
      Cookie[i].idUser = idUser;
      return i;
      }
    if(curr > Cookie[i].startTime) {
      curr = Cookie[i].startTime;
      pos = i;
      }
    }
  strcpy(Cookie[pos].c, c);
  Cookie[pos].startTime = millis();
  Cookie[pos].idUser = idUser;
  return pos;
}
//----------------------------------
void manCookie::rem(const char* c)
{
  for(uint8_t i = 0; i < SIZE_A(Cookie); ++i) {
    if(!strncmp(c, Cookie[i].c, DIM_COOKIE)) {
      Cookie[i].c[0] = 0;
      Cookie[i].startTime = 0;
      }
    }
}
//----------------------------------
uint8_t manCookie::make(int8_t idUser)
{
  unsigned long t = random(0x7fffffff);
  char* p = g_Buff;
  for(uint8_t i = 0; i < DIM_COOKIE; ++i) {
    *p++ = '0' + t % 10;
    t /= 10;
    }
  *p = 0;
  return add(g_Buff, idUser);
}
//--------------------------------------------
