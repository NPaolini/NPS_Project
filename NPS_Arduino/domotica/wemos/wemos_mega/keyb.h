#ifndef keyb_H
#define keyb_H
//---------------------------------
#define MAX_PIN_KEYB_ROW 4
#define TKR_KEYB_12
#ifdef TKR_KEYB_12
  #define MAX_PIN_KEYB_COL 5
  #define MIN_SQUARE 3
#else
  #define MAX_PIN_KEYB_COL 4
#endif
//---------------------------------
class keyb
{
  public:
    keyb(pin_t* pinRead, pin_t* pinWrite) : pinRead(pinRead), pinWrite(pinWrite) { }
    void begin();
    int8_t getStatus(); // -1 nessun tasto premuto
    char codeToChar(int8_t v);
    int8_t charToCode(char v);
  private:
    pin_t* pinRead;
    pin_t* pinWrite;
};
//---------------------------------
#endif
