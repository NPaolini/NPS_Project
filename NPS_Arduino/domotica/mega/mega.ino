#include <Ethernet.h>

#include "domotica.h"
#include "man_eeprom.h"
//--------------------------------------------------
pin_t lastPinSPI = -1;
static pin_t allpin[] = {
  PIN_ET_CARD, PIN_SD_CARD1, PIN_SD_CARD2, RFID_SS_PIN, TFT_CS, PIN_SHIFT_REG_LATCH
  };
//--------------------------------------------------
void enableBegin()
{
  lastPinSPI = -1;
  for(byte i = 0; i < SIZE_A(allpin); ++i) {
    pinMode(allpin[i], OUTPUT);
    digitalWrite(allpin[i], HIGH);
    }
}
//--------------------------------------------------
void enableGen(pin_t pin)
{
  if(pin == lastPinSPI)
    return;
  if(lastPinSPI != -1)
    digitalWrite(lastPinSPI, HIGH);
  lastPinSPI = pin;
  digitalWrite(lastPinSPI, LOW);
}
//--------------------------------------------------
void enableSD() { enableGen(PIN_SD_CARD1); }
void enableLan(){ enableGen(PIN_ET_CARD); }

void enableSD2() { enableGen(PIN_SD_CARD2); }
void enableTFT() { enableGen(TFT_CS); }
void enableRFID(){ enableGen(RFID_SS_PIN); }
void enableSHIFT_REG(){ enableGen(PIN_SHIFT_REG_LATCH); }
//--------------------------------------------------
#ifdef USE_FULL_TIME
DS1302 rtc(RTC_ce_pin, RTC_data_pin, RTC_sclk_pin);
EthernetUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

#elif defined(USE_HW_TIME)
DS1302 rtc(RTC_ce_pin, RTC_data_pin, RTC_sclk_pin);
#else
EthernetUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
myRtc rtc;
#endif

pin_t mplexSel1[] = { MPLEX_1_S1, MPLEX_1_S2, MPLEX_1_S3, MPLEX_1_S4 };
mplex MPlex1(MPLEX_1_Z, mplexSel1);

pin_t mplexSel2[] = { MPLEX_2_S1, MPLEX_2_S2, MPLEX_2_S3, MPLEX_2_S4 };
mplex MPlex2(MPLEX_2_Z, mplexSel2);

pin_t mplexEna[] = { MPLEX_1_EN, MPLEX_2_EN };
manageMPlex ManMPlex(mplexEna, SIZE_A(mplexEna));

sh_reg Sh_Reg(PIN_SHIFT_REG_CLK, PIN_SHIFT_REG_LATCH, PIN_SHIFT_REG_DATA);
//---------------------------------
motor mot1U(MOTOR_OUT_1_U, MOTOR_INP_1_U);
motor mot1D(MOTOR_OUT_1_D, MOTOR_INP_1_D);

motor mot2U(MOTOR_OUT_2_U, MOTOR_INP_2_U);
motor mot2D(MOTOR_OUT_2_D, MOTOR_INP_2_D);

motor mot3U(MOTOR_OUT_3_U, MOTOR_INP_3_U);
motor mot3D(MOTOR_OUT_3_D, MOTOR_INP_3_D);

motor mot4U(MOTOR_OUT_4_U, MOTOR_INP_4_U);
motor mot4D(MOTOR_OUT_4_D, MOTOR_INP_4_D);

roll_up_shutter rollUpShutter1(&mot1U, &mot1D);
roll_up_shutter rollUpShutter2(&mot2U, &mot2D);
roll_up_shutter rollUpShutter3(&mot3U, &mot3D);
roll_up_shutter rollUpShutter4(&mot4U, &mot4D);

roll_up_shutter* rollUpShutter[] {
  &rollUpShutter1, &rollUpShutter2, &rollUpShutter3, &rollUpShutter4
};
//---------------------------------
pin_t l1Inp[] = { RELE_INP_1, RELE_INP_2 };
pin_t l2Inp[] = { RELE_INP_3 };
pin_t l3Inp[] = { RELE_INP_4 };
pin_t l4Inp[] = { RELE_INP_5 };
pin_t l5Inp[] = { RELE_INP_6 };
pin_t l6Inp[] = { RELE_INP_7 };
pin_t l7Inp[] = { RELE_INP_8 };
pin_t l8Inp[] = { RELE_INP_9 };
lighting L1(RELE_OUT_1, l1Inp, SIZE_A(l1Inp));
lighting L2(RELE_OUT_2, l2Inp, SIZE_A(l2Inp));
lighting L3(RELE_OUT_3, l3Inp, SIZE_A(l3Inp));
lighting L4(RELE_OUT_4, l4Inp, SIZE_A(l4Inp));
lighting L5(RELE_OUT_5, l5Inp, SIZE_A(l5Inp));
lighting L6(RELE_OUT_6, l6Inp, SIZE_A(l6Inp));
lighting L7(RELE_OUT_7, l7Inp, SIZE_A(l7Inp));
lighting L8(RELE_OUT_8, l8Inp, SIZE_A(l8Inp));
lighting* AllLighting[] = { &L1, &L2, &L3, &L4, &L5, &L6, &L7, &L8 };

