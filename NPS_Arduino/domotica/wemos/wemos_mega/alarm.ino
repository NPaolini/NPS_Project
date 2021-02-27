#include "alarm.h"
//---------------------------------
alarmItem::alarmItem(pin_t pinInput) : PinInput(pinInput), Excluted(false), active(false) {}
//---------------------------------
void alarmItem::begin()
{
  G_pinMode(PinInput, INPUT);
}
//---------------------------------
bool alarmItem::getInput(bool force)
{
  if(!force && isExcluted())
    return false;

  active = G_getInput(PinInput, active);
  return active;
}
//---------------------------------
baseAlarm::baseAlarm(pin_t pinLedOn, pin_t pinOut) :
  PinLedOn(pinLedOn), PinOut(pinOut), totItem(0),
  tmWait(0), tmLast(0), status(0), LedRGB(pinLedOn)
{
  for(uint8_t i = 0; i < SIZE_A(Items); ++i)
    Items[i] = 0;
}
//---------------------------------
void ledRGB::setPin(pin_t pin)
{
  for(uint8_t i = 0; i < 3; ++i)
    G_prepareWriteOut(FirstPin + i, true);
  if(pin)
    G_prepareWriteOut(FirstPin + pin - 1, false);
  G_finalizeWriteOut();
}
//---------------------------------
void ledRGB::begin()
{
  for(uint8_t i = 0; i < 3; ++i)
    G_pinMode(FirstPin + i, OUTPUT);
  setPin(inactive);
}
//---------------------------------
baseAlarm::~baseAlarm()
{
  for(uint8_t i = 0; i < totItem; ++i)
    delete Items[i];
}
//---------------------------------
void baseAlarm::begin(pin_t* pinInput, uint8_t num)
{
  if(num > SIZE_A(Items))
    num = SIZE_A(Items);
  totItem = num;
  for(uint8_t i = 0; i < totItem; ++i)
    Items[i] = makeItem(i, pinInput[i]);

  LedRGB.begin();
  G_pinMode(PinOut, OUTPUT);
  G_writeOut(PinOut, HIGH);

  infoSetup* isetup = getISetup();
  for(uint8_t i = 0; i < totItem; ++i) {
    Items[i]->begin();
    bool set = isSetBit(isetup->alarmBit, i + 1);
    Items[i]->setExcluted(set);
    }
  bool set = isSetBit(isetup->alarmBit, 0);
  enable(set);
}
//---------------------------------
uint32_t setBit(uint32_t& v, uint32_t bit, bool set)
{
  v &= ~bit;
  if(set)
    v |= bit;
  return v;
}
//---------------------------------
/*
  sequenza:
    arriva l'allarme, suona la sirena per t1 minuti poi ignora l'allarme per t2 minuti
    se nel tempo precedente arriva l'ack, spegna la sirena (se accesa) ed ignora l'allarme per t3 minuti
*/
//---------------------------------
#define TIME2WAIT_WAIT_1  (1000 * (unsigned long)ISetup.secAlarmTiming1)
#define TIME2WAIT_WAIT_2  (1000 * (unsigned long)ISetup.secAlarmTiming2)
#define TIME2WAIT_ACK     (1000 * (unsigned long)ISetup.secAlarmTiming3)
//---------------------------------
void baseAlarm::loop()
{
  if(!isSet(status, a_on)) {
    return;
    }
  if(isSet(status, a_active | a_ack | a_wait)) {
    unsigned long t = millis();
    // scaduto il tempo si spegne l'uscita
    if(t - tmLast > tmWait) {
      DEBUG_ALARM(Serial.print(" - timeout"))
      G_writeOut(PinOut, HIGH);
      // se era il primo time
      if(!isSet(status, a_wait)) {
        DEBUG_ALARM(Serial.print(" - onWait - "))
        DEBUG_ALARM(Serial.println(rtc.getTimeStr()))

        setBit(status, a_wait, true);
        tmWait = TIME2WAIT_WAIT_2;
        tmLast = millis();
        LedRGB.setPin(ledRGB::onWait);
        }
      // è il secondo, ripulisce
      else {
        setBit(status, a_active | a_ack | a_wait, false);
        DEBUG_ALARM(Serial.print(" - clear:"))
        DEBUG_ALARM(Serial.print(status, BIN))
        DEBUG_ALARM(Serial.print(" - "))
        DEBUG_ALARM(Serial.println(rtc.getTimeStr()))
        LedRGB.setPin(ledRGB::active);
        }
      }
    }
  bool curr = false;
  for(uint8_t i = 0; i < totItem; ++i) {
    curr |= Items[i]->getInput(false);
    }
  bool chg = curr ^ isSet(status, a_active);
  if(chg && curr) {
    setBit(status, a_active, curr);
    DEBUG_ALARM(Serial.print(" - changed:"))
    DEBUG_ALARM(Serial.print(status, BIN))
    DEBUG_ALARM(Serial.print(" "))
    if(!isSet(status, a_ack | a_wait)) {
      DEBUG_1(Serial.print(" - now ON "))
      G_writeOut(PinOut, LOW);
      LedRGB.setPin(ledRGB::onAlarm);
      tmWait = TIME2WAIT_WAIT_1;
      tmLast = millis();
      }
    DEBUG_ALARM(Serial.println(rtc.getTimeStr()))
    }
}
//---------------------------------
bool baseAlarm::getItemStatus(uint8_t ix, bool force)
{
  if(ix >= totItem)
    return false;
  return Items[ix]->getInput(force);
}
//---------------------------------
byte baseAlarm::getStatus()
{
  return status;
}
//---------------------------------
// allarme riconosciuto, si ferma l'output e si avvia il time prima della verifica/notifica successiva
bool baseAlarm::acknoledge()
{
  // disabilitato
  if(!(status & a_on))
    return false;
  // non in stato di allarme
  if(!isSet(status, a_active))
    return false;
  bool chg = !isSet(status, a_ack);
  setBit(status, a_ack, true);
  // se non è stato già riconosciuto
  if(chg) {
    setBit(status, a_wait, true);
    tmWait = TIME2WAIT_ACK;
    tmLast = millis();
    G_writeOut(PinOut, HIGH);
    LedRGB.setPin(ledRGB::onWait);
    return true;
    }
  return false;
}
//---------------------------------
void baseAlarm::enable(bool set)
{
  bool chg = set ^ isSet(status, a_on);
  if(chg) {
    if(!set) {
      G_writeOut(PinOut, HIGH);
      LedRGB.setPin(ledRGB::inactive);
      status = 0;
      }
    else {
      setBit(status, a_on, true);
      LedRGB.setPin(ledRGB::active);
      }
    infoSetup* isetup = getISetup();
    setBitBit(isetup->alarmBit, 0, set);
    saveSetup();
    }
}
//---------------------------------
bool baseAlarm::isEnabled()
{
  return isSet(status, a_on);
}
//---------------------------------
void baseAlarm::exclude(uint8_t ix, bool set)
{
  if(ix >= totItem)
    return;
  Items[ix]->setExcluted(set);
  infoSetup* isetup = getISetup();
  setBitBit(isetup->alarmBit, ix + 1, set);
  saveSetup();
}
//---------------------------------
bool baseAlarm::isExcluted(uint8_t ix)
{
  if(ix >= totItem)
    return true;
  return Items[ix]->isExcluted();
}
//---------------------------------
//---------------------------------
#define TIME2DELAY (1000 * (unsigned long)ISetup.secDelayAlarm)
//---------------------------------
bool alarmItemDelayed::getInput(bool force)
{
  bool on = baseClass::getInput(force);
  if(on) {
    if(!onDelay && !force) {
      tmLast =  millis();
      G_writeOut(pinLed, LOW);
      onDelay = true;
      }
    if(!force) {
      if(millis() - tmLast < TIME2DELAY)
        on = false;
      else
        reset();
      }
    }
  else
    reset();
  return on;
}
//---------------------------------
void alarmItemDelayed::reset()
{
  if(tmLast) {
    tmLast = 0;
    G_writeOut(pinLed, HIGH);
    onDelay = false;
    }
}
//---------------------------------
//---------------------------------
alarmItem* alarm::makeItem(uint8_t ix, uint8_t pinInp)
{
  if(ix > 1)
    return new alarmItem(pinInp);
  return new alarmItemDelayed(pinInp, ALARM_PIN_LED_DELAY);
}
//---------------------------------
void alarm::loop()
{
  if(!isSet(status, a_on)) {
    for(uint8_t i = 0; i < totItem; ++i)
      Items[i]->reset();
    }
  baseClass::loop();
}
