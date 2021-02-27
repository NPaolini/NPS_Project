/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "common_def.h"

#define DEF_LISTEN_PORT 80
ESP8266WebServer server(DEF_LISTEN_PORT);

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

manCookie Cookie;
static char currSID[DIM_COOKIE + 1] = {0};
//-------------------------------------------------------------------
infoSetup ISetup =
{
  { 192, 168, 1, 95 },
  { 192, 168, 1, 254 },
  { 192, 168, 1, 254 },
  DEF_LISTEN_PORT,
  STASSID,
  STAPSK
};
byte stdEE_data[DIM_HEADER] = { 'N', 'P', 'D' };

char g_Buff[1024];

char g_File[1024];
//------------------------------
#define SIZE_A(a) (sizeof(a) / sizeof(*a))
//------------------------------
#define saveSetup() writeMasterEE((byte*)&ISetup, sizeof(ISetup))
//-------------------------------------------------------------------
enum eMIME { eHTML = 1, eCSS, eJS, ePNG, eGIF, eJPEG, eICO, eXML, eDEFAULT };
//-------------------------------------------------------------------
static const char* getContentType(eMIME type)
{
  switch(type) {
    case eHTML:
      return "text/html";
    case eCSS:
      return "text/css";
    case eJS:
      return "application/javascript";
    case ePNG:
      return "image/png";
    case eGIF:
      return "image/gif";
    case eJPEG:
      return "image/jpeg";
    case eICO:
      return "image/x-icon";
    case eXML:
      return "text/xml";
    case eDEFAULT:
    default:
      return "text/plain";
    }
}
//-------------------------------------------------------------------
template <typename T>void sendFile(T buff, eMIME mime, size_t len)
{
  const char* m = getContentType(mime);
  server.send(200, m, buff, len);
}
//-------------------------------------------------------------------
template <typename T>void sendHeader (T client, eMIME type, int len, const char* addHeader = 0)
{
  client.println(F("HTTP/1.1 200 OK"));
  client.println(F("Connection: close"));
  client.print(F("Content-Length: "));
  client.println(len);
  client.print(F("Content-Type: "));
  client.println(getContentType(type));
  if(*currSID) {
    client.print(F("Set-Cookie: SID:"));
    client.println(currSID);
    }
  if (addHeader)
    client.println(addHeader);
  client.println();
}
//-------------------------------------------------------------------
static const char* no_cache = "Cache-Control: private, no-cache, no-store";
static const char* ok_cache = "Cache-Control: private, max-age=2592000";
//-------------------------------------------------------------------
template <typename T>void loadAndsendFile(T filename, eMIME mime)
{
  Serial.print(F("cmd=getFile:"));
  Serial.print(filename);
  Serial.print(F("#"));
  char resp[] = "r=getFile:";
  WiFiClient client = server.client();
  if(Serial.find(resp, strlen(resp))) {
    int16_t len = Serial.readBytesUntil('#', g_Buff, sizeof(g_Buff));
    g_Buff[len] = 0;
    int32_t dim = atoi(g_Buff);
    if(!dim) {
      client.stop();
      return;
      }
    sendHeader(client, mime, dim, (eHTML == mime) ? no_cache : ok_cache);
    Serial.print(F("ACK"));
    while(dim > 0) {
      len = Serial.readBytesUntil('#', g_Buff, sizeof(g_Buff));
      g_Buff[len] = 0;
      int lenChunk = atoi(g_Buff);
      if(!lenChunk)
        break;
      len = min((unsigned int)lenChunk, sizeof(g_File));
      while(lenChunk > 0) {
        len = Serial.readBytes(g_File, len);
        if(len <= 0)
          break;
        else {
          client.write((const uint8_t *)g_File, (size_t)len);
          dim -= len;
          lenChunk -= len;
          }
        }
      Serial.print(F("ACK"));
      }
    }
  client.stop();
}
//-------------------------------------------------------------------
void send_code_js()
{
  loadAndsendFile(F("code.js"), eJS);
}
//-------------------------------------------------------------------
void send_canvas_js()
{
  loadAndsendFile(F("canvas.js"), eJS);
}
//-------------------------------------------------------------------
void send_canvtext_js()
{
  loadAndsendFile(F("canvtext.js"), eJS);
}
//-------------------------------------------------------------------
void send_nps_ico()
{
  loadAndsendFile(F("nps.ico"), eICO);
}
//-------------------------------------------------------------------
void send_sky_jpg()
{
  loadAndsendFile(F("sky.jpg"), eJPEG);
}
//-------------------------------------------------------------------
void send_down_png()
{
  loadAndsendFile(F("down.png"), ePNG);
}
//-------------------------------------------------------------------
void send_up_png()
{
  loadAndsendFile(F("up.png"), ePNG);
}
//-------------------------------------------------------------------
void send_btn_png()
{
  loadAndsendFile(F("btn.png"), ePNG);
}
//-------------------------------------------------------------------
void send_style_css()
{
  loadAndsendFile(F("style.css"), eCSS);
}
//-------------------------------------------------------------------
void send_login_htm()
{
  loadAndsendFile(F("login.htm"), eHTML);
}
//-------------------------------------------------------------------
template <typename T>void sendData(T buff, size_t len)
{
  const char* m = getContentType(eDEFAULT);
  server.send(200, m, buff, len);
}
//-------------------------------------------------------------------
bool compare(const char* match, const char* test)
{
  while(*match) {
    if(*match++ != *test++)
      return false;
    }
  return true;
}
//-------------------------------------------------------------------
int8_t GS_currUser;
//-------------------------------------------------------------------
void parseArg(String argName, String arg)
{
  Serial.print(F("cmd="));
  Serial.print(argName);
  Serial.print(F(":"));
  Serial.print(arg);
  if('x' == argName.c_str()[0]) {
    Serial.print(F(","));
    Serial.print(GS_currUser);
    }
  Serial.print(F("#"));
  char resp[] = "r=";
  if(Serial.find(resp, 2)) {
    int len = Serial.readBytesUntil('#', g_Buff, sizeof(g_Buff));
    g_Buff[len] = 0;
    if(compare("<Refresh>", g_Buff))
      Cookie.rem(GS_currUser);
    sendData(g_Buff, len);
    }
  else
    sendData(g_Buff, 0);
}
//-------------------------------------------------------------------
//Check if header is present and correct
#define MAX_ERR_AUTH 5
bool is_authentified(bool home)
{
  static uint8_t maxErr = 0;
  bool auth = false;
  if (server.hasHeader("Cookie")) {
    String cookie = server.header("Cookie");
    int pos = cookie.indexOf("SID:");
    if(-1 != pos) {
      const char* p = cookie.c_str() + pos + 4;
      auth = Cookie.isValid(p, GS_currUser);
      }
    }
  if(auth)
    maxErr = 0;
  else if(!home && ++maxErr < MAX_ERR_AUTH)
    return true;
  return auth;
}
//-------------------------------------------------------------------
#define DIM_COOKIE 8
//-------------------------------------------------------------------
void handleLogin()
{
  if (server.hasArg("UserName") && server.hasArg("UserPass")) {
    Serial.print(F("cmd=w:"));
    Serial.print(server.arg("UserName"));
    Serial.print(F(","));
    Serial.print(server.arg("UserPass"));
    Serial.print(F("#"));
    char resp[] = "r=w:";
    if(Serial.find(resp, 4)) {
      char buff[32];
      memset(buff, 0, sizeof(buff));
      int len = Serial.readBytesUntil('#', buff, sizeof(buff)-1);
      buff[len] = 0;
      GS_currUser = atoi(buff);
      if((uint8_t)GS_currUser < MAX_USER) {
        uint8_t pos = Cookie.make(GS_currUser);
        strcpy(buff, "SID:");
        Cookie.get(buff + 4, pos);
        }
      server.sendHeader("Location", "/");
      server.sendHeader("Cache-Control", "no-cache");
      server.sendHeader("Set-Cookie", buff);
      server.send(301);
      }
    }
}
//-------------------------------------------------------------------
void handleRoot()
{
  int count = server.args();
  bool auth = is_authentified(!count);

  if(!count) {
    if(!auth) {
      server.sendHeader("Location", "/login.htm");
      server.sendHeader("Cache-Control", "no-cache");
      server.send(301);
      return;
      }
    loadAndsendFile(F("index.htm"), eHTML);
    }
  else {
    if(!auth) {
      sendData("<Refresh>", strlen("<Refresh>"));
      return;
      }
    for(int i = 0; i < count; ++i) {
      String argName = server.argName(i);
      String arg = server.arg(i);
      parseArg(argName, arg);
      }
    }
}
//-------------------------------------------------------------------
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}
//-------------------------------------------------------------------
bool onDaylight(int day, int month)
{
  if(3 == month) { // marzo, ultima domenica
    int dow = timeClient.getDay();
    if(day >= 31 - 7 + dow)
      return true;
    return false;
    }
  if(10 == month) { // ottobre, ultima domenica
    int dow = timeClient.getDay();
    if(day < 31 - 7 + dow)
      return true;
    return false;
    }
  if(3 < month && month < 10)
    return true;
  return false;
}
//-------------------------------------------------------------------
bool updateTime()
{
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime ((time_t *)&epochTime);

  int currentYear = ptm->tm_year+1900;
  if(currentYear < 2000) {
    timeClient.forceUpdate();
    epochTime = timeClient.getEpochTime();
    ptm = gmtime ((time_t *)&epochTime);
    currentYear = ptm->tm_year+1900;
    if(currentYear < 2000)
      return false;
    }

  int monthDay = ptm->tm_mday;
  int currentMonth = ptm->tm_mon+1;
  time_t offset = 3600;
  if(onDaylight(monthDay, currentMonth))
    offset += 3600;
  timeClient.setTimeOffset(offset);
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

  setTime(currentHour, currentMinute, currentSecond, monthDay, currentMonth, currentYear);
  return true;
}
//-------------------------------------------------------------------
void setup(void) {
  if(!readMasterEE(&ISetup))
    initializeEE(&ISetup);

  Serial.begin(1000000);

  IPAddress subnet(255, 255, 255, 0);
  WiFi.config(IPAddress(ISetup.ip), IPAddress(ISetup.gateway), subnet, IPAddress(ISetup.dns));
  WiFi.mode(WIFI_STA);
  WiFi.begin(ISetup.ssid, ISetup.password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    }

  server.on("/", handleRoot);
  server.on("/index.htm", HTTP_POST, handleLogin);
  server.on("/index.htm", HTTP_GET, handleRoot);

  server.on("/code.js", send_code_js);
  server.on("/canvas.js", send_canvas_js);
  server.on("/canvtext.js", send_canvtext_js);
  server.on("/nps.ico", send_nps_ico);
  server.on("/sky.jpg", send_sky_jpg);
  server.on("/down.png", send_down_png);
  server.on("/up.png", send_up_png);
  server.on("/btn.png", send_btn_png);
  server.on("/style.css", send_style_css);
  server.on("/login.htm", send_login_htm);
  server.onNotFound(handleNotFound);
  const char* headerKeys = "Cookie";
  server.collectHeaders(&headerKeys, 1);
  server.begin(ISetup.port);

// Initialize a NTPClient to get time
  timeClient.begin();
  updateTime();
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
  if(!buff || !*buff)
    return 0;
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
int8_t GS_loadParam(uint16_t* param, int8_t num)
{
  int len = Serial.readBytesUntil('#', g_Buff, sizeof(g_Buff) - 1);
  g_Buff[len] = 0;
  int8_t res = G_fillParam(param, num, g_Buff);
  return res;
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
  uint16_t par[12] = {};
  int8_t num = GS_loadParam(par, 12);
  if(12 == num) {
    copy(ISetup.ip, par, 4);
    copy(ISetup.dns, par + 4, 4);
    copy(ISetup.gateway, par + 8, 4);
    saveSetup();
    IPAddress staticIP(ISetup.ip);
    IPAddress dns(ISetup.dns);
    IPAddress gateway(ISetup.gateway);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.disconnect();
    WiFi.config(staticIP, gateway, subnet, dns);
    WiFi.mode(WIFI_STA); //WiFi mode station (connect to wifi router only
    WiFi.begin(ISetup.ssid, ISetup.password);

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
      delay(100);
      }
    }
  Serial.print(F("r=ip:"));
  Serial.print(WiFi.localIP());
  Serial.print(F("#"));
}
//-------------------------------------------------------------------
void GS_setPort()
{
  uint16_t par[1] = {};
  GS_loadParam(par, 1);
  server.close();
  server.begin(par[0]);
  Serial.print(F("r=port:"));
  Serial.print(server.getServer().port());
  Serial.print(F("#"));
  ISetup.port = par[0];
  saveSetup();
}
//-------------------------------------------------------------------
void GS_sendEmail()
{
  infoEmail ie;
  Serial.readBytes((char*)&ie, sizeof(ie));
  int len = Serial.readBytesUntil('#', g_Buff, sizeof(g_Buff));
  uint8_t resp = 255;
  if(len) {
    g_Buff[len] = 0;
    iSenderEmail ise(ie);
    resp = ise.send(g_Buff);
    }
  Serial.print(F("r=email:"));
  Serial.print(resp);
  Serial.print(F("#"));
}
//-------------------------------------------------------------------
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
//-------------------------------------------------------------------
void GS_getIP()
{
  Serial.print(F("r=getip:"));

  GS_printInfoIP(WiFi.localIP(), 0, false);
  Serial.print(F(","));
  GS_printInfoIP(WiFi.dnsIP(), 0, false);
  Serial.print(F(","));
  GS_printInfoIP(WiFi.gatewayIP(), 0, false);
  Serial.print(F(","));
  Serial.print(server.getServer().port());
  Serial.println(F("#"));
}
//-------------------------------------------------------------------
void GS_getWIFI()
{
  Serial.print(F("r=getwifi:"));
  Serial.print(ISetup.ssid);
  Serial.print(F(","));
  Serial.print(ISetup.password);
  Serial.print(F("#"));
}
//-------------------------------------------------------------------
bool readToBuff(char* target, uint8_t dim)
{
  //char buff[MAX_BUFF_USER] = {0};
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
void GS_setWIFI()
{
  char ssid[DIM_SSID + 1] = {};
  char password[DIM_PSW + 1] = {};
  if(readToBuff(ssid, DIM_SSID)) {
    if(readToBuff(password, DIM_PSW)) {
      strcpy(ISetup.ssid, ssid);
      strcpy(ISetup.password, password);
      saveSetup();
      }
    }
  Serial.print(F("r=wifi:"));
  Serial.print(ssid);
  Serial.print(F(","));
  Serial.print(password);
  Serial.print(F("#"));
  WiFi.disconnect();
  WiFi.begin(ISetup.ssid, ISetup.password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    }
}
//-------------------------------------------------------------------
void GS_syncTime()
{
  uint16_t par[1] = {};
  GS_loadParam(par, 1);
  if(par[0])
    updateTime();
  Serial.print(F("r=synctime:"));
  Serial.print(now());
  Serial.print(F("#"));
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
  if(!strcmp(buff, "ip"))
    GS_setIP();
  else if(!strcmp(buff, "port"))
    GS_setPort();
  else if(!strcmp(buff, "email"))
    GS_sendEmail();
  else if(!strcmp(buff, "getip"))
    GS_getIP();
  else if(!strcmp(buff, "wifi"))
    GS_setWIFI();
  else if(!strcmp(buff, "getwifi"))
    GS_getWIFI();
  else if(!strcmp(buff, "synctime"))
    GS_syncTime();
  return true;
}
//-------------------------------------------------------------------
#define TIME_TO_UPDATE (1000L * 60 * 60)
#define TIME_TO_UPDATE_NEAR (1000L * 60)
//-------------------------------------------------------------------
void loop(void) {
  server.handleClient();
  GS_cmdBySerial();
  Cookie.loop();
  static unsigned long prev = 0;
  if(millis() - prev > TIME_TO_UPDATE) {
    prev = millis();
    if(!updateTime())
      prev -= TIME_TO_UPDATE - TIME_TO_UPDATE_NEAR;
    }
}
