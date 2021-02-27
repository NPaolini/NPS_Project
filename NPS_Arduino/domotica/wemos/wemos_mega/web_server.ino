//------ web_server.ino ------------
//----------------------------------
#include <SPI.h>
#include <SD.h>
//----------------------------------
#include "domotica.h"
#include "man_tft.h"
//----------------------------------
class webServer : public IFace_web_server
{
  private:
    typedef IFace_web_server baseClass;
  public:
    webServer();
    virtual ~webServer();
    virtual void begin();
    virtual void beginLan(byte ip[], byte dns[], byte gateway[], byte mac[]);
    virtual void beginServer(int listenPort);

    virtual bool webServerLoop();
  private:
    bool parseReceived(int len);

    void sendTemper();
    void sendRele(const char* cmd);
    void sendHum();
    void sendCrono(const char* cmd);
    void sendImpTemper();
    void syncTime(const char* p);
    void sendAlarm(const char* cmd);
    void sendRain();
    void sendMotor(const char* cmd);
    void sendAll(const char* cmd);
    void sendGeneric(uint16_t type, const char* cmd = 0);

    bool checkValid(const char* u_p);

    int ListenPort;
    char Buff[1024];
    bool needCheckUser;
};
//----------------------------------
static webServer Server;
//----------------------------------
IFace_web_server* getServer() {
  return &Server;
}
//----------------------------------
webServer::webServer() : ListenPort(0), needCheckUser(true)
{
}
//----------------------------------
webServer::~webServer()
{
}
//----------------------------------
const char*formatIp(char* buff, byte ip[])
{
  sprintf(buff, "%d,%d,%d,%d,", ip[0], ip[1], ip[2], ip[3]);
  return buff;
}
//----------------------------------
void webServer::beginLan(byte ip[], byte dns[], byte gateway[], byte mac[])
{
  Serial3.print(F("cmd=ip:"));
  Serial3.print(formatIp(Buff, ip));
  Serial3.print(formatIp(Buff, dns));
  Serial3.print(formatIp(Buff, gateway));
  Serial3.print(F("#"));
  for(int i = 0; i < 50; ++i) {
    if(Serial3.available())
      break;
    delay(100);
    }
  int16_t len = Serial3.readBytesUntil('#', Buff, sizeof(Buff) - 1);
  Buff[len] = 0;
  DEBUG_WEB(Serial.print(F("begin Lan:")); Serial.println(Buff))
}
//----------------------------------
void webServer::beginServer(int port)
{
  ListenPort = port;
  sprintf(Buff, "cmd=port:%d#", port);
  Serial3.print(Buff);
  int8_t len = Serial3.readBytesUntil('#', Buff, sizeof(Buff));
  if(len)
    Buff[len] = 0;
  DEBUG_WEB(Serial.print(F("begin Server:")); Serial.println(Buff))
}
//----------------------------------
void webServer::begin()
{
  Serial3.begin(1000000);
  while(!Serial3);
}
//-------------------------------------------------------------------
bool webServer::webServerLoop()
{
  if (!ListenPort)
    return false;
  if(!Serial3.available())
    return false;
  char cmd[] = "cmd=";
  if(!Serial3.find(cmd, 4))
    return false;
  int len = Serial3.readBytesUntil('#', Buff, sizeof(Buff));
  Buff[len] = 0;
  DEBUG_WEB(Serial.print(F("web server loop: ")); Serial.println(Buff))
  return parseReceived(len);
}
//-------------------------------------------------------------------
// aggiustare in base alla ram disponibile
//#define SIZE_CHUNK 128
#define SIZE_CHUNK 1024
//-------------------------------------------------------------------
bool webServer::checkValid(const char* u_p)
{
  strcpy(g_Buff, u_p);
  char* pp = g_Buff;
  while(*pp) {
    if(',' == *pp) {
      *pp++ = 0;
      break;
      }
    ++pp;
    }

  infoSetup* isetup = getISetup();
  for (int8_t i = 0; i < MAX_USER; ++i) {
    if (!strcmp(isetup->users[i].u, g_Buff) && !strcmp(isetup->users[i].p, pp)) {
      Serial3.print(F("r=w:"));
      Serial3.print(i);
      Serial3.print(F("#"));
      return true;
    }
  }
  Serial3.print(F("r=w:-1#"));
  return false;
}
//-------------------------------------------------------------------
bool compare(const char* buff, const char* match)
{
  while (*match) {
    if (*match++ != *buff++)
      return false;
  }
  return true;
}
//---------------------------------------
extern bool isFileLog(const char* filename);
extern const char* getLogDir();
//---------------------------------------
File myFile;
//---------------------------------------
static bool openFile(const char* f)
{
  char path[32] = "site/";
  if (isFileLog(f))
    strcpy(path, getLogDir());
  strcat(path, f);
  DEBUG_WEB(Serial.print(F("file to send:")))
  DEBUG_WEB(Serial.print(path))

  if (!SD.exists(path)) {
    DEBUG_WEB(Serial.println(F(" -> Not Exists")))
    //    return false;
    }
  myFile = SD.open(path);
  if (!myFile) {
    DEBUG_WEB(Serial.println(F(" -> Open Error")))
    return false;
    }
  DEBUG_WEB(Serial.println(F(" -> Ok")))
  return true;
}
//---------------------------------------
static bool performSendFile(int32_t dim)
{
#define chunk g_Buff
#define sizeChunk sizeof(chunk)

  char resp[] = "ACK";
  if(!Serial3.find(resp, strlen(resp)))
    return false;
  int point = 0;
  DEBUG_WEB(Serial.print(F("Free memory: ")); Serial.println(FreeRam());Serial.print(F("Sending ")); Serial.print(dim); Serial.print(F(" bytes ")))
  while (dim > 0) {
    int len = min((unsigned int)dim, sizeChunk);
    len = myFile.read(chunk, len);
    Serial3.print(len);
    Serial3.print(F("#"));
    Serial3.write(chunk, len);
    if(!Serial3.find(resp, strlen(resp)))
      break;
    DEBUG_WEB(Serial.print(F(".")))
    dim -= len;
    }
  DEBUG_WEB(Serial.println(F(" Done ")))
  myFile.close();
  return true;
}
//-------------------------------------------------------------------
bool GS_set_file(const char* filename)
{
  Serial3.print(F("r=getFile:"));
  enableSD();
  if(!openFile(filename)) {
    Serial3.print(F("0#"));
    return false;
    }
  int32_t dim = myFile.size();
  Serial3.print(dim);
  Serial3.print(F("#"));
  return performSendFile(dim);
}
//-------------------------------------------------------------------
// a, b, c, d, [p], t, r, x, m, u, i, o
// rimosso p
// nuovi k -> cookie, w -> user e psw
bool webServer::parseReceived(int len)
{
  if(compare(Buff, "getFile")) {
    return GS_set_file(Buff + sizeof("getFile"));
    }
  bool result = false;
  char* p = Buff;
  switch (p[0]) {
    case 't': // richiesta temperature
      sendTemper();
      break;
    case 'r': // richiesta/invio stato relè (pulsante software + uscita attuale)
      if (':' == p[1])
        sendRele(p + 2);
      else
        sendRele(0);
      break;
    case 'x': // richiesta/invio stato relè (pulsante software + uscita attuale)
      if (':' == p[1])
        sendAlarm(p + 2);
      else
        sendAlarm(0);
      break;
    case 'm': // richiesta/invio stato relè (pulsante software + uscita attuale)
      if (':' == p[1])
        sendMotor(p + 2);
      else
        sendMotor(0);
      break;
    case 'u': // umidit�
      sendHum();
      break;
    case 'i': // temperatura impostata corrente
      sendImpTemper();
      break;
    case 'c': // richiesta/invio stato cronotermostato
      if (':' == p[1])
        sendCrono(p + 2);
      else
        sendCrono(0);
      break;
    case 'a': // invia tutto
      if (':' == p[1])
        sendAll(p + 2);
      else
        sendAll(0);
      break;
    case 'd':
      syncTime(p + 2);
      break;
    case 'b':
      sendRain();
      break;
    case 'o':
      sendHeaderRefresh();
      break;
    case 'w':
      checkValid(p + 2);
      break;
    }
  return result;
}
//---------------------------------------
static void sendHeaderRefresh()
{
  const char* msg = "r=<Refresh>#";
  Serial3.print(msg);
}
//---------------------------------------
/*
type = 1 > temper, 2 -> rele, 4 -> umidità , 8 -> alarm, 16 -> cronotermostato,
32 -> temperature impostata corrente, 64 -> imposta datetime, 128 -> pioggia,
256 -> motor, -1 -> all
*/
extern byte fillData(char* buff, int len, uint16_t& type, const char* cmd);
extern void switchRele(const char* cmd);
extern void switchMotor(const char* cmd);
extern void switchAlarm(const char* cmd);
extern void toggleCrono(const char* cmd, bool forceRefresh);
//-------------------------------------------------------------------
void webServer::sendGeneric(uint16_t type, const char* cmd)
{
  int16_t res = 0;
  do {
    res = fillData(g_Buff, sizeof(g_Buff), type, cmd);
    Serial3.print(F("r="));
    Serial3.print(g_Buff);
    Serial3.print(F("#"));
    DEBUG_WEB(Serial.println(g_Buff))
  } while (res);
}
//-------------------------------------------------------------------
void webServer::sendTemper()
{
  sendGeneric(1);
}
//-------------------------------------------------------------------
void webServer::sendRele(const char* cmd)
{
  if(cmd)
    switchRele(cmd);
  sendGeneric(2);
}
//-------------------------------------------------------------------
void webServer::sendHum()
{
  sendGeneric(4);
}
//-------------------------------------------------------------------
void webServer::sendAlarm(const char* cmd)
{
  if (cmd)
    switchAlarm(cmd);
  sendGeneric(8);
}
//-------------------------------------------------------------------
void webServer::sendCrono(const char* cmd)
{
  if (cmd)
    toggleCrono(cmd, true);
  sendGeneric(16);
}
//-------------------------------------------------------------------
void webServer::sendImpTemper()
{
  sendGeneric(32);
}
//-------------------------------------------------------------------
extern void SyncDateTime(const char* p);
//-------------------------------------------------------------------
void webServer::syncTime(const char* p)
{
  SyncDateTime(p);
  sendGeneric(64);
}
//-------------------------------------------------------------------
void webServer::sendRain()
{
  sendGeneric(128);
}
//-------------------------------------------------------------------
void webServer::sendMotor(const char* cmd)
{
  if (cmd)
    switchMotor(cmd);
  sendGeneric(256);
}
//-------------------------------------------------------------------
void webServer::sendAll(const char* cmd)
{
  sendGeneric((uint16_t)-1, cmd);
}
