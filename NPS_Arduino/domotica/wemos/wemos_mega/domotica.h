#ifndef domotica_H
#define domotica_H

#include "pin_def.h"
#include "mydebug.h"
//---------------------------------
inline bool isSetBit(uint32_t v, uint32_t bit)
{
  return v & (1 << bit);
}
//---------------------------------
inline bool isSet(uint32_t v, uint32_t bit)
{
  return v & bit;
}
//---------------------------------
uint32_t setBit(uint32_t& v, uint32_t bit, bool set);
//---------------------------------
inline uint32_t setBitBit(uint32_t& v, uint32_t bit, bool set)
{
  return setBit(v, 1 << bit, set);
}
//------------------------------
#include <EEPROM.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SD.h>
#include <SPI.h>
#include <MFRC522.h>
//---------------------------------
#include <LiquidCrystal_I2C.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735

#include <Time.h>
#include <TimeLib.h>
//#define USE_HW_TIME
#ifdef USE_HW_TIME
#include <DS1302.h>
#else
class myRtc
{
  public:
    myRtc() {}
    void run(bool update);
    String getTimeStr();
    String getDateStr();
    TimeElements getTime();
    void setDOW(uint8_t d);
    void setTime(uint8_t h, uint8_t m, uint8_t s, uint8_t d, uint8_t month, uint16_t y);
};
#endif

#define DIM_USER      16
#define DIM_PASSWORD  16
#define MAX_USER      10
//------------------------------
#include "man_log.h"
#include "mplex.h"
#include "web_server.h"
#include "sensor.h"
#include "dht_sensor.h"
#include "lighting.h"
#include "pid.h"
#include "crono_termostato.h"
#include "email_sender.h"
#include "alarm.h"
#include "rfid.h"
#include "motor.h"
#include "keyb.h"
//------------------------------
#define MAX_DIM_EMAIL 64
//------------------------------
#define DIM_HEADER 3 // -> NPD
#define OFFS_DATA DIM_HEADER
//------------------------------
#define addr_EE_data 0

#define DIM_CRONO (24 * 2)
#define NUM_CRONO 7
#define TOT_CRONO (DIM_CRONO * NUM_CRONO)
#define DIM_EEPROM (1024 * 4)

#define addr_EE_crono (DIM_EEPROM - TOT_CRONO - 32)
//------------------------------
struct user_
{
  char u[DIM_USER];
  char p[DIM_PASSWORD];
  char rfid_code[RFID_DIM_CODE + 1];
  user_()
  {
    memset(u, 0, sizeof(u));
    memset(p, 0, sizeof(p));
    memset(rfid_code, 0, sizeof(rfid_code));
  }
};
//--------------------------
struct roll_up_shutterPos
{
  byte bits;
  byte pos[8];
  roll_up_shutterPos() : bits(0) { memset(pos, 0, sizeof(pos)); }
  byte getVal(uint8_t ix) { return pos[ix]; }
  void setVal(uint8_t ix, byte val) { pos[ix] = val; }
};
//------------------------------
struct infoEmail
{
  int16_t port;
  char smtp[MAX_DIM_EMAIL];
  char from[MAX_DIM_EMAIL];
  char to[MAX_DIM_EMAIL];
  char user[MAX_DIM_EMAIL];
  char psw[MAX_DIM_EMAIL];
  infoEmail() { memset(this, 0, sizeof(*this)); }
};
//------------------------------
struct infoSetup
{
  byte ip[4];
  byte dns[4];
  byte gateway[4];
  int16_t port;
  uint32_t useDebug;
  byte items[2]; // status, manual temper, gli altri dati direttamente in eeprom
  byte sec4endRun; // secondi prima del finecorsa software
  infoEmail iEmail;

  byte secDelayAlarm; // ritardo all'attivazione del primo allarme
  byte sd_on_use;

  user_ users[MAX_USER];
  uint32_t softBit; // buffer per bit di stato software luci
  uint32_t alarmBit;
  uint16_t secAlarmTiming1; // allarme attivo, sirena on
  uint16_t secAlarmTiming2; // tempo prima della riattivazione allarme (dopo spegnimento sirena)
  uint16_t secAlarmTiming3; // tempo prima della riattivazione allarme (dopo riconoscimento e spegnimento sirena)

  roll_up_shutterPos rusPos;
};

extern infoSetup* getISetup();
//------------------------------
extern uint32_t useDebug;

extern char g_Buff[1024];
//------------------------------
#define SIZE_A(a) (sizeof(a) / sizeof(*a))
//------------------------------
class man_serial
{
  public:
    man_serial() {}
    virtual ~man_serial() {}
    virtual void begin() = 0;
    virtual bool loop() = 0;
    virtual void printIP(uint16_t port) = 0;
    virtual void printParam(uint16_t* par, int8_t num) = 0;
    virtual void printEmailData() = 0;
};

man_serial* getManSerial();
//------------------------------
#define saveSetup() writeMasterEE((byte*)&ISetup, sizeof(ISetup))
//------------------------------
extern void enableSD();
extern void enableLan();
//------------------------------
extern void enableRFID();
extern void enableTFT();
//------------------------------
extern void reloadSDinfo(bool dump = false);
//------------------------------
// fromWhere -> 1 = web, 2 = rfid, 3 = keyb
extern void writeLogAccess(uint8_t ix_user, uint8_t fromWhere);
extern bool isValidKeyReduced(const char* code, const char* enter);
//------------------------------
#endif