//---------------------------------
int8_t currDOW = 0;
//---------------------------------
uint32_t useDebug = 0;
uint8_t currRain = 0;
//---------------------------------
crono_termostato C_T(CRONO_TERM_OUT, 1000);
//---------------------------------
pin_t onWirePin[] = { SENSOR_ONWIRE_1, SENSOR_ONWIRE_2, SENSOR_ONWIRE_3 };
Sensors sensors(onWirePin, sizeof(onWirePin) / sizeof(onWirePin[0]));
//---------------------------------
DHT_Sensor dht(false, SENSOR_DHT_1);
//---------------------------------
//LiquidCrystal_I2C lcd(0x27,16,2);
//---------------------------------
bool needUpdateCrono;
int8_t needSendEmail = 0;
pin_t inputAlarm[] = {
  ALARM_SENSOR_INP_1, ALARM_SENSOR_INP_2, ALARM_SENSOR_INP_3, ALARM_SENSOR_INP_4,
        ALARM_SENSOR_INP_5, ALARM_SENSOR_INP_6, ALARM_SENSOR_INP_7, ALARM_SENSOR_INP_8 };
alarm Alarm(ALARM_PIN_LED1, ALARM_PIN_OUT);

#ifdef TKR_KEYB_12
pin_t keybRead[] = {KEYB_READ1, KEYB_READ2, KEYB_READ3, KEYB_READ4, KEYB_READ5 };
#else
pin_t keybRead[] = {KEYB_READ1, KEYB_READ2, KEYB_READ3, KEYB_READ4};
#endif
pin_t keybWrite[] = {KEYB_WRITE1, KEYB_WRITE2, KEYB_WRITE3, KEYB_WRITE4};
keyb Keyb(keybRead, keybWrite);
uint32_t lastKeyDown;
byte keyBuffer[RFID_DIM_ONLY_CODE + 1];
byte keyPos;
int8_t keyPressed;
bool canDisplayKey;
//---------------------------------------
infoSetup ISetup = {
    {192, 168, 1, 98 }, {192, 168, 1, 254 }, {192, 168, 1, 254 }, 80,
    0, // relè
    { 0 }, // crono
    0,     // softBit
    0, "", "", "", "", "", // email
};
infoSetup* getISetup() { return &ISetup; }
//---------------------------------------
rfid RFID;
SDClass SD2;

manCookie Cookie;
static char currSID[DIM_COOKIE + 1] = {0};

static void (*enaSd)() = enableSD;
static SDClass* currSD = &SD;

