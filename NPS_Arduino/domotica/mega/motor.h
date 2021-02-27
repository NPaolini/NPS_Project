#ifndef motor_H
#define motor_H
//---------------------------------
#define TIME_TO_STOP_MOTOR (ISetup.sec4endRun) //(1000L * 20)
// avvio motore
#define IX_PIN_RUN       0
// verifica linea
#define IX_PIN_CHECK_RUN 1

// nuova versione, nessun fine corsa hardware ed un solo pin di controllo-comando
// Se viene premuto il pulsante non si può attivare da web
// la scadenza per il fine corsa vale anche per il pulsante
//---------------------------------
class motor
{
  public:
    motor(pin_t pin_run_mot, pin_t pin_check) :
      pinRunMot(pin_run_mot), pinCheckMot(pin_check), stat(ems_undef), hardRun(false), softRun(false), lastTime(0), pos(0)
      {

      }
    enum ems { ems_undef = 0, ems_onRun = 1 << 0, ems_onWait = 1 << 1, ems_onEndrun = 1 << 2,
               ems_dejaWaiting = ems_onRun | ems_onWait
               };
    enum onoff { e_ON = 0, e_OFF = 1 };
    ems getStatus() { return stat; }
    void begin();
    bool action(); // true if changed status
    bool run(bool on);
    void reset(bool endrun);
    void abort();
    bool getHardStatus();
    bool getTrueHardStatus();
    byte getEndRun();
    bool getSoftStatus() { return softRun; }
    int getPos() { return (int)pos; }
    void setPos(byte p) { pos = p; }
  protected:
    pin_t pinRunMot;
    pin_t pinCheckMot;
    ems stat;
    bool hardRun;
    bool softRun;
    unsigned long lastTime;
    unsigned long offsTime;
    byte pos; // 0 -> indefinito, altrimenti la posizione in base al time
};
/*
stati:
0 -> indefinito
1 -> premuto on da pulsante
2 -> premuto on da web
3 -> scaduto time
4 -> rilasciato pulsante
5 -> rilascia(to) da web
6 -> finecorsa
----------------
transizioni
0 -> 1, 2
1 -> 4, 3 (6)
2 -> 5, 3 (6)
3 -> 5, 6
4 -> 0
5 -> reset() -> 0
6 -> reset() -> 0
*/
//--------------------------
class roll_up_shutter
{
  public:
    roll_up_shutter(motor* mot_up, motor* mot_dn) : lastRun(255)
      { Mot[0] = mot_up; Mot[1] = mot_dn; }
    void begin();
    bool loop(); // true if changed status
    void runByWeb(bool down, bool set);
    void fillForWeb(byte ix, char* buff, int len);
    void setPos(byte up, byte dn) { Mot[0]->setPos(up); Mot[1]->setPos(dn); }
    void savePos(int ix);
  protected:
    byte lastRun;
    motor* Mot[2];
    void verifyPos(byte ixOn);
    bool performAct(byte ixOn);
    char* addValMot(char* buff, byte ix, motor* m);    
};
//--------------------------
#endif
