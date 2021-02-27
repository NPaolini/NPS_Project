#include "motor.h"
//--------------------------
#define WEB_BTN(a) ((a) & 1)
#define LOC_BTN(a) (((a) >> 1) & 1)
#define END_RUN(a) ((((a) & motor::ems::ems_onEndrun) != 0) ? 2 : 0)
//--------------------------
void motor::begin()
{
  G_pinMode(pinRunMot, OUTPUT);
  G_pinMode(pinCheckMot, INPUT_PULLUP);

  G_writeOut(pinRunMot, e_OFF);

  reset(false);
}
//--------------------------
bool motor::run(bool on)
{
  // se richiesta avvio ed il pin è già attivo non esegue nulla
  if(on && getTrueHardStatus())
    return false;
  bool old = softRun;
  if(!on) {
    G_writeOut(pinRunMot, e_OFF);
    reset(false);
    }
  softRun = on;
  return old;
} // bySoft
//--------------------------
bool motor::getTrueHardStatus()
{
  bool prev = hardRun ? e_ON : e_OFF;
  return hardRun = (G_getInput(pinCheckMot, prev) == e_ON);  
}
//--------------------------
bool motor::getHardStatus()
{
  // se è stato attivato da web, si considera che il pulsante non è premuto
  // essendo sullo stesso filo non è possibile distinguere l'avvio da web o da pulsante
  if(getSoftStatus())
    return false;
  return getTrueHardStatus();
}
//--------------------------
void motor::reset(bool endrun)
{
  softRun = hardRun = false;
  stat = endrun ? ems_onEndrun: ems_undef; lastTime = 0;
  offsTime = 0;
  if(endrun)
    pos = 255;
}
//--------------------------
void motor::abort()
{
  if(ems_dejaWaiting & stat)
    G_writeOut(pinRunMot, e_OFF);
  reset(false);
}
//--------------------------
byte motor::getEndRun()
{
  return END_RUN(getStatus());
}
//--------------------------
bool motor::action()
{
  bool active = getTrueHardStatus() | softRun;
  // e' attivo pulsante fisico o da web
  if(active) {
    // aveva gia' raggiunto il fine corsa
    if(stat & ems_onEndrun) {
      reset(true);
      return false;
      }
    // prima entrata attiva
    if(!(stat & ems_dejaWaiting)) { // && !lastTime
      stat = ems_dejaWaiting;
      lastTime = millis();
      offsTime = (unsigned long)pos * 1000 * TIME_TO_STOP_MOTOR / 255;
      if(softRun)
        G_writeOut(pinRunMot, e_ON);
      return true;
      }
    }

  // se si trova in movimento
  if(ems_dejaWaiting & stat) {
    bool end = !active;
    unsigned long t = millis() - lastTime + offsTime;
    if(t >= TIME_TO_STOP_MOTOR * 1000)
      end = 1;
    if(end) {
      G_writeOut(pinRunMot, e_OFF);
      // se era attivo vuol dire che ha raggiunto il timeout
      // altrimenti il movimento e' stato fermato da pulsante e quindi lo stato
      // risulta sconosciuto
      reset(active);
      return true;
      }
    else {
      t *= 255;
      t /= TIME_TO_STOP_MOTOR;
      t /= 1000;
      if(t > 255)
         t = 255;
      pos = t;
      }
    }
  return false;
}
//--------------------------
//--------------------------
void roll_up_shutter::begin()
{
  for(int i = 0; i < 2; ++i)
    Mot[i]->begin();
}
//--------------------------
void roll_up_shutter::runByWeb(bool down, bool set)
{
  if(set)
    Mot[!down]->run(false);
  Mot[down]->run(set);
  loop();
}
//--------------------------
void roll_up_shutter::savePos(int ix)
{
  byte v = Mot[0]->getPos();
  if(v) 
    ISetup.rusPos.bits &= ~(1 << ix);
  else {
    v = Mot[1]->getPos();
    ISetup.rusPos.bits |= 1 << ix;
    }
  ISetup.rusPos.pos[ix] = v;
}
//--------------------------
void roll_up_shutter::verifyPos(byte ixOn)
{
  if(lastRun == ixOn)
    return;
  byte pos = Mot[!ixOn]->getPos();
  Mot[!ixOn]->setPos(0);
  if(lastRun == 255) {
    if(Mot[ixOn]->getPos()) {
      lastRun = ixOn;
      return;
      }
    }
  lastRun = ixOn;
  Mot[ixOn]->setPos(255 - pos);
}
//--------------------------
bool roll_up_shutter::performAct(byte ixOn)
{
  verifyPos(ixOn);
  Mot[!ixOn]->abort();
  return Mot[ixOn]->action();
}
//--------------------------
bool roll_up_shutter::loop()
{
  bool on1 = Mot[0]->getHardStatus();
  bool on2 = Mot[1]->getHardStatus();
  // sono premuti entrambi i pulsanti ... nessuna azione
  if(on1 && on2)
    return false;
  // se il primo ha il pulsante fisico premuto
  if(on1) {
    return performAct(0);
    }
  else if(on2) {
    return performAct(1);
    }
  // se il primo ha il pulsante web premuto
  if(Mot[0]->getSoftStatus()) {
    return performAct(0);
    }
  else if(Mot[1]->getSoftStatus()) {
    return performAct(1);
    }
  on1 = Mot[0]->action();
  on2 = Mot[1]->action();
  return on1 || on2;
}
//--------------------------
static char* addVal(char* buff, int val)
{
  char frmt[4];
  frmt[0] = '|';
  frmt[1] = '%';
  frmt[2] = 'd';
  frmt[3] = 0;
  int wr = sprintf(buff, frmt, val);
  if(wr > 0)
    return buff + wr;
  return buff;   
}
//--------------------------
char* roll_up_shutter::addValMot(char* buff, byte ix, motor* m)
{
  char* p = buff;
  p = addVal(p, ix);
  p = addVal(p, m->getTrueHardStatus());
  p = addVal(p, m->getSoftStatus());
  p = addVal(p, m->getEndRun());
  p = addVal(p, m->getPos());
  return p;
}  
//--------------------------
void roll_up_shutter::fillForWeb(byte ix, char* buff, int len)
{
  char* p = addValMot(buff, ix, Mot[0]);
  addValMot(p, ix + 1, Mot[1]);
}
//---------------------------------