char g_Buff[1024];
//---------------------------------
// i dati che seguono sono quelli della struttura infoSetup
byte stdEE_data[DIM_HEADER] = { 'N', 'P', 'D' };
//---------------------------------------
byte mac[] = {0xDE, 0xAB, 0xBE, 0xEF, 0xFE, 0xED };
//----Adafruit --------------------------
// hardware
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
// software
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);
#include "man_tft.h"
//---------------------------------------
IFace_web_server* server = 0;
//------------------------------
int8_t getCurrWeek()
{
#if defined(USE_HW_TIME) || defined(USE_FULL_TIME)
  int8_t dow = rtc.getTime().dow;
  if(!dow)
    return 1;
#else
  int8_t dow = ::weekday() - 1;
  if(dow <= 0)
    dow = 7;
#endif
  return dow;
}
//---------------------------------
void G_pinMode(pin_t pin, int how)
{
  if(OFFS_MPX_1 > pin)
    pinMode(pin, how);
}
//---------------------------------
bool G_digRead(pin_t pin)
{
  return ManMPlex.digRead(pin);
}
//---------------------------------
void G_prepareWriteOut(pin_t pin, bool set)
{
  if(OFFS_SR_1 > pin)
    digitalWrite(pin, set);
  else
    Sh_Reg.preparePinValue(REM_SR_1(pin), set);
}
//---------------------------------
void G_finalizeWriteOut()
{
  Sh_Reg.finalizeValue();
}
//---------------------------------
void G_writeOut(pin_t pin, bool set)
{
  if(OFFS_SR_1 > pin)
    digitalWrite(pin, set);
  else
    Sh_Reg.setPinValue(REM_SR_1(pin), set);
}
//---------------------------------
bool G_getInput(pin_t pin, bool prev)
{
  bool active = G_digRead(pin) == HIGH;
  if(active ^ prev) {
    do {
      delay(10L);
      prev = active;
      active = G_digRead(pin) == HIGH;
      } while(active ^ prev);
    }
  return active;
}
//---------------------------------
bool G_getInput2(pin_t pin)
{
  bool active = G_digRead(pin) == HIGH;
  bool prev;
  do {
    prev = active;
    delay(5L);
    active = G_digRead(pin) == HIGH;
    } while(active ^ prev);
  return active;
}
//-------------------------------------------------------------------
unsigned long lastSaved = 0;
//-------------------------------------------------------------------
extern void(*Restart)(void);
extern void reloadSDinfo(bool dump = false);
//-------------------------------------------------------------------
void setup()
{
  SPI.begin();
  if(!readMasterEE(&ISetup))
    initializeEE(&ISetup);
  useDebug = ISetup.useDebug & D_BIT_SETUP;

  needSendEmail = 0;
  man_serial* mserial = getManSerial();
  mserial->begin();
  enableBegin();
  Sh_Reg.begin();
  ManMPlex.begin();
  server = getServer();
  server->beginLan(ISetup.ip, ISetup.dns, ISetup.gateway, mac);
  server->beginServer(ISetup.port);
  DEBUG_SETUP(mserial->printIP(ISetup.port))
  currSID[0] = 0;
  for(int i = 0; i < SIZE_A(rollUpShutter); ++i) {
    rollUpShutter[i]->begin();
    if(ISetup.rusPos.bits & (1 << i))
      rollUpShutter[i]->setPos(0, ISetup.rusPos.pos[i]);
    else
      rollUpShutter[i]->setPos(ISetup.rusPos.pos[i], 0);
    }
  if(!ISetup.sd_on_use || ISetup.sd_on_use > 2) {
    ISetup.sd_on_use = 1;
    saveSetup();
    }
  DEBUG_SETUP(mserial->printEmailData())

  Alarm.begin(inputAlarm, SIZE_A(inputAlarm));

  enableTFT();
  tft.initR(INITR_BLACKTAB);      // Init ST7735S chip, black tab
  tft.setRotation(1);
  tft.fillScreen(ST77XX_BLACK);
  tft_println(F(""), 2, (uint16_t)ST77XX_GREEN);
  tft_println(F("   Welcome"));
  tft_println(F("   to Home"));
  tft_println(F("  Automation"));
  tft_println(F("    by NPS"));
  tft.setTextWrap(false);

  RFID.begin();
#ifdef USE_FULL_TIME
  rtc.halt(false);
  rtc.writeProtect(false);
  timeClient.begin();
  timeClient.update();
#else
#ifdef USE_HW_TIME
  rtc.halt(false);
  rtc.writeProtect(false);
#else
  timeClient.begin();
  rtc.update(false);
#endif
#endif
  currDOW = getCurrWeek();
  C_T.setItems(ISetup.items, SIZE_A(ISetup.items), 0);
  C_T.begin();
  needUpdateCrono = true;
  updateCronoWorking(true);
  for(uint32_t i = 0; i < SIZE_A(AllLighting); ++i) {
    AllLighting[i]->begin();
    if(ISetup.softBit & (1 << i))
      AllLighting[i]->toggleSoft();
    }
  dht.begin();
  dht.loop(false);
  DEBUG_SETUP(Serial.println("Sto cercando i sensori..."))
  sensors.begin();
  DEBUG_SETUP(sensors.debug(true))
  sensors.loadData();
  needSendEmail = 0;
  DEBUG_SETUP(Serial.print(F("occupazione EEPROM: ")))
  DEBUG_SETUP(Serial.println(sizeof(ISetup) + TOT_CRONO + sizeof(stdEE_data)))
  // usare un pin non usato
  randomSeed(analogRead(1));
  lastSaved = millis();
  enableTFT();
  tft.fillScreen(ST77XX_BLACK);
  Keyb.begin();
  lastKeyDown = 0;
  memset(keyBuffer, 0, sizeof(keyBuffer));
  keyPos = 0;
  keyPressed = -1;
  canDisplayKey = false;
  Serial.println(F("Welcome to Home Automation by NPS"));
#ifdef USE_FULL_TIME
  timeClient.update();
#else
  #ifndef USE_HW_TIME
  // lo rifacciamo ...
  rtc.update(true);
  #endif
#endif
}
//-------------------------------------------------------------------
const char* nextParam(const char* buff)
{
  while(*buff && *buff != ',')
    ++buff;
  if(!*buff)
    return 0;
  return ++buff;
}
//-------------------------------------------------------------------
int8_t G_fillParam(uint16_t* par, int8_t num, const char* buff)
{
  par[0] = atoi(buff);
  int8_t i = 1;
  for(; i < num; ++i) {
    buff = nextParam(buff);
    if(!*buff)
      break;
    par[i] = atoi(buff);
    }
  return i;
}
//-------------------------------------------------------------------
const char* parseDOW(const char* buff)
{
  const char* p = buff;
  while(*buff) {
    if('W' == *buff) {
      buff = nextParam(buff);
      if(!*buff)
        break;
      int8_t t = atoi(buff);
      if(t && t <= 7) {
        needUpdateCrono |= currDOW != t;
        currDOW = t;
        }
      p = nextParam(buff);
      break;
      }
    ++buff;
    }
  return p;
}
//-------------------------------------------------------------------
void updateCronoWorking(bool force)
{
  int8_t curr = getCurrWeek();
  if(currDOW != curr && !force)
    return;
  needUpdateCrono = true;
  readCronoEE(g_Buff, curr);
  C_T.setItems(g_Buff, DIM_CRONO, 2);
}
//-------------------------------------------------------------------
void setSingleCronoGen(uint16_t par[], int8_t num)
{
  for(int8_t i = 0; i < num; i += 2) {
    if(!par[i])
      break;
    if(1 == par[i])
      C_T.setValue(par[i], par[i + 1] / 25);
    else {
      byte val = par[i + 1] / 25;
      int8_t addr = par[i] - 2;
      writeCronoEESingle(&val, &addr, 1, currDOW);
      }
  }
  C_T.getItems(ISetup.items, SIZE_A(ISetup.items), 0);
  updateCronoWorking(false);
  C_T.forceRefresh();
  saveSetup();
}
//-------------------------------------------------------------------
#define COLOR_565(r,g,b) ((((uint16_t)r & 0xF8) << 8) | (((uint16_t)g & 0xFC) << 3) | ((uint16_t)b >> 3))
#define ST77XX_GRAY_(c) COLOR_565(c, c, c)
#define ST77XX_GRAY ST77XX_GRAY_(80)
//-------------------------------------------------------------------
void drawAlarmState(int x, int y, int r, int color)
{
  tft.fillRoundRect(x, y, r, r, 2, color);
}
//-------------------------------------------------------------------
void clearRect()
{
  int16_t w = tft.width();
  int16_t h = tft.height();
  int16_t y = tft.getCursorY();
  tft.fillRect(0, y, w, h - y, ST77XX_BLACK);
}
//-------------------------------------------------------------------
#define TEXT_COLOR (uint16_t)ST77XX_GREEN
//-------------------------------------------------------------------
void displayDate()
{
  static byte oldAl = 255;
  enableTFT();
  if(lastKeyDown) {
    if(oldAl != 255) {
      tft.fillScreen(ST77XX_BLACK);
      tft_println(F(""), 2, TEXT_COLOR, 0, 0);
      tft_println(F(""));
      }
    if(canDisplayKey) {
      tft.setTextWrap(true);
      tft_print(F("*"), 3, TEXT_COLOR);
      tft.setTextWrap(false);
      canDisplayKey = false;
      }
    oldAl = 255;
    return;
    }
  tft_print(F(""), 2, TEXT_COLOR, 0, 0);
#if defined(USE_HW_TIME) || defined(USE_FULL_TIME)
  Time t = rtc.getTime();
  switch(t.dow) {
    case MONDAY:
      tft_print(F("Lun"), 2, TEXT_COLOR, 0, 0);
      break;
    case TUESDAY:
      tft_print(F("Mar"), 2, TEXT_COLOR, 0, 0);
      break;
    case WEDNESDAY:
      tft_print(F("Mer"), 2, TEXT_COLOR, 0, 0);
      break;
    case THURSDAY:
      tft_print(F("Gio"), 2, TEXT_COLOR, 0, 0);
      break;
    case FRIDAY:
      tft_print(F("Ven"), 2, TEXT_COLOR, 0, 0);
      break;
    case SATURDAY:
      tft_print(F("Sab"), 2, TEXT_COLOR, 0, 0);
      break;
    case SUNDAY:
      tft_print(F("Dom"), 2, TEXT_COLOR, 0, 0);
      break;
    }
#else
  int t = weekday();
  switch(t) {
    case dowMonday:
      tft_print(F("Lun"), 2, TEXT_COLOR, 0, 0);
      break;
    case dowTuesday:
      tft_print(F("Mar"), 2, TEXT_COLOR, 0, 0);
      break;
    case dowWednesday:
      tft_print(F("Mer"), 2, TEXT_COLOR, 0, 0);
      break;
    case dowThursday:
      tft_print(F("Gio"), 2, TEXT_COLOR, 0, 0);
      break;
    case dowFriday:
      tft_print(F("Ven"), 2, TEXT_COLOR, 0, 0);
      break;
    case dowSaturday:
      tft_print(F("Sab"), 2, TEXT_COLOR, 0, 0);
      break;
    case dowSunday:
      tft_print(F("Dom"), 2, TEXT_COLOR, 0, 0);
      break;
    }
#endif

  tft_print(F(" "));
  tft_println(rtc.getTimeStr());
  tft_println(rtc.getDateStr());

  tft_print(F("Allarmi: "), (uint16_t)ST77XX_WHITE);
  if(Alarm.isEnabled())
    tft_println(F("ON "), (uint16_t)ST77XX_GREEN);
  else
    tft_println(F("OFF"), (uint16_t)ST77XX_RED);

  tft_println(F(""), (uint8_t)0);
  tft_print(F("Temp: "), 1, (uint16_t)ST77XX_YELLOW);
  tft_print(F("Int:"), (uint16_t)ST77XX_WHITE);
  char tmp[40];
  char* p = tmp;
  dtostrf (dht.getTemperature(), 2, 2, tmp);
  tft_print(tmp, (uint16_t)ST77XX_YELLOW);

  int8_t tot = sensors.getTot();
  if(tot) {
    tft_print(F(" Est:"), (uint16_t)ST77XX_WHITE);
    float t[8];
    sensors.fillData(t);
    int8_t j = 0;
    float t2 = 0;
    for(int8_t i = 0; i < tot; ++i)
      if(t[i]) {
        t2 += t[i];
        ++j;
        }
    if(j)
      t2 /= j;
    dtostrf (t2, 2, 2, tmp);
    tft_println(tmp, (uint16_t)ST77XX_YELLOW);
    }
  tft_println(F(""), (uint8_t)0);
  byte item;
  C_T.getItems(&item, 1, 0);
  tft_print(F("CronoTerm:"), 1, (uint16_t)ST77XX_WHITE);
  if(item & 1) {
    tft_print(F("ON - "), (uint16_t)ST77XX_GREEN);
    if(item & 2)
      tft_print(F("Man  - "));
    else
      tft_print(F("Auto - "));
    if(C_T.getPinStatus())
      tft_println(F("On "));
    else
      tft_println(F("Off"), (uint16_t)ST77XX_WHITE);
    tft_print(F("Temp. Imp: "), (uint16_t)ST77XX_WHITE);
    C_T.getImpTemp(tmp);
    tft_println(tmp, (uint16_t)ST77XX_YELLOW);
    }
  else {
    tft_println(F("OFF            "));
    tft_println(F("                  "));
    }
  byte al = Alarm.getStatus();
  if(al != oldAl) {
    oldAl = al;
    if(!isSet(al, alarm::a_on))
      clearRect();
    else {
      tft_println(F(""), (uint8_t)0);
      // è stato riconosciuto oppure è scaduto il time di allarme
      if(isSet(al, alarm::a_wait))
        tft_println(F("  In attesa "), 2, (uint16_t)ST77XX_BLUE);
      // l'allarme è attivo
      else if(isSet(al, alarm::a_active))
        tft_println(F("  In allarme"), 2, (uint16_t)ST77XX_RED);
      else
        tft_println(F("  No allarmi"), 2, (uint16_t)ST77XX_GREEN);
      }
    }
  uint8_t count = Alarm.getTotItem();
  if(!count)
    return;
  int16_t w = tft.width();
  int16_t h = tft.height();
  int16_t y = h - 10;
  int16_t dx = w / count;
  w = dx / 2;
  int16_t x = w / 2;
  if(w > 8)
    w = 8;
  for(uint8_t i = 0; i < count; ++i) {
    uint16_t color = ST77XX_GRAY;
    if(Alarm.isExcluted(i)) {
      if(Alarm.getItemStatus(i, true))
        color = ST7735_ORANGE;
      }
    else {
      if(Alarm.getItemStatus(i, true))
        color = ST77XX_RED;
      else
        color = ST77XX_GREEN;
      }

    drawAlarmState(x, y, w, color);
    x += dx;
    }
}
//-------------------------------------------------------------------
void getDateTimeStr(char* buff, int8_t dim)
{
#if defined(USE_HW_TIME) || defined(USE_FULL_TIME)
  strcpy(buff, rtc.getDateStr());
  strcat(buff, " - ");
  strcat(buff, rtc.getTimeStr());
#else
  strcpy(buff, rtc.getDateStr().c_str());
  strcat(buff, " - ");
  strcat(buff, rtc.getTimeStr().c_str());
#endif
}
//-------------------------------------------------------------------
void checkMidNight()
{
  static uint8_t old;
#if defined(USE_HW_TIME) || defined(USE_FULL_TIME)
  Time tm = rtc.getTime();
  if(old != tm.date) {
    old = tm.date;
    currDOW = getCurrWeek();
    updateCronoWorking(true);
    }
#else
  time_t t = ::now();
  if(old != ::day(t)) {
    old = ::day(t);
    currDOW = getCurrWeek();
    updateCronoWorking(true);
    }
#endif
}
//-------------------------------------------------------------------
const char* alname[] = { "Sala (sensore)", "Portone ingresso", "Finestra cucina", "Camera (sensore)",
  "Camera (finestra)", "Cameretta (sensore)", "Cameretta (finestra)", "Finestra bagno" };
