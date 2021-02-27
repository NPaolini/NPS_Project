#include <SPI.h>
#include <SD.h>

#include "domotica.h"
#include "man_tft.h"
#include "utility/w5100.h"

static File myFile;
static bool needSendSID = false;
//----------------------------------
class webServer : public IFace_web_server
{
  private:
    typedef IFace_web_server baseClass;
  public:
    webServer();
    virtual ~webServer();
    virtual void beginLan(byte ip[], byte dns[], byte gateway[], byte mac[]);
    virtual void beginServer(int listenPort);

    virtual bool webServerLoop();
  private:
    EthernetServer server;
    void sendFileNoCheck(EthernetClient& client, const char* f);
    void sendFile(EthernetClient& client, const char* f);
    bool parseReceived(EthernetClient& client, int len);

    void sendTemper(EthernetClient& client);
    void sendRele(EthernetClient& client, const char* cmd);
    void sendHum(EthernetClient& client);
    void sendCrono(EthernetClient& client, const char* cmd);
    void sendImpTemper(EthernetClient& client);
    void syncTime(EthernetClient& client, const char* p);
    void sendAlarm(EthernetClient& client, const char* cmd);
    void sendMotor(EthernetClient& client, const char* cmd);
    void sendAll(EthernetClient& client, const char* cmd);
    void sendGeneric(EthernetClient& client, uint16_t type, const char* cmd = 0);

