//------ email_sender.cpp -----------------------------------------
//-----------------------------------------------------------------
#include "email_sender.h"
//-----------------------------------------------------------------
iSenderEmail::iSenderEmail(const infoEmail& ie) : iEmail(ie) {}
//------------------------------------------------------------------
iSenderEmail::~iSenderEmail() { }
//------------------------------------------------------------------
class performSendEmail
{
  public:
    performSendEmail(const infoEmail& ie);
    ~performSendEmail() {  }
    uint8_t send(const char* msg);
    bool send_RCPT_TO(const char* target);
    bool sendBody(const char* text);
    template<typename T> bool sendDataNL(T* text, uint16_t numResp = 0);
    template<typename T> bool sendData(T* text, uint16_t numResp = 0);
    bool getResp(uint16_t numResp);
  private:
    const infoEmail& iEmail;
    WiFiClient client;
    bool valid;
    template<typename T> bool sendGenData(T* text);
    bool sendRawData(const char* text, int len);
    bool sendHeaderSmtp();
};
//------------------------------------------------------------------
performSendEmail::performSendEmail(const infoEmail& ie) : iEmail(ie)
{
}
//------------------------------------------------------------------
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
uint8_t performSendEmail::send(const char* msg)
{
  if(!client.connect(iEmail.smtp, iEmail.port)) {
    client.stop();
    return 21;
    }
  bool success = false;
  uint8_t codelog = 10;
  do {
    if(!getResp(rOpen))
      break;
    codelog = 9; // 9
    if(iEmail.user[0]) {
      sendGenData(myT("EHLO "));
      if(!sendDataNL(iEmail.smtp, rOk))
        break;
      codelog = 8; // 8
      if(!sendDataNL(myT("AUTH LOGIN"), rAccepted))
        break;
      codelog = 7; // 7
      if(!sendDataNL(iEmail.user, rAccepted))
        break;
      codelog = 6; // 6
      if(!sendDataNL(iEmail.psw, rOkUser))
        break;
      codelog = 5;  // 5
      }
    else {
      sendGenData(myT("HELO "));
      if(!sendDataNL(iEmail.smtp, rOk))
        break;
      codelog = 8; // 5
      }
    sendGenData(myT("MAIL FROM: <"));
    sendGenData(iEmail.from);
    if(!sendDataNL(myT(">"), rOk))
      break;
    codelog = 4; // 4

    if(!send_RCPT_TO(iEmail.to))
      break;

    codelog = 3; // 3
    if(!sendDataNL(myT("DATA"), rData))
      break;

    codelog = 2; // 2
    if(!sendBody(msg))
      break;
    success = true;
    codelog = 1; // 1
    if(!sendDataNL(myT("QUIT"), rClose))
      break;
    codelog = 0; // 0
    } while(false);
  client.stop();
  return codelog;
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
    if(!sendGenData(iEmail.from)) break;
    if(!sendGenData(myT(">\r\n"))) break;

    if(!sendGenData(myT("To: <"))) break;
    if(!sendGenData(iEmail.to)) break;
    if(!sendGenData(myT(">\r\n"))) break;

    if(!sendGenData(myT("Subject: Allarme casa\r\n"))) break;
    if(!sendGenData(myT("X-Mailer: Simple SendMail by NPS\r\n"))) break;
    if(!sendGenData(myT("X-Priority: 2\r\n"))) break;
    if(!sendGenData(myT("Content-Type: text/plain\r\n\r\n"))) break;
    success = true;
    } while(false);
  return success;
}
//------------------------------------------------------------------
uint8_t iSenderEmail::send(const char* msg)
{
  bool valid = false;
  do {
    if(!iEmail.smtp[0])
      break;
    if(!iEmail.from[0])
      break;
    if(!iEmail.to[0])
      break;
    if(!iEmail.port)
      break;

    valid = true;
    } while(false);
  if(!valid)
    return 20;
  performSendEmail pse(iEmail);
  return pse.send(msg);
}
//-----------------------------------------------------------------