//-------------------------------------------------------------------
void manageKeybInput(int8_t* keyBuffer, byte dim)
{
  uint8_t j = 0;
  infoSetup* isetup = getISetup();
  for(; j < MAX_USER; ++j) {
    if(isValidKeyReduced(isetup->users[j].rfid_code, keyBuffer))
      break;
    }
  if(MAX_USER == j)
    return;

  bool enabled = Alarm.isEnabled();
  bool onlog = false;
  if('0' == keyBuffer[RFID_DIM_ONLY_CODE]) {
    if(enabled) {
      Alarm.enable(false);
      onlog = true;
      }
    }
  else if('1' == keyBuffer[RFID_DIM_ONLY_CODE]) {
    if(!enabled) {
      Alarm.enable(true);
      onlog = true;
      }
    }
  else
    return;
  if(onlog)
    writeLogAccess(j, 3);
}
//-------------------------------------------------------------------
bool onDaylight(int day, int month, int dow)
{
  DEBUG_DATE(Serial.print(F("Mese:")); Serial.println(month))
  if(3 == month) { // marzo, ultima domenica
    if(day >= 31 - 7 + dow)
      return true;
    return false;
    }
  if(10 == month) { // ottobre, ultima domenica
    if(day < 31 - 7 + dow)
      return true;
    return false;
    }
  if(3 < month && month < 10)
    return true;
  return false;
}
//-------------------------------------------------------------------
void updateTime()
{
#ifdef USE_FULL_TIME
  static unsigned long oldTime = 0;
  bool notValid = false;
  if(!oldTime || millis() > oldTime + 1000L * 30) {
    Time rtcTm = rtc.getTime();
    notValid = 2000 >= rtcTm.year;
    }
  if(notValid || millis() > oldTime + 1000L * 60 * 60) {
    if(notValid)
      timeClient.forceUpdate();
    else
      timeClient.update();
    unsigned long epochTime = timeClient.getEpochTime();
    tmElements_t tm;
    breakTime((time_t)epochTime, tm);
    int monthDay = tm.Day;
    int currentMonth = tm.Month;
    int currentYear = tmYearToCalendar(tm.Year);
    if(currentYear < 2000)
      return;
    time_t offset = 3600;
    if(onDaylight(monthDay, currentMonth, timeClient.getDay()))
      offset += 3600;
    timeClient.setTimeOffset(offset);
    int currentHour = timeClient.getHours();
    int currentMinute = timeClient.getMinutes();
    int currentSecond = timeClient.getSeconds();
    int8_t dow = tm.Wday - 1;
    if(dow <= 0)
      dow = 7;
    rtc.setTime(currentHour, currentMinute, currentSecond);
    rtc.setDate(monthDay, currentMonth, currentYear);
    rtc.setDOW(dow);
    oldTime = millis();
    }
#else
  #ifndef USE_HW_TIME
  rtc.loop();
  #endif
#endif
}
//-------------------------------------------------------------------
void loop()
{
  static bool first = true;
  if(first) {
    enableTFT();
    tft.fillScreen(ST77XX_BLACK);
    first = false;
    }
  man_serial* mserial = getManSerial();
  if(mserial->loop())
    return;
  checkMidNight();
  RFID.loop();
  if(needSendEmail > 0) {
    iSenderEmail* email = getSenderEmail();
    email->begin();
    strcpy(g_Buff, "Rilevato allarme:");
    uint8_t num = Alarm.getTotItem();
    char* p = g_Buff + strlen(g_Buff);
    for(uint8_t i = 0; i < num; ++i) {
      if(Alarm.getItemStatus(i, true)) {
        sprintf(p, " %s[%d],", alname[i], i + 1);
        p += strlen(p);
        }
      }
    *--p = 0;
    DEBUG_EMAIL(Serial.println(g_Buff))
    needSendEmail = 0;
    email->send(g_Buff);
    needSendEmail = 0;
    }

  if(!server || !server->webServerLoop()) {
    sensors.loadData();
    dht.loop(true);
    }
  for(int i = 0; i < SIZE_A(rollUpShutter); ++i) {
    rollUpShutter[i]->loop();
    rollUpShutter[i]->savePos(i);
    }
  static unsigned long prev;
  unsigned long curr = millis();
  if(curr - prev > 300) {
    int8_t stat = Keyb.getStatus();
    if(stat != keyPressed) {
      keyPressed = stat;
      if(-1 != keyPressed) {
        if(lastKeyDown && keyPos < sizeof(keyBuffer))
          ++keyPos;
        lastKeyDown = millis();
        keyBuffer[keyPos] = Keyb.codeToChar(keyPressed);
        DEBUG_SETUP(Serial.print(F("Premuto: ")); Serial.println(Keyb.codeToChar(keyPressed)))
        canDisplayKey = true;
        }
      }
    if(-1 == keyPressed && lastKeyDown) {
      if(millis() - lastKeyDown >= 3000) {
        lastKeyDown = 0;
        manageKeybInput(keyBuffer, keyPos + 1);
        keyPos = 0;
        memset(keyBuffer, 0, sizeof(keyBuffer));
        enableTFT();
        tft.fillScreen(ST77XX_BLACK);
        }
      }
    enableTFT();
    displayDate();
    prev = millis();
    }
  C_T.Loop();
  for(byte i = 0; i < SIZE_A(AllLighting); ++i)
    AllLighting[i]->loop();
  Alarm.loop();
  Cookie.loop();
  if(millis() - lastSaved > (1 * 60 * 1000L)) {
    saveSetup();
    lastSaved = millis();
    }
  updateTime();
}
//-------------------------------------------------------------------
void switchRele(const char* cmd)
{
  uint8_t ix = *cmd - 'A';
  if(ix < SIZE_A(AllLighting))
    AllLighting[ix]->toggleSoft();

  uint32_t old = 0;
  for(uint32_t i = 0; i < SIZE_A(AllLighting); ++i)
    old |= AllLighting[i]->getSoftStatus() << i;
  if(old != ISetup.softBit) {
    ISetup.softBit = old;
    saveSetup();
    }
}
//-------------------------------------------------------------------
void switchMotor(const char* cmd)
{
  if('Z' == *cmd) { // m=Z0 | m=Z1
    for(byte i = 0; i < SIZE_A(rollUpShutter); ++i)
      rollUpShutter[i]->runByWeb('0' == cmd[1], true);
    return;
    }
   // m=A00 | m=A11 | ...
  uint8_t ix = *cmd - 'A';
  if(ix >= SIZE_A(rollUpShutter))
    return;
  rollUpShutter[ix]->runByWeb('2' == cmd[1], '1' == cmd[2]);
}
//--------------------------------------------------
void writeLogExclude(uint8_t ix_user, uint8_t ix_zone)
{
  infoSetup* isetup = getISetup();
  const char* pu = ix_user >= MAX_USER ? "???" : isetup->users[ix_user].u;
  const char* ena = Alarm.isExcluted(ix_zone) ? "excluted by" : "incluted by";
  const char* log[] = { alname[ix_zone], ena, pu };
  man_log ml(man_log::eL_Access);
  ml.logSend(log, SIZE_A(log));
}
//--------------------------------------------------
void writeLogAck(uint8_t ix_user)
{
  infoSetup* isetup = getISetup();
  const char* pu = ix_user >= MAX_USER ? "???" : isetup->users[ix_user].u;
  const char* ack = "Acknoledge by";
  const char* log[] = { ack, pu };
  man_log ml(man_log::eL_Access);
  ml.logSend(log, SIZE_A(log));
}
//-------------------------------------------------------------------
void switchAlarm(const char* cmd)
{
  int8_t idUser = Cookie.getUser(currSID);
  if('a' == *cmd) {
    if(Alarm.acknoledge())
      writeLogAck(idUser);
    return;
    }
  if('e' == *cmd) {
    uint32_t ix = atoi(cmd + 1);
    if(ix > MAX_ITEM_ALARM)
      return;
    if(!ix) {
      bool enabled = Alarm.isEnabled();
      Alarm.enable(!enabled);
      setBitBit(ISetup.alarmBit, ix, !enabled);
      writeLogAccess(idUser, 1);
      }
    else {
      bool exclude = Alarm.isExcluted(ix - 1);
      Alarm.exclude(ix - 1, !exclude);
      setBitBit(ISetup.alarmBit, ix, !exclude);
      writeLogExclude(idUser, ix - 1);
      }
    saveSetup();
    return;
    }
  if('x' == *cmd) { // from,to,on_off
    uint16_t par[3] = {};
    int8_t num = G_fillParam(par, SIZE_A(par), cmd + 1);
    uint8_t from = par[0];
    uint8_t to = par[1];
    if(from > Alarm.getTotItem())
      from = 1;
    if(to > Alarm.getTotItem())
      to = Alarm.getTotItem();
    if(from > to) {
      uint8_t t = from;
      from = to;
      to = t;
      }
    for(uint16_t i = from; i <= to; ++i) {
      Alarm.exclude(i - 1, par[2]);
      setBitBit(ISetup.alarmBit, i, par[2]);
      writeLogExclude(idUser, i - 1);
      }
    saveSetup();
    }
}
//-------------------------------------------------------------------
void setCronoValue(const char* values)
{
  values = parseDOW(values);
  needUpdateCrono = true;
  // serie di coppie indice-valore
  uint16_t par[TOT_ITEMS * 2] = {};
  int8_t num = G_fillParam(par, SIZE_A(par), values);
  setSingleCronoGen(par, num);
}
//-------------------------------------------------------------------
void copyBlock(const char* cmd)
{
  cmd += 2;
  int8_t s = atoi(cmd);
  parseDOW(cmd);
  readCronoEE(g_Buff, s);
  writeCronoEE(g_Buff, currDOW);
  needUpdateCrono = true;
  updateCronoWorking(true);
}
//-------------------------------------------------------------------
void toggleCrono(const char* cmd, bool forceRefresh)
{
  int8_t len = 1;
  switch(*cmd) {
    case 'A':
      C_T.toggleOnOff();
      break;
    case 'B':
      C_T.toggleAuto();
      break;
    case 'S':
      setCronoValue(cmd);
      len = SIZE_A(ISetup.items);
      break;
    case 'C':
      copyBlock(cmd);
      return;
    case 'W':
      parseDOW(cmd);
      if(forceRefresh)
        needUpdateCrono = 1;
      return;
    case 'R':
      needUpdateCrono = 1;
      break;
    default:
      return;
    }
  C_T.getItems(ISetup.items, len, 0);
  saveSetup();
}
//-------------------------------------------------------------------
void fillTemper(char* buff, int len)
{
  float tmp[8];
  int8_t tot = sensors.getTot();
  sensors.fillData(tmp);
  sprintf(buff, "%d|t|", tot);
  for(int8_t i = 0; i < tot; ++i) {
    char t[20];
    dtostrf (tmp[i], 2, 2, t);
    buff += strlen(buff);
    sprintf(buff, "%s|", t);
    }
}
//-------------------------------------------------------------------
#define ADD_OFFS_BASE(l, b) { int _l_ = strlen(b); b += _l_; l -= _l_; }
//-------------------------------------------------------------------
#define ADD_OFFS ADD_OFFS_BASE(len, buff)
#define ADD_OFFS2 { ADD_OFFS; if(len < 100) return 1; }
//-------------------------------------------------------------------
void fillRele(char* buff, int len)
{
  byte num = SIZE_A(AllLighting);
  sprintf(buff, "%d|r|", num);
  ADD_OFFS
  for(byte i = 0; i < num; ++i) {
    sprintf(buff, "%d|", AllLighting[i]->getStatus());
    ADD_OFFS
    }
}
//-------------------------------------------------------------------
void fillHumid(char* buff, int len)
{
  int8_t h = (int8_t)dht.getdHumidity();
  char tmp[20];
  dtostrf (dht.getTemperature(), 2, 2, tmp);
  char tmp2[20];
  dtostrf (dht.getHeatIndex(), 2, 2, tmp2);
  sprintf(buff, "3|u|%d|%s|%s|", h, tmp, tmp2);
}
//-------------------------------------------------------------------
void fillMotor(char* buff, int len)
{
  byte num = SIZE_A(rollUpShutter);
  sprintf(buff, "%d|m", num * 5 * 2);
  ADD_OFFS
  for(byte i = 0; i < num; ++i) {
    rollUpShutter[i]->fillForWeb(i * 2, buff, len);
    ADD_OFFS
    }
}
//-------------------------------------------------------------------
void fillAlarm(char* buff, int len)
{
  buff[0] = 0;
  static byte old = 0;
  byte num = Alarm.getTotItem();
  sprintf(buff, "%d|x|", (num + 1) * 3);
  ADD_OFFS

  static byte email_sent = 0;
  byte al = Alarm.getStatus();
  if(al != old) {
    DEBUG_ALARM(Serial.print("Status: "))
    DEBUG_ALARM(Serial.println(al, BIN))
    old = al;
    }
  bool on = isSet(al, alarm::a_on);
  byte v = 0;
  // è stato riconosciuto oppure è scaduto il time di allarme
  if(isSet(al, alarm::a_wait))
    v = 1;
  // l'allarme è attivo
  else if(isSet(al, alarm::a_active)) {
    v = 2;
    if(!email_sent) {
      email_sent = 1;
      needSendEmail = 1;
      }
    }
  if(!v || !on)
    email_sent = 0;

  sprintf(buff, "%d|%d|%d|", 0, on, v);
  ADD_OFFS
  for(uint8_t i = 0; i < num; ++i) {
    on = !Alarm.isExcluted(i);
    v = Alarm.getItemStatus(i, true);
    sprintf(buff, "%d|%d|%d|", i + 1, on, v);
    ADD_OFFS
    }
}
//-------------------------------------------------------------------
void convert4Output(int8_t val, char* buff)
{
  float f = val;
  f /= 4;
  char t[20];
  dtostrf (f, 2, 2, t);
  sprintf(buff, "%s|", t);
}
//-------------------------------------------------------------------
void fillCrono(char* buff, int len)
{
  int8_t dim = 3;
  if(needUpdateCrono)
    dim += DIM_CRONO;
  byte items[2];
  C_T.getItems(items, 2, 0);
  byte stat = items[0] | (C_T.getPinStatus() << 2);
  sprintf(buff, "%d|c|%d|%d|", dim, currDOW, stat);
  ADD_OFFS
  convert4Output(items[1], buff);
  if(!needUpdateCrono)
    return;
  ADD_OFFS
  needUpdateCrono = false;
  for(int8_t i = 0; i < DIM_CRONO && len > 0; ++i) {
    readCronoEESingle(items, &i, 1, currDOW);
    convert4Output(items[0], buff);
    ADD_OFFS
    }
}
//-------------------------------------------------------------------
void fillImpTemper(char* buff, int len)
{
  C_T.fillImpData(buff, len);
}
//-------------------------------------------------------------------
// type = 1 > temper, 2 -> rele, 4 -> umidit?, 8 -> alarm,
// 16 > crono, 32 > temper impostata, 64 > synctime, 128 > rain, 256 > motor, -1 -> all
byte fillData(char* buff, int len, uint16_t& type, const char* cmd)
{
  if(type & 1) {
    fillTemper(buff, len);
    type &= ~1;
    ADD_OFFS2
    }
  if(type & 2) {
    fillRele(buff, len);
    type &= ~2;
    ADD_OFFS2
    }
  if(type & 4) {
    fillHumid(buff, len);
    type &= ~4;
    ADD_OFFS2
    }
  if(type & 8) {
    fillAlarm(buff, len);
    type &= ~8;
    ADD_OFFS2
    }
  if(type & 16) {
    if(cmd)
      toggleCrono(cmd, type == 16);
    fillCrono(buff, len);
    type &= ~16;
    ADD_OFFS2
    }
  if(type & 32) {
    fillImpTemper(buff, len);
    type &= ~32;
    ADD_OFFS2
    }
  if(type & 64) {
    *buff = 0;
    type &= ~64;
    ADD_OFFS2
    }
  if(type & 256) {
    fillMotor(buff, len);
//    ADD_OFFS
    }
  return 0;
}
//------------------------------
// per il crono_termostato
//------------------------------
float getCurrTemper()
{
  // a scelta ... temperatura percepita, si potrebbero fare dei calcoli in base alla temperatura esterna ...
//  return dht.getHeatIndex();
  return dht.getTemperature();
}
//------------------------------
 // nel formato hh*100 + mm