    bool checkValid(const char* u, const char* p);
    bool checkLogin(EthernetClient& client);

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
void printInfoSD(bool success)
{
  infoSetup* isetup = getISetup();
  Serial.print("Card in use: ");
  Serial.print(isetup->sd_on_use);
  Serial.print(" - Loaded: ");
  Serial.println(success ? "Success" : "Failed");
}
//----------------------------------
void reloadSDinfo(bool dump = false)
{
  infoSetup* isetup = getISetup();
  pin_t pin = PIN_SD_CARD1;
  if (2 == isetup->sd_on_use) {
    enaSd = enableSD2;
    currSD = &SD2;
    pin = PIN_SD_CARD2;
  }
  else {
    enaSd = enableSD;
    currSD = &SD;
  }
  enaSd();
  bool success = currSD->begin(pin);
  if (dump)
    DEBUG_SETUP(printInfoSD(success))
  }
//----------------------------------
webServer::webServer() : server(80), ListenPort(0), needCheckUser(true)
{
}
//----------------------------------
webServer::~webServer()
{
}
//----------------------------------
void webServer::beginLan(byte ip[], byte dns[], byte gateway[], byte mac[])
{
  reloadSDinfo(true);
  enableLan();
  Ethernet.begin(mac, ip, dns, gateway);
}
//----------------------------------
void webServer::beginServer(int port)
{
  enableLan();
  ListenPort = port;
  server = EthernetServer(port);
  server.begin();
}
//-------------------------------------------------------------------
void getParam(const char* buff, char* target, const char* match, int16_t dim)
{
  target[0] = 0;
  const char* p = strstr(buff, match);
  if (p) {
    p += strlen(match) + 1;
    while (*p && *p != '&' && --dim > 0)
      *target++ = *p++;
    *target = 0;
  }
}
//----------------------------------
bool webServer::webServerLoop()
{
  if (!ListenPort)
    return false;
  enableLan();
  EthernetClient client = server.available();
  if (!client)
    return false;
  bool result = false;
  if (client.connected()) {
    if (client.find(' ')) { // GET /fn HTTP/1.1
      int l = client.readBytesUntil(' ', Buff, sizeof(Buff));
      Buff[l] = 0;
      result = parseReceived(client, l);
    }
  }
  enableLan();
  client.stop();
  return result;
}
//-------------------------------------------------------------------
#define HOME_PAGE "index.htm"
#define LOGIN_PAGE "login.htm"
//-------------------------------------------------------------------
// aggiustare in base alla ram disponibile
//#define SIZE_CHUNK 128
#define SIZE_CHUNK 1024
//-------------------------------------------------------------------
bool webServer::checkValid(const char* u, const char* p)
{
  currSID[0] = 0;
  infoSetup* isetup = getISetup();
  for (int8_t i = 0; i < MAX_USER; ++i) {
    if (!strcmp(isetup->users[i].u, u) && !strcmp(isetup->users[i].p, p)) {
      uint8_t pos = Cookie.make(i);
      Cookie.get(currSID, pos);
      DEBUG_WEB(Serial.print(F("New SID:")))
      DEBUG_WEB(Serial.println(currSID))
      needSendSID = true;
      return true;
    }
  }
  return false;
}
//-------------------------------------------------------------------
bool webServer::checkLogin(EthernetClient& client)
{
  currSID[0] = 0;
  int16_t len = 0;
  int8_t toFind = 2;
  bool result = false;
  do {
    len = client.readBytesUntil('\n', g_Buff, sizeof(g_Buff));
    if (!len)
      break;
    g_Buff[len] = 0;
    char u[DIM_USER + 2];
    getParam(g_Buff, u, "Cookie: SID", SIZE_A(u));
    if (u[0]) {
      u[DIM_COOKIE] = 0;
      strcpy(currSID, u);
      toFind = needCheckUser;
      }
    getParam(g_Buff, u, "UserName", SIZE_A(u));
    if (u[0]) {
      char p[DIM_PASSWORD + 2];
      getParam(g_Buff, p, "UserPass", SIZE_A(p));
      if (p[0]) {
        toFind = 0;
        if (checkValid(u, p)) {
          DEBUG_WEB(Serial.println(F("check login: valid")))
          result = true;
          needCheckUser = false;
          }
        }
      }
    } while (toFind);
  return result;
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
//-------------------------------------------------------------------
bool webServer::parseReceived(EthernetClient& client, int len)
{
  if (checkLogin(client)) {
    sendFileNoCheck(client, HOME_PAGE);
    return true;
    }
  if ('/' != *Buff)
    return false;
  if (1 == len) {
    sendFile(client, HOME_PAGE);
    return true;
    }
  bool result = false;
  char* p = strchr(Buff, '?');
  if (p) {
    switch (p[1]) { // segue carattere '='
      case 'p': // richiesta pagina
        sendFile(client, p + 3);
        result = true;
        break;
      case 't': // richiesta temperature
        sendTemper(client);
        break;
      case 'r': // richiesta/invio stato relè (pulsante software + uscita attuale)
        if ('=' == p[2])
          sendRele(client, p + 3);
        else
          sendRele(client, 0);
        break;
      case 'x': // richiesta/invio stato relè (pulsante software + uscita attuale)
        if ('=' == p[2])
          sendAlarm(client, p + 3);
        else
          sendAlarm(client, 0);
        break;
      case 'm': // richiesta/invio stato relè (pulsante software + uscita attuale)
        if ('=' == p[2])
          sendMotor(client, p + 3);
        else
          sendMotor(client, 0);
        break;
      case 'u': // umidit�
        sendHum(client);
        break;
      case 'i': // temperatura impostata corrente
        sendImpTemper(client);
        break;
      case 'c': // richiesta/invio stato cronotermostato
        if ('=' == p[2])
          sendCrono(client, p + 3);
        else
          sendCrono(client, 0);
        break;
      case 'a': // invia tutto
        if ('=' == p[2])
          sendAll(client, p + 3);
        else
          sendAll(client, 0);
        break;
      case 'd':
        syncTime(client, p + 3);
        break;
      case 'o':
        Cookie.rem(currSID);
        sendHeaderRefresh(client);
        break;
      }
    }
  else { // è una richiesta di oggetto, es. immagine o script
    sendFile(client, Buff + 1);
    result = true;
    }
  return result;
}
//-------------------------------------------------------------------
static const char* getContentType(const char* ext)
{
  if (!strcmp(ext, ".html") || !strcmp(ext, ".htm"))
    return "text/html";
  if (!strcmp(ext, ".css"))
    return "text/css";
  if (!strcmp(ext, ".js"))
    return "application/javascript";
  if (!strcmp(ext, ".png"))
    return "image/png";
  if (!strcmp(ext, ".gif"))
    return "image/gif";
  if (!strcmp(ext, ".jpg"))
    return "image/jpeg";
  if (!strcmp(ext, ".ico"))
    return "image/x-icon";
  if (!strcmp(ext, ".xml"))
    return "text/xml";
  return "text/plain";
}
//-------------------------------------------------------------------
const char* getExt(const char* name)
{
  int len = strlen(name);
  for (int i = len - 1; i; --i) {
    if ('.' == name[i])
      return name + i;
  }
  return "";
}
//-------------------------------------------------------------------
void sendHeader404(EthernetClient& client, const char* fn)
{
  enableLan();
  client.println(F("HTTP/1.1 404 Not Found"));
  client.print(F("Content-Length: "));
  client.println(12 + strlen(fn));
  client.println();
  client.print(F("["));
  client.print(fn);
  client.print(F("] not found"));
}
//-------------------------------------------------------------------
static void sendHeader (EthernetClient& client, const char* ext, int len, const char* addHeader = 0)
{
  enableLan();
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Connection: close"));
  client.print(F("Content-Length: "));
  client.println(len);
  client.print(F("Content-Type: "));
  client.println(getContentType(ext));
  if (needSendSID) {
    client.print(F("Set-Cookie: SID="));
    client.println(currSID);
    needSendSID = false;
  }
  if (addHeader)
    client.println(addHeader);
  client.println();
}
//---------------------------------------
static const char* no_cache = "Cache-Control: private, no-cache, no-store";
static const char* ok_cache = "Cache-Control: private, max-age=2592000";
//---------------------------------------
static void sendHeaderRefresh(EthernetClient& client)
{
  enableLan();
  const char* msg = "<Refresh>";
  int len = strlen(msg);
  sendHeader(client, 0, len, no_cache);
  client.write(msg, len);
}
//---------------------------------------
extern bool isFileLog(const char* filename);
extern const char* getLogDir();
//---------------------------------------
static bool openFile(const char* f)
{
  char path[32] = "site/";
  if (isFileLog(f))
    strcpy(path, getLogDir());
  strcat(path, f);
  DEBUG_WEB(Serial.print(F("file to send:")))
  DEBUG_WEB(Serial.print(path))

  enaSd();
  if (!currSD->exists(path)) {
    DEBUG_WEB(Serial.println(F(" -> Not Exists")))
  }
  myFile = currSD->open(path);
  if (!myFile) {
    DEBUG_WEB(Serial.println(F(" -> Open Error")))
    return false;
  }
  DEBUG_WEB(Serial.println(F(" -> Ok")))
  return true;
}
//---------------------------------------
static bool performSendFile(EthernetClient& client)
{
  enaSd();
  int32_t dim = myFile.size();
#define chunk g_Buff
  DEBUG_WEB(Serial.print(F("Free memory: ")))
  DEBUG_WEB(Serial.println(FreeRam()))
  while (dim > 0) {
    enaSd();
    int len = myFile.read(chunk, sizeof(chunk));
    enableLan();
    client.write(chunk, len);
    dim -= len;
  }
  enaSd();
  myFile.close();
  return true;
}
//-------------------------------------------------------------------
void webServer::sendFileNoCheck(EthernetClient& client, const char* f)
{
  extern bool needUpdateCrono;
  needUpdateCrono = true;

  if (!openFile(f)) {
    sendHeader404(client, f);
    return;
  }
  char* ext = strchr(f, '.');
  const char* cache = ok_cache;
  if (!strcmp(".htm", ext))
    cache = no_cache;
  sendHeader(client, ext, myFile.size(), cache);
  performSendFile(client);
}
//-------------------------------------------------------------------
void webServer::sendFile(EthernetClient& client, const char* f)
{
  if (!strcmp(f, HOME_PAGE)) {
    if (!Cookie.isValid(currSID)) {
      f = LOGIN_PAGE;
      needCheckUser = true;
    }
    DEBUG_WEB(Serial.print(F("Open file:")))
    DEBUG_WEB(Serial.println(f))
  }
  sendFileNoCheck(client, f);
}
//-------------------------------------------------------------------
// type = 1 > temper, 2 -> rele, 4 -> umidità , 8 -> alarm, 16 -> cronotermostato,
// 32 -> temperature impostata corrente, 64 -> imposta datetime, 256 -> motor, 255 -> all
extern byte fillData(char* buff, int len, uint16_t& type, const char* cmd);
extern void switchRele(const char* cmd);
extern void switchMotor(const char* cmd);
extern void switchAlarm(const char* cmd);
extern void toggleCrono(const char* cmd, bool forceRefresh);
//-------------------------------------------------------------------
void webServer::sendGeneric(EthernetClient& client, uint16_t type, const char* cmd)
{
  static unsigned long last;
  if (millis() - last > 1000L * 10) {
    last = millis();
    if (!Cookie.isValid(currSID)) {
      sendHeaderRefresh(client);
      return;
    }
  }
  int16_t res = 0;
  do {
    res = fillData(g_Buff, sizeof(g_Buff), type, cmd);
    int len = strlen(g_Buff);
    sendHeader(client, 0, len, no_cache);
    client.write(g_Buff, len);
    DEBUG_WEB(Serial.println(g_Buff))
  } while (res);
}
//-------------------------------------------------------------------
void webServer::sendTemper(EthernetClient& client)
{
  sendGeneric(client, 1);
}
//-------------------------------------------------------------------
void webServer::sendRele(EthernetClient& client, const char* cmd)
{
  if (cmd)
    switchRele(cmd);
  sendGeneric(client, 2);
}
//-------------------------------------------------------------------
void webServer::sendHum(EthernetClient& client)
{
  sendGeneric(client, 4);
}
//-------------------------------------------------------------------
void webServer::sendAlarm(EthernetClient& client, const char* cmd)
{
  if (cmd)
    switchAlarm(cmd);
  sendGeneric(client, 8);
}
//-------------------------------------------------------------------
void webServer::sendCrono(EthernetClient& client, const char* cmd)
{
  if (cmd)
    toggleCrono(cmd, true);
  sendGeneric(client, 16);
}
//-------------------------------------------------------------------
void webServer::sendImpTemper(EthernetClient& client)
{
  sendGeneric(client, 32);
}
//-------------------------------------------------------------------
extern void SyncDateTime(const char* p);
//-------------------------------------------------------------------
void webServer::syncTime(EthernetClient& client, const char* p)
{
  SyncDateTime(p);
  sendGeneric(client, 64);
}
//-------------------------------------------------------------------
void webServer::sendMotor(EthernetClient& client, const char* cmd)
{
  if (cmd)
    switchMotor(cmd);
  sendGeneric(client, 256);
}
//-------------------------------------------------------------------
void webServer::sendAll(EthernetClient& client, const char* cmd)
{
  sendGeneric(client, (uint16_t) - 1, cmd);
}
