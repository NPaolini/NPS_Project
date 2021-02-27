#ifndef lighting_H
#define lighting_H
//---------------------------------
#define MAX_SWITCH 4
//---------------------------------
class lighting
{
  public:
    lighting(pin_t pinOut, pin_t* pinInput, byte num);
    void begin();
    void loop();
    bool getStatus();
    bool getSoftStatus();
    void toggleSoft();
  private:
    pin_t* PinInput;
    byte Num;
    pin_t PinOut;
    byte status;
};
//---------------------------------
#endif