int16_t getIxTime()
{
#if defined(USE_HW_TIME) || defined(USE_FULL_TIME)
  Time tm = rtc.getTime();
  int16_t res = tm.hour;
  res *= 100;
  res += tm.min;
  return res;
#else
  #if 0
  TimeElements tm = rtc.getTime();
  int16_t res = tm.Hour;
  res *= 100;
  res += tm.Minute;
  #else
  int16_t res = ::hour();
  res *= 100;
  res += ::minute();
  #endif
  return res;
#endif
}
//------------------------------
void SyncDateTime(const char* p)
{
  uint16_t par[7];
  DEBUG_DATE(Serial.println(p))
  int8_t num = G_fillParam(par, SIZE_A(par), p);
  if(num != SIZE_A(par))
    return;
  if(useDebug & D_BIT_DATE) {
    for(int8_t i = 0; i < num; ++i) {
      Serial.print(par[i]);
      Serial.print("-");
      }
    Serial.println();
    }
#if defined(USE_HW_TIME) || defined(USE_FULL_TIME)
  rtc.setDOW(par[0]);
  rtc.setDate(par[1], par[2], par[3]);
  rtc.setTime(par[4], par[5], par[6]);
#else
  rtc.setTime(par[4], par[5], par[6], par[1], par[2], par[3]);
#endif
}
//------------------------------
//------------------------------
#ifndef USE_FULL_TIME
  #ifndef USE_HW_TIME
