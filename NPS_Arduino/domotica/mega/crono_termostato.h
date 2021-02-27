#ifndef crono_termostato_H
#define crono_termostato_H
//----------------------------------
#define HOUR_ITEMS 24
#define TOT_ITEMS (HOUR_ITEMS * 2 + 1)
#define DIM_ITEMS (TOT_ITEMS + 1)
//----------------------------------
/*
  il primo elemento rappresenta lo stato del controller: 1 -> ON(1)/OFF(0), 2 -> auto(0)/manuale(1)
  OFF: non esegue alcuna azione
  ON : esegue l'azione dipendente dal bit 1 (valore 2)
  auto/manuale: se manuale, legge la temperatura impostata nel secondo elemento, se auto legge la temperatura impostata nell'ora corrispondente

  il secondo elemento rappresenta la temperatura per lo stato manuale

  a seguire ci sono gli elementi corrispondenti agli orari impostati con delta di mezz'ora
  ogni elemento ha un passo di un quarto di grado: es. 22.75° -> 91 (22 * 4 + 3) == (22.75 * 4)

*/
//----------------------------------
class crono_termostato
{
  public:
    crono_termostato(pin_t pin, unsigned long delayStep);
    virtual ~crono_termostato() {}
    virtual void setItems(const byte* buff, byte num, byte offs);
    virtual void getItems(byte* buff, byte num, byte offs);
    void begin();
    virtual bool Loop();
    void fillImpData(char* buff, int len);
    void toggleOnOff();
    void toggleAuto();
    void setValue(byte ix, byte val); // ix zero based, include anche i primi due valori
    void forceRefresh() { lastTime = 0; }
    byte getPinStatus() { return pinStatus; }
    void getImpTemp(char* buff);
  private:
    byte items[DIM_ITEMS];
    pin_t pinOut;
    byte pinStatus;
    unsigned long lastTime;
    unsigned long DelayStep;
    PID pid;
    byte getIx(bool forceAuto);
};
//----------------------------------
#endif
