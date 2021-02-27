//------ man_log.ino ---------------------------
#include "man_log.h"
//--------------------------------------------
man_log::man_log(eFile e) : eF(e) {}
//--------------------------------------------
man_log::~man_log() {}
//--------------------------------------------
#define WRITE_T(t) if(t && *t) { file.write(t, strlen(t)); file.write('\t'); DEBUG_LOG(Serial.print(t); Serial.print('\t')) }
#define WRITE_S(t) if(t && *t) { DEBUG_LOG(Serial.print(t); Serial.print('\t')) }
//--------------------------------------------
void logRow(File& file, const char* buff)
{
  WRITE_T(buff)
}
//--------------------------------------------
  #define WRITE_LOG(t) logRow(file, t);
//--------------------------------------------
//#define CC(a) ((const char*)(a))
#define CC(a) a
//--------------------------------------------
#define DEF_DIR_LOG CC("log")

#define DEF_EMAIL_LOG CC("emailog.txt")
#define DEF_ACCESS_LOG CC("access.txt")
#define DEF_ALARM_LOG CC("alarm.txt")

#define DEF_PATH_EMAIL_LOG DEF_DIR_LOG "/" DEF_EMAIL_LOG
#define DEF_PATH_ACCESS_LOG DEF_DIR_LOG "/" DEF_ACCESS_LOG
#define DEF_PATH_ALARM_LOG DEF_DIR_LOG "/" DEF_ALARM_LOG

#define OFFS_NAME 4
//--------------------------------------------
bool isFileLog(const char* filename)
{
  return !strcmp(filename, DEF_EMAIL_LOG) || !strcmp(filename, DEF_ACCESS_LOG) || !strcmp(filename, DEF_ALARM_LOG);
}
//--------------------------------------------
const char* getLogDir() { return DEF_DIR_LOG CC("/"); }
//--------------------------------------------
void dateTime(uint16_t* date, uint16_t* time)
{
#ifdef USE_HW_TIME
  Time t = rtc.getTime();
// return date using FAT_DATE macro to format fields
  *date = FAT_DATE(t.year, t.mon, t.date);

 // return time using FAT_TIME macro to format fields
  *time = FAT_TIME(t.hour, t.min, t.sec);
#else
  TimeElements t = rtc.getTime();
  *date = FAT_DATE(t.Year, t.Month, t.Day);
  *time = FAT_TIME(t.Hour, t.Minute, t.Second);
#endif
}
//--------------------------------------------
const char* man_log::getFilename()
{
  switch(eF) {
    case eL_Email:
      return DEF_PATH_EMAIL_LOG;
    case eL_Access:
      return DEF_PATH_ACCESS_LOG;
    case eL_Alarm:
      return DEF_PATH_ALARM_LOG;
    default:
      return 0;
    }
}
//--------------------------------------------
void man_log::logSend(const char* info[], uint8_t num)
{
  enableSD();
  SdFile::dateTimeCallback(dateTime);
  const char* fn = getFilename();
  if(!fn)
    return;
  File file = SD.open(fn, FILE_WRITE);

  static char buff[32];
  getDateTimeStr(buff, sizeof(buff));
  if(!file) {
    DEBUG_LOG(Serial.print(F("error open file:")))
    DEBUG_LOG(Serial.println(fn + OFFS_NAME))
    WRITE_S(buff)
    for(uint8_t i = 0; i < num; ++i)
      WRITE_S(info[i])
    DEBUG_LOG(Serial.println())
    }
  else {
    file.seek(file.size());
    WRITE_LOG(buff)
    for(uint8_t i = 0; i < num; ++i)
      WRITE_LOG(info[i])
    file.write("\r\n", 2);
    DEBUG_LOG(Serial.println())
    file.close();
    }
  SdFile::dateTimeCallbackCancel();
}
//--------------------------------------------
void man_log::deleteLog()
{
  enableSD();
  const char* fn = getFilename();
  if(!fn)
    return;
  DEBUG_LOG(Serial.print(F("File ")))
  DEBUG_LOG(Serial.print(fn + OFFS_NAME))
  if(SD.remove(fn))
    DEBUG_LOG(Serial.println(F(" rimosso con successo")))
  else
    DEBUG_LOG(Serial.println(F(" non eliminato")))
}
//--------------------------------------------
void man_log::printLog()
{
  Serial.print(F("Lista Log "));
  if(eL_Email == eF)
    Serial.println(F("invio email"));
  else if(eL_Access == eF)
    Serial.println(F("accessi"));
  else if(eL_Alarm == eF)
    Serial.println(F("allarmi"));
  enableSD();
  const char* fn = getFilename();
  if(!fn)
    return;

  File file = SD.open(fn);
  if(!file) {
    Serial.println(F("Impossibile aprire il file"));
    return;
    }
  while(true) {
    int readed = file.read(g_Buff, sizeof(g_Buff));
    if(!readed)
      break;
    Serial.write(g_Buff, readed);
    }
  Serial.println();
  enableSD();
  file.close();
}