//------------------------------
//-------------------------------------------------------------------
void myRtc::begin()
{
//  timeClient.setUpdateInterval(1000L * 60);
}
//-------------------------------------------------------------------
void myRtc::update(bool force)
{
  if(force)
    timeClient.forceUpdate();
  else
    timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  DEBUG_DATE(Serial.print(F("epochTime=")); Serial.println(epochTime))
   tmElements_t tm;
   breakTime((time_t)epochTime, tm);
  int monthDay = tm.Day;
  int currentMonth = tm.Month;
  int currentYear = tmYearToCalendar(tm.Year);
  if(currentYear < 2000)
    return;
  time_t offset = 3600;
  if(onDaylight(monthDay, currentMonth, timeClient.getDay()))
    offset += 3600;
  DEBUG_DATE(Serial.print(F("Offset:")); Serial.println(offset))
  timeClient.setTimeOffset(offset);
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

  setTime(currentHour, currentMinute, currentSecond, monthDay, currentMonth, currentYear);
  DEBUG_DATE(Serial.print(getDateStr()); Serial.print(F(" "));Serial.println(getTimeStr()))
  prev = millis();
}
//-------------------------------------------------------------------
void myRtc::loop()
{
  bool force = false;
  if(::year() < 2000) {
    if(millis() < prev + 1000L * 5)
      return;
    force = true;
  }
  else if(millis() < prev + 1000L * 60 * 60)
    return;
  update(force);
}
//------------------------------
String myRtc::getTimeStr()
{
  char buff[32];
  sprintf(buff, "%02d:%02d:%02d", ::hour(), ::minute(), ::second());
  return String(buff);
}
//------------------------------
String myRtc::getDateStr()
{
  char buff[32];
  sprintf(buff, "%02d-%02d-%04d", ::day(), ::month(), ::year());
  return String(buff);
}
//------------------------------
TimeElements myRtc::getTime()
{
  TimeElements te;
  time_t t = ::now();
  te.Second = ::second(t);
  te.Minute = ::minute(t);
  te.Hour = ::hour(t);
  te.Wday = ::weekday(t);
  te.Day = ::day(t);
  te.Month = ::month(t);
  te.Year = CalendarYrToTm(::year(t));
  return te;
}
//------------------------------
void myRtc::setTime(uint8_t h, uint8_t m, uint8_t s, uint8_t d, uint8_t month, uint16_t y)
{
  ::setTime(h, m, s, d, month, y);
}
  #endif
#endif
//------------------------------
