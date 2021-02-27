#ifndef mplex_H
#define mplex_H
//---------------------------------
#define MAX_MPX_SEL 4
//---------------------------------
class mplex
{
  public:
    mplex(pin_t pinSig, pin_t* pinSel);
    void begin();
    bool getStatus(pin_t pin);
  private:
    void prepare(pin_t pin);
    pin_t PinSel[MAX_MPX_SEL];
    pin_t pinSig;
    bool useShReg;
};
//---------------------------------
class manageMPlex
{
  public:
    manageMPlex(pin_t* ena_pin, byte num) : EnaPin(ena_pin), Num(num), useShReg(false), oldEnabled(255) {}
    // usa riferimenti esterni a oggetti mplex creati
    bool digRead(pin_t pin);
    void begin();
  private:
    void enable(byte ix);
    pin_t* EnaPin;
    byte Num;
    bool useShReg;
    byte oldEnabled;
};
//---------------------------------
#define SR_CASCADE 4
//---------------------------------
class sh_reg
{
  public:
    sh_reg(pin_t clockPin, pin_t latchPin, pin_t dataPin);
    void begin();
    void setPinValue(pin_t pin, bool set);
    bool preparePinValue(pin_t pin, bool set);
    void finalizeValue();
  private:
    pin_t ClockPin;
    pin_t LatchPin;
    pin_t DataPin;
    uint8_t Values[SR_CASCADE];
    void setValues();
};
//---------------------------------
#endif
