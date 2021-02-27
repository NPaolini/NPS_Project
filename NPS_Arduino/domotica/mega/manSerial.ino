#include "domotica.h"
//---------------------------------------
class IMan_serial : public man_serial
{
  private:
    typedef man_serial baseClass;
  public:
    IMan_serial() {}
    ~IMan_serial() {}
    void begin();
    bool loop();
    void printIP(uint16_t port);
    void printParam(uint16_t* par, int8_t num);
    void printEmailData();
};
//---------------------------------------
IMan_serial IManSerial;
man_serial* getManSerial() { return &IManSerial; }
//---------------------------------------
void GS_printInfoIP(IPAddress ip, uint16_t port, bool nl)
{
  char t[32];
  if(port)
    sprintf(t, "%d.%d.%d.%d:%d", ip[0], ip[1], ip[2], ip[3], port);
  else
    sprintf(t, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
  if(nl)
    Serial.println(t);
  else
    Serial.print(t);
}
//--------------------------------------------------
void IMan_serial::printIP(uint16_t port)
{
  GS_printInfoIP(Ethernet.localIP(), port, true);
  GS_printInfoIP(Ethernet.subnetMask(), 0, true);
  GS_printInfoIP(Ethernet.gatewayIP(), 0, true);
  GS_printInfoIP(Ethernet.dnsServerIP(), 0, true);
}
//--------------------------------------------------
void IMan_serial::begin()
{
  Serial.begin(115200);
  while(!Serial);
}
//-------------------------------------------------------------------
void IMan_serial::printParam(uint16_t* par, int8_t num)
{
  for(int8_t i = 0; i < num; ++i) {
    Serial.print(par[i]);
    Serial.print("-");
    }
  Serial.println();
}
//-------------------------------------------------------------------
int8_t GS_loadParam(uint16_t* param, int8_t num, bool checkDow = false)
{
  int len = Serial.readBytesUntil('#', g_Buff, sizeof(g_Buff));
  g_Buff[len] = 0;
  DEBUG_SETUP(Serial.println(g_Buff))
  const char* p = g_Buff;
  if(checkDow)
    p = parseDOW(g_Buff);
  int8_t res = G_fillParam(param, num, p);
  DEBUG_SETUP(getManSerial()->printParam(param, res))
  return res;
}
//-------------------------------------------------------------------
//-------------------------------------------------------------------
#if defined(USE_HW_TIME) || defined(USE_FULL_TIME)
//-------------------------------------------------------------------
void GS_setRCTDate()
{
  // es. cmd=date:3,6,11,2019#
  // N.B: 3 -> giorno della settimana, 1 -> lunedì, 7 -> domenica
  uint16_t par[4] = {};
  GS_loadParam(par, 4);
  rtc.setDOW(par[0]);
  rtc.setDate(par[1], par[2], par[3]);

}
//-------------------------------------------------------------------
void GS_setRCTTime()
{
  // es. cmd=time:14,46,00#
  uint16_t par[3] = {};
  GS_loadParam(par, 3);
  rtc.setTime(par[0], par[1], par[2]);
}
//-------------------------------------------------------------------
void GS_setRCTTimeDate()
{
  // es. cmd=timedate:14,46,00,dow,25,12,2019#
  uint16_t par[7] = {};
  GS_loadParam(par, SIZE_A(par));
  rtc.setTime(par[0], par[1], par[2]);
  rtc.setDOW(par[3]);
  rtc.setDate(par[4], par[5], par[6]);
}
//-------------------------------------------------------------------
#else
//-------------------------------------------------------------------
void GS_setRCTDate()
{
  // es. cmd=date:3,6,11,2019#
  // N.B: 3 -> giorno della settimana, 1 -> lunedì, 7 -> domenica
//  uint8_t h, uint8_t m, uint8_t s, uint8_t d, uint8_t month, uint16_t y
  uint16_t par[4] = {};
  GS_loadParam(par, SIZE_A(par));
  time_t t = ::now();
  rtc.setTime(::hour(t), ::minute(t), ::second(t), par[1], par[2], par[3]);
}
//-------------------------------------------------------------------
void GS_setRCTTime()
{
  // es. cmd=time:14,46,00#
  uint16_t par[3] = {};
  GS_loadParam(par, 3);
  time_t t = ::now();
  rtc.setTime(par[0], par[1], par[2], ::day(t), ::month(t), ::year(t));
}
//-------------------------------------------------------------------
void GS_setRCTTimeDate()
{
  // es. cmd=timedate:14,46,00,dow,25,12,2019#
  uint16_t par[7] = {};
  GS_loadParam(par, SIZE_A(par));
  rtc.setTime(par[0], par[1], par[2], par[4], par[5], par[6]);
}
#endif
//-------------------------------------------------------------------
void GS_getRCTDate()
{
  Serial.print(F("r=getdate:"));
  Serial.print(rtc.getDateStr());
  Serial.print(F(","));
  Serial.print(rtc.getTimeStr());
  Serial.println(F("#"));
}
//-------------------------------------------------------------------
template <typename T, typename S> void copy(T* t, S* s, int len)
{
  for(int8_t i = 0; i < len; ++i)
    t[i] = s[i];
}
//-------------------------------------------------------------------
void GS_setIP()
{
  // es. cmd=ip:192,168,1,98,192,168,1,254,192,168,1,254#
  // [ip][dns][gateway]
  uint16_t par[12] = {};
  GS_loadParam(par, 12);
  copy(ISetup.ip, par, 4);
  copy(ISetup.dns, par + 4, 4);
  copy(ISetup.gateway, par + 8, 4);
  server->beginLan(ISetup.ip, ISetup.dns, ISetup.gateway, mac);
  saveSetup();
  DEBUG_IP(getManSerial()->printIP(ISetup.port))
}
//-------------------------------------------------------------------
void GS_getIP()
{
  // es. r=getip:192.168.1.98,192.168.1.254,192.168.1.254,80#
  // [ip],[dns],[gateway],[port]#
  Serial.print(F("r=getip:"));
  GS_printInfoIP(Ethernet.localIP(), 0, false);
  Serial.print(F(","));
  GS_printInfoIP(Ethernet.dnsServerIP(), 0, false);
  Serial.print(F(","));
  GS_printInfoIP(Ethernet.gatewayIP(), 0, false);
  Serial.print(F(","));
  Serial.print(ISetup.port);
  Serial.println(F("#"));
}
//-------------------------------------------------------------------
void GS_setPort()
{
  uint16_t par[1] = {};
  GS_loadParam(par, 1);
  ISetup.port = par[0];
  server->beginServer(ISetup.port);
  saveSetup();
}
//-------------------------------------------------------------------
#define MAX_BUFF_USER (RFID_DIM_CODE + DIM_USER)
//-------------------------------------------------------------------
bool readToBuff(char* target, uint8_t dim)
{
  int len = Serial.readBytesUntil(',', g_Buff, sizeof(g_Buff));
  if(!len)
    return false;
  if(len > dim)
    len = dim;
  g_Buff[len] = 0;
  char* p = g_Buff;
  // elimina gli spazi iniziali
  while(*p && *p <= ' ') {
    ++p;
    --len;
    }
  // elimina gli spazi finali
  while(len > 0) {
    if(p[len - 1] > ' ') {
      p[len] = 0;
      break;
      }
    --len;
    }
  if(!len)
    target[0] = 0;
  else
    strcpy(target, p);
  return true;
}
//-------------------------------------------------------------------
bool printUser(uint8_t id)
{
  Serial.print(id);
  Serial.print(F(","));
  Serial.print(ISetup.users[id].u);
  Serial.print(F(","));
  Serial.print(ISetup.users[id].p);
  Serial.print(F(","));
  Serial.print(ISetup.users[id].rfid_code);
  return true;
}
//-------------------------------------------------------------------
void GS_getUser()
{
// indice,user,psw,codice_rfid,....#
// r=user:0,pippo,password,nps1123581321001,1,pluto,psw2,nps2358132134001,2, , , ,#
//  char buff[MAX_BUFF_USER] = {0};
  Serial.print(F("r=getuser:"));
  bool first = true;
  for(uint8_t i = 0; i < MAX_USER; ++i) {
    if(!first)
      Serial.print(F(","));
    if(printUser(i) && first)
      first = false;
    }
  Serial.println(F("#"));
}
//-------------------------------------------------------------------
void GS_setUser()
{
// es. per impostare i primi due utenti, indice parte da zero fino a MAX_USER-1
// indice,user,psw,codice_rfid,....#
// cmd=user:0,pippo,password,nps1123581321001,1,pluto,psw2,nps2358132134001,#
// N.B. poiche' ogni pezzo viene letto fino alla virgola, ce la vuole anche dopo l'ultimo pezzo, prima del #
//  char buff[MAX_BUFF_USER] = {0};
  DEBUG_USER(Serial.print(F("Free memory: ")))
  DEBUG_USER(Serial.println(FreeRam()))
  bool first = true;
  do {
    if(first) {
      while(true)
        if(-1 != Serial.peek())
          break;
      first = false;
      }
    if('#' == Serial.peek())
      break;
    char id[10];
    uint8_t lid = Serial.readBytesUntil(',', id, sizeof(id));
    if(!lid)
      break;
    id[lid] = 0;
    lid = atoi(id);
    if(lid >= MAX_USER)
      break;
    if(!readToBuff(ISetup.users[lid].u, DIM_USER))
      break;
    if(!readToBuff(ISetup.users[lid].p, DIM_PASSWORD))
      break;
    if(!readToBuff(ISetup.users[lid].rfid_code, RFID_DIM_CODE))
      break;
    if(useDebug & D_BIT_USER) {
      Serial.print(ISetup.users[lid].u);
      Serial.print(F(" - "));
      Serial.print(ISetup.users[lid].p);
      Serial.print(F(" - "));
      Serial.println(ISetup.users[lid].rfid_code);
      }
    } while(true);
  saveSetup();
}

//-------------------------------------------------------------------
int GS_copyEmailData(char* t, const char* s)
{
  int8_t i = 0;
  for(; i < MAX_DIM_EMAIL - 1; ++i) {
    if(',' == *s || '#' == *s || 0 == *s)
      break;
    *t++ = *s++;
    }
  *t = 0;
  return i + 1;
}
//-------------------------------------------------------------------
void IMan_serial::printEmailData()
{
  Serial.print(F("smtp:"));
  Serial.println(ISetup.smtp);
  Serial.print(F("port:"));
  Serial.println(ISetup.email_port);
  Serial.print(F("from:"));
  Serial.println(ISetup.from);
  Serial.print(F("to:"));
  Serial.println(ISetup.to);
  Serial.print(F("user:"));
  Serial.println(ISetup.user);
  Serial.print(F("psw:"));
  Serial.println(ISetup.psw);
}
//-------------------------------------------------------------------
void GS_getEmail()
{
  Serial.print(F("r=getemail:s,"));
  Serial.print(ISetup.smtp);
  Serial.print(F(",p,"));
  Serial.print(ISetup.email_port);
  Serial.print(F(",f,"));
  Serial.print(ISetup.from);
  Serial.print(F(",t,"));
  Serial.print(ISetup.to);
  Serial.print(F(",u,"));
  Serial.print(ISetup.user);
  Serial.print(F(",w,"));
  Serial.print(ISetup.psw);
  Serial.println(F("#"));
}
//-------------------------------------------------------------------
#define MAX_BUFF_EMAIL MAX_DIM_EMAIL
//-------------------------------------------------------------------
void GS_setEmail()
{
  bool first = true;
  do {
    if(first) {
      while(true)
        if(-1 != Serial.peek())
          break;
      first = false;
      }
    if('#' == Serial.peek())
      break;
    char cmd[10];
    int lcmd = Serial.readBytesUntil(',', cmd, sizeof(cmd));
    if(!lcmd)
      break;
    int len = Serial.readBytesUntil(',', g_Buff, sizeof(g_Buff));
    if(!len)
      break;
    g_Buff[len] = 0;
    switch(*cmd) {
      case 's':
        GS_copyEmailData(ISetup.smtp, g_Buff);
        break;
      case 'p':
        ISetup.email_port = atoi(g_Buff);
        break;
      case 'f':
        GS_copyEmailData(ISetup.from, g_Buff);
        break;
      case 't':
        GS_copyEmailData(ISetup.to, g_Buff);
        break;
      case 'u':
        GS_copyEmailData(ISetup.user, g_Buff);
        break;
      case 'w':
        GS_copyEmailData(ISetup.psw, g_Buff);
        break;
      }
    } while(true);
  saveSetup();
  DEBUG_EMAIL(getManSerial()->printEmailData())
}
//-------------------------------------------------------------------
void GS_setDebugLevel()
{
  int len = Serial.readBytesUntil('#', g_Buff, sizeof(g_Buff));
  if(!len)
    return;
  g_Buff[len] = 0;
  useDebug = atoi(g_Buff);
  ISetup.useDebug = useDebug;
  saveSetup();
  Serial.print(F("Debug level: "));
  Serial.println(useDebug);
}
//-------------------------------------------------------------------
void GS_setDebugLevelByBits()
{
  uint16_t par[32] = {};
  uint8_t num = GS_loadParam(par, 32);
  useDebug = 0;
  for(uint8_t i = 0; i < num; ++i)
    useDebug |= (uint32_t)1 << par[i];
  ISetup.useDebug = useDebug;
  saveSetup();
  Serial.print(F("Debug level: "));
  Serial.println(useDebug);
}
//-------------------------------------------------------------------
void GS_getDebugLevel()
{
  Serial.print(F("r=getdebug:"));
  Serial.print(useDebug);
  Serial.println(F("#"));
}
//-------------------------------------------------------------------
void GS_setSD()
{
  uint16_t par[1] = {};
  GS_loadParam(par, 1);
  ISetup.sd_on_use = par[0];
  if(!ISetup.sd_on_use || ISetup.sd_on_use > 2)
    ISetup.sd_on_use = 1;
  saveSetup();
  reloadSDinfo(true);
}
//-------------------------------------------------------------------
void GS_setTiming()
{
  uint16_t par[5] = {};
  GS_loadParam(par, SIZE_A(par));
  ISetup.secAlarmTiming1 = par[0];
  ISetup.secAlarmTiming2 = par[1];
  ISetup.secAlarmTiming3 = par[2];
  ISetup.secDelayAlarm = par[3];
  ISetup.sec4endRun = par[4];
  saveSetup();
}
//-------------------------------------------------------------------
void GS_getTiming()
{
  Serial.print(F("r=gettiming:"));
  Serial.print(ISetup.secAlarmTiming1);

  uint16_t arr[] = { ISetup.secAlarmTiming2, ISetup.secAlarmTiming3, ISetup.secDelayAlarm, ISetup.sec4endRun };
  for(byte i = 0; i < SIZE_A(arr); ++i) {
    Serial.print(F(","));
    Serial.print(arr[i]);
    }
  Serial.println(F("#"));
}
//-------------------------------------------------------------------
void GS_show_rem_log(uint8_t c)
{
  char buff[4];
  int len = Serial.readBytesUntil('#', buff, sizeof(buff));
  if(!len)
    return;
  buff[len] = 0;
  man_log::eFile t = (man_log::eFile)atoi(buff);
  man_log ml(t);
  if(1 == c)
    ml.printLog();
  else if(2 == c)
    ml.deleteLog();
}
//-------------------------------------------------------------------
#define DIM_RESTORE_1 (sizeof(ISetup) + sizeof(stdEE_data))
//-------------------------------------------------------------------
void GS_backup()
{
  Serial.print(F("r=backup:"));
  uint16_t dim = DIM_RESTORE_1;
  Serial.print(dim);
  Serial.print(F(","));
  Serial.print(TOT_CRONO);
  Serial.print(F("#"));

  for(uint16_t i = 0; i < dim; ++i)
    Serial.write(EEPROM.read(addr_EE_data + i));
  for(uint16_t i = 0; i < TOT_CRONO; ++i)
    Serial.write(EEPROM.read(addr_EE_crono + i));
}
//-------------------------------------------------------------------
void(*Restart)(void) = 0;
//-------------------------------------------------------------------
#define performRead(buff, dim)  (dim == Serial.readBytes(buff, dim))
//-------------------------------------------------------------------
uint8_t GS_PerformRestore()
{
  uint16_t par[2] = {};
  uint8_t num = GS_loadParam(par, 2);

  char buff[DIM_RESTORE_1] = {0};
  uint16_t dim = min(par[0], DIM_RESTORE_1);

  Serial.print(F("ACK"));
  if(!performRead(buff, dim))
    return 2;

  for(int i = 0; i < DIM_HEADER; ++i) {
    if(buff[i] != stdEE_data[i])
      return 3;
    }
  // se ci sono più dati ... svuota l'eccesso
  if(par[0] > DIM_RESTORE_1)
    performRead(g_Buff, par[0] - DIM_RESTORE_1);

  Serial.print(F("ACK"));
  uint16_t dim2 = min(par[1], TOT_CRONO);
  if(!performRead(g_Buff, dim2))
    return 5;
  memcpy((char*)&ISetup, buff + sizeof(stdEE_data), dim);
  saveSetup();

  for(int16_t i = 0; i < dim2; ++i) {
    EEPROM.update(addr_EE_crono + i, g_Buff[i]);
    }
  return 0;
}
//-------------------------------------------------------------------
void GS_restore()
{
  Serial.setTimeout(1000L * 10);
  uint8_t result = GS_PerformRestore();
  Serial.setTimeout(1000L);
  Serial.print("r=restore:");
  switch(result) {
    case 0:
      Serial.println(F("Ok#"));
      break;
    case 1:
      Serial.println(F("Failed - No Read From Serial!#"));
      break;
    case 2:
      Serial.println(F("Failed - No read Header!#"));
      break;
    case 3:
      Serial.println(F("Failed - No match Header!#"));
      break;
    case 4:
      Serial.println(F("Failed - No read Setup!#"));
      break;
    case 5:
      Serial.println(F("Failed - No read Crono!#"));
      break;
    default:
      Serial.println(F("Failed - Unknow error#"));
      break;
    }
  if(!result) {
    delay(1000);
    Restart();
    }
}
//-------------------------------------------------------------------
#define DEF_DIR_SITE "site/"
//-------------------------------------------------------------------
void GS_show_sitelist()
{
  Serial.println(F("r=site_list:"));
  enaSd();
  File dir = SD.open(DEF_DIR_SITE);
  if(!dir) {
    Serial.println(F("Err open dir#"));
    return;
    }
  dir.rewindDirectory();
  do {
    File file = dir.openNextFile();
    if(!file)
      break;
    if(!file.isDirectory()) {
      uint32_t sz = file.size();
      Serial.print(file.name());
      Serial.print(F(","));
      char t[12];
      char format[4];
      format[0] = '%';
      format[1] = 'l';
      format[2] = 'd';
      format[3] = 0;
      sprintf(t, format, sz);
      Serial.println(t);
      }
    file.close();
  } while(true);
  dir.close();
  Serial.println("#");
}
//-------------------------------------------------------------------
int32_t GS_get_site_filename(char* buff, int32_t dim)
{
  strcpy(buff, DEF_DIR_SITE);
  char* p = buff + strlen(buff);
  int len = Serial.readBytesUntil(',', p, dim - (p - buff));
  if(!len)
    return -1;
  p[len] = 0;
  len = Serial.readBytesUntil('#', g_Buff, sizeof(g_Buff));
  g_Buff[len] = 0;
  dim = atol(g_Buff);
  if(dim <= 0)
    return -2;
  return dim;
}
//-------------------------------------------------------------------
int GS_perform_set_site_file()
// cmd=set_site_file:index.htm,15503#1024#.....
// nome_file,dim#dim_chunk#binary...dim_chunk#binary...
{
  char filename[24];
  int32_t dim = GS_get_site_filename(filename, sizeof(filename));
  if(dim < 0)
    return dim;
  enaSd();
  SdFile::dateTimeCallback(dateTime);
  if(currSD->exists(filename))
    currSD->remove(filename);
  File file = currSD->open(filename, FILE_WRITE);
  if(!file)
    return -3;
  Serial.print(F("ACK"));
  while(dim > 0) {
    int len = Serial.readBytesUntil('#', g_Buff, sizeof(g_Buff));
    g_Buff[len] = 0;
    int lenChunk = atoi(g_Buff);
    if(lenChunk <= 0)
      break;
    len = min((unsigned int)lenChunk, sizeof(g_Buff));
    while(lenChunk > 0) {
      len = Serial.readBytes(g_Buff, len);
      if(len <= 0)
        break;
      file.write(g_Buff, len);
      dim -= len;
      lenChunk -= len;
      }
    Serial.print(F("ACK"));
    }
  file.close();
  SdFile::dateTimeCallbackCancel();
  return 0;
}
//-------------------------------------------------------------------
void GS_set_site_file()
{
  int result = GS_perform_set_site_file();
  Serial.print(F("r=set_site_file:"));

  switch(result) {
    case 0:
      Serial.println(F("Ok#"));
      break;
    case -1:
      Serial.println(F("No filename#"));
      break;
    case -2:
      Serial.println(F("No dim#"));
      break;
    case -3:
      Serial.println(F("No open file#"));
      break;
    default:
      Serial.println(F("Unknow#"));
      break;
    }
}
//-------------------------------------------------------------------
char* only_name(char* filename)
{
  char* p = filename;
  while(*p) {
    if('/' == *p || '\\' == *p)
      return p + 1;
    ++p;
    }
  return filename;
}
//-------------------------------------------------------------------
int GS_perform_get_site_file(char* filename)
{
  enaSd();
  SdFile::dateTimeCallback(dateTime);
  File file = currSD->open(filename);
  if(!file) {
    return - 2;
    }
  uint32_t dim = file.size();
  Serial.print(dim);
  Serial.print(F("#"));
  do {
    uint32_t n = min(sizeof(g_Buff), dim);
    n = file.read(g_Buff, n);
    if(!n)
      break;
    Serial.write(g_Buff, n);
    dim -= n;
    } while(dim);
  file.close();
  SdFile::dateTimeCallbackCancel();
  return 0;
}
//-------------------------------------------------------------------
void GS_get_site_file()
// cmd=get_site_file:index.htm,15503#
// nome_file,dim#
{
  Serial.print(F("r=get_site_file:"));
  char filename[24];
  int result = GS_get_site_filename(filename, sizeof(filename));
  if(-1 == result) {
    Serial.println(F("No filename#"));
    return;
    }
  Serial.print(only_name(filename));
  Serial.print(F(","));
  result = GS_perform_get_site_file(filename);

  switch(result) {
    case 0:
      break;
    case -2:
      Serial.println(F("No open file#"));
      break;
    default:
      Serial.println(F("Unknow#"));
      break;
    }
}
//-------------------------------------------------------------------
bool GS_cmdBySerial()
{
  // cmd=xxx:yyyy# -> # == chiusura comando
  if(!Serial.available())
    return false;
  char cmd[] = "cmd=";
  if(!Serial.find(cmd, 4))
    return false;
  char buff[64];
  int len = Serial.readBytesUntil(':', buff, sizeof(buff));
  if(!len)
    return false;
  buff[len] = 0;
  if(!strcmp(buff, "date"))
    GS_setRCTDate();
  else if(!strcmp(buff, "time"))
    GS_setRCTTime();
  else if(!strcmp(buff, "ip"))
    GS_setIP();
  else if(!strcmp(buff, "port"))
    GS_setPort();
  else if(!strcmp(buff, "email"))
    GS_setEmail();
  else if(!strcmp(buff, "user"))
    GS_setUser();
  else if(!strcmp(buff, "debug"))
    GS_setDebugLevel();
  else if(!strcmp(buff, "debug-bits"))
    GS_setDebugLevelByBits();
  else if(!strcmp(buff, "remlog")) {
    GS_show_rem_log(2);
    }
  else if(!strcmp(buff, "sd"))
    GS_setSD();
  else if(!strcmp(buff, "timing"))
    GS_setTiming();
  else if(!strcmp(buff, "showlog")) {
    GS_show_rem_log(1);
    }
  else if(!strcmp(buff, "site_list")) {
    GS_show_sitelist();
    }
  else if(!strcmp(buff, "set_site_file")) {
    GS_set_site_file();
    }
  else if(!strcmp(buff, "get_site_file")) {
    GS_get_site_file();
    }

  else if(!strcmp(buff, "getdate"))
    GS_getRCTDate();
  else if(!strcmp(buff, "getip"))
    GS_getIP();
  else if(!strcmp(buff, "getemail"))
    GS_getEmail();
  else if(!strcmp(buff, "getuser"))
    GS_getUser();
  else if(!strcmp(buff, "getdebug"))
    GS_getDebugLevel();
  else if(!strcmp(buff, "gettiming"))
    GS_getTiming();
  else if(!strcmp(buff, "backup"))
    GS_backup();
  else if(!strcmp(buff, "restore"))
    GS_restore();

  return true;
}
//-------------------------------------------------------------------
bool IMan_serial::loop()
{
  return GS_cmdBySerial();
}
//-------------------------------------------------------------------
