//------ email_sender.cpp -----------------------------------------
//-----------------------------------------------------------------
#include "domotica.h"
//-----------------------------------------------------------------
iSenderEmail::iSenderEmail() { }
//------------------------------------------------------------------
iSenderEmail::~iSenderEmail() { }
//------------------------------------------------------------------
class performSendEmail : public iSenderEmail
{
  private:
    typedef iSenderEmail baseClass;
  public:
    performSendEmail();
    ~performSendEmail() {  }
    void begin();
    bool send(const char* msg);
    bool send_RCPT_TO(const char* target);
    bool sendBody(const char* text);
    template<typename T> bool sendDataNL(T* text, uint16_t numResp = 0);
    template<typename T> bool sendData(T* text, uint16_t numResp = 0);
    bool getResp(uint16_t numResp);
    void deleteLog();
    void printLog();
  private:
    EthernetClient client;
    const infoSetup* isetup;
    bool valid;
    void logSend(const char* target, bool success, const char* msg, const char* info = 0);

    template<typename T> bool sendGenData(T* text);
    bool sendRawData(const char* text, int len);
    bool sendHeaderSmtp();
};
//------------------------------------------------------------------
performSendEmail::performSendEmail() : baseClass(), isetup(0), valid(false), client(0)
{
}
//------------------------------------------------------------------
void performSendEmail::begin()
{
  valid = false;
  isetup = getISetup();
  do {
    DEBUG_EMAIL(Serial.println(isetup->smtp))
    if(!isetup->smtp[0])
      break;
    DEBUG_EMAIL(Serial.println(isetup->from))
    if(!isetup->from[0])
      break;
    DEBUG_EMAIL(Serial.println(isetup->to))
    if(!isetup->to[0])
      break;
    DEBUG_EMAIL(Serial.println(isetup->email_port))
    if(!isetup->email_port)
      break;
    DEBUG_EMAIL(Serial.println("Ok"))

    valid = true;
    } while(false);
}
//------------------------------------------------------------------
performSendEmail SendEmail;
iSenderEmail* getSenderEmail() { return &SendEmail; }
//-----------------------------------------------------------------
enum retSmtp { rOpen = 220, rClose = 221, rOk = 250, rData = 354, rAccepted = 334, rOkUser = 235 };
//-----------------------------------------------------------------
static bool isValidAddr(const char* p)
{
  if(!p)
    return false;
  return 0 != strchr(p, '@');
}
//-----------------------------------------------------------------
#define myT F
//#define myT
//-----------------------------------------------------------------
bool performSendEmail::send(const char* msg)
{
  if(!valid) {
    logSend("", false, msg, "Data not valid");
    return false;
    }
  enableLan();
  if(!client.connect(isetup->smtp, isetup->email_port)) {
    logSend(isetup->smtp, false, msg, "Failed open conn");
    enableLan();
    client.stop();
    return false;
    }
  bool success = false;
  uint8_t codelog = 0;
  do {
    if(!getResp(rOpen))
      break;
    ++codelog; // 1
    if(isetup->user[0]) {
      sendGenData(myT("EHLO "));
      if(!sendDataNL(isetup->smtp, rOk))
        break;
      ++codelog; // 2
      if(!sendDataNL(myT("AUTH LOGIN"), rAccepted))
        break;
      ++codelog; // 3
      if(!sendDataNL(isetup->user, rAccepted))
        break;
      ++codelog; // 4
      if(!sendDataNL(isetup->psw, rOkUser))
        break;
      ++codelog;  // 5
      }
    else {
      sendGenData(myT("HELO "));
      if(!sendDataNL(isetup->smtp, rOk))
        break;
      codelog += 4; // 5
      }
    sendGenData(myT("MAIL FROM: <"));
    sendGenData(isetup->from);
    if(!sendDataNL(myT(">"), rOk))
      break;
    ++codelog; // 6

    if(!send_RCPT_TO(isetup->to))
      break;

    ++codelog; // 7
    if(!sendDataNL(myT("DATA"), rData))
      break;

      ++codelog; // 8
    if(!sendBody(msg))
      break;
    success = true;
    ++codelog; // 9
    if(!sendDataNL(myT("QUIT"), rClose))
      break;
    ++codelog; // 10
    } while(false);
  const char* err = 0;
  const char* responceMsg[] = {
     "Err REsponce on Open",
     "Err HELO (EHLO) smtp",
     "Err AUTH LOGIN",
     "Err Send User",
     "Err Send Psw",
     "Err MAIL FROM",
     "Err No Target",
     "Err DATA",
     "Err send Body",
     "Err QUIT"
    };
  if(codelog < SIZE_A(responceMsg) - 1)
    err = responceMsg[codelog];
  enableLan();
  client.stop();
  logSend(isetup->to, success, msg, err);
  return true;
}
//-----------------------------------------------------------------
bool performSendEmail::getResp(uint16_t numResp)
{
  char buff[4] = {0};
  for(int8_t i = 0; i < 5000; ++i) {
    char c = client.read();
    if(-1 == c)
      delay(10);
    else {
      if(c >= '0' && '9' >= c) {
        buff[0] = c;
        client.readBytes(buff + 1, 3);
        break;
        }
      }
    }
  buff[3] = 0;
  while(true) {
    if(-1 == client.read())
      break;
    delay(1);
    }
  return atoi(buff) == numResp;
}
//-----------------------------------------------------------------
bool performSendEmail::sendRawData(const char* text, int len)
{
  return len == client.write(text, len);
}
//-----------------------------------------------------------------
template<typename T> bool performSendEmail::sendGenData(T* text)
{
  String s(text);
  return sendRawData(s.c_str(), s.length());
}
//-----------------------------------------------------------------
bool performSendEmail::send_RCPT_TO(const char* target)
{
  if(!isValidAddr(target))
    return false;
  sendGenData(myT("RCPT TO: <"));
  sendGenData(target);
  if(!sendDataNL(myT(">"), rOk))
    return false;
  return true;
}
//-----------------------------------------------------------------
template<typename T> bool performSendEmail::sendData(T* text, uint16_t numResp)
{
  if(!sendGenData(text))
    return false;
  if(numResp)
    return getResp(numResp);
  return true;
}
//-----------------------------------------------------------------
#define MAX_LEN_LINE 80
#define NL myT("\r\n")
//-----------------------------------------------------------------
template<typename T> bool performSendEmail::sendDataNL(T* text, uint16_t numResp)
{
  if(!sendGenData(text))
    return false;
  return sendData(NL, numResp);
}
//-----------------------------------------------------------------
bool performSendEmail::sendBody(const char* body)
{
  if(!sendHeaderSmtp())
    return false;

#if defined(USE_HW_TIME) || defined(USE_FULL_TIME)
  sendData(rtc.getDateStr());
  sendData(" - ");
  sendDataNL(rtc.getTimeStr());
#else
  sendData(rtc.getDateStr().c_str());
  sendData(" - ");
  sendDataNL(rtc.getTimeStr().c_str());
#endif
  int len = strlen(body);
  bool success = false;
  while(len) {
    success = false;
    int l = min(len, MAX_LEN_LINE);
    if(MAX_LEN_LINE == l) {
      int l2 = l;
      while(l2 > 0) {
        if(body[l2] > ' ')
          --l2;
        else
          break;
        }
      if(l2 > 0)
        l = l2;
      }
    if(!sendRawData(body, l))
      break;
    if(!sendData(NL))
      break;
    len -= l;
    body += l;
    success = true;
    }
  if(success)
    return sendDataNL(myT("\r\n."), rOk);
  return false;
}
//-----------------------------------------------------------------
bool performSendEmail::sendHeaderSmtp()
{
  bool success = false;
  do {
    if(!sendGenData(myT("From: <"))) break;
    if(!sendGenData(isetup->from)) break;
    if(!sendGenData(myT(">\r\n"))) break;

    if(!sendGenData(myT("To: <"))) break;
    if(!sendGenData(isetup->to)) break;
    if(!sendGenData(myT(">\r\n"))) break;

    if(!sendGenData(myT("Subject: Allarme casa\r\n"))) break;
    if(!sendGenData(myT("X-Mailer: Simple SendMail by NPS\r\n"))) break;
    if(!sendGenData(myT("X-Priority: 2\r\n"))) break;
    if(!sendGenData(myT("Content-Type: text/plain\r\n\r\n"))) break;
    success = true;
    } while(false);
  return success;
}
//-----------------------------------------------------------------
void performSendEmail::logSend(const char* target, bool success, const char* msg, const char* info)
{
  {
  man_log ml(man_log::eL_Email);
  const char* s = success ? "Success" : "Failed";
  const char* clog[] = { target, info, s, msg };
  ml.logSend(clog, SIZE_A(clog));
  }
  man_log ml(man_log::eL_Alarm);
  const char* clog[] = { msg };
  ml.logSend(clog, SIZE_A(clog));
}
//-----------------------------------------------------------------
