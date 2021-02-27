#ifndef alarm_H
#define alarm_H
//---------------------------------
/*
  Il sensore utilizzato e' PIR HC-SR501.
  I collegamenti sono, lato componenti, con i pin nella parte bassa:
  VCC - OUT - GND

  Esempi: https://win.adrirobot.it/sensori/pir_sensor/pir_sensor_hc-sr501_arduino.htm
*/
//---------------------------------
class alarmItem
{
  public:
    alarmItem(pin_t pinInput);
    virtual ~alarmItem() {}
    virtual void begin();
    virtual bool getInput(bool force);
    virtual bool isExcluted() { return Excluted; }
    virtual void setExcluted(bool set) { Excluted = set; }
    virtual void reset() { }

  protected:
    pin_t PinInput;
    bool Excluted;
    bool active;
};
//---------------------------------
class ledRGB
{
  public:
    ledRGB(pin_t firstPin) : FirstPin(firstPin) {}
    enum stat { inactive, onWait, active, onAlarm };
    // 0 -> spegne, 1 | 2 | 3 -> accende il rispettivo spegnendo gli altri
    void setPin(pin_t pin);
    void begin();
  private:
    pin_t FirstPin;
};
//---------------------------------
#define MAX_ITEM_ALARM 16
//---------------------------------
class baseAlarm
{
  public:
    baseAlarm(pin_t pinLedOn, pin_t pinOut);
    virtual ~baseAlarm();
    virtual void begin(pin_t* pinInput, uint8_t num);
    virtual void loop();
    byte getStatus();
    bool acknoledge();
    void enable(bool set);
    bool isEnabled();
    void exclude(uint8_t ix, bool set);
    bool isExcluted(uint8_t ix);
    bool getItemStatus(uint8_t ix, bool force);
    uint8_t getTotItem() { return totItem; }
    enum { a_on = 1 << 0, a_active = 1 << 1, a_ack = 1 << 2, a_wait = 1 << 3 };
  protected:
    virtual alarmItem* makeItem(uint8_t ix, uint8_t pinInp) = 0;
    alarmItem* Items[MAX_ITEM_ALARM];
    uint8_t totItem;
    pin_t PinOut;
    pin_t PinLedOn;
    uint32_t status;
    unsigned long tmWait;
    unsigned long tmLast;
    ledRGB LedRGB;
};
//---------------------------------
class alarmItemDelayed : public alarmItem
{
  private:
    typedef alarmItem baseClass;
  public:
    alarmItemDelayed(pin_t pinInput, pin_t pinLed) :
      baseClass(pinInput), pinLed(pinLed), tmLast(0), onDelay(false) {}
    virtual bool getInput(bool force);
    void reset();
  protected:
    pin_t pinLed;
    unsigned long tmLast;
    bool onDelay;
};
//---------------------------------
class alarm : public baseAlarm
{
  private:
    typedef baseAlarm baseClass;
  public:
    alarm(pin_t pinLedOn, pin_t pinOut) :
      baseClass(pinLedOn, pinOut) {}
    virtual void loop();
  protected:
    virtual alarmItem* makeItem(uint8_t ix, uint8_t pinInp);
};
//---------------------------------
#endif
