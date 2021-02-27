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
  private:
    const infoSetup* isetup;
    bool valid;
    void logSend(const char* target, bool success, const char* msg, const char* info = 0);
};
//------------------------------------------------------------------
performSendEmail::performSendEmail() : baseClass(), isetup(0), valid(false)
{
}
//------------------------------------------------------------------
void performSendEmail::begin()
{
  valid = false;
  isetup = getISetup();
  do {
    DEBUG_EMAIL(Serial.println(isetup->iEmail.smtp))
    if(!isetup->iEmail.smtp[0])
      break;
    DEBUG_EMAIL(Serial.println(isetup->iEmail.from))
    if(!isetup->iEmail.from[0])
      break;
    DEBUG_EMAIL(Serial.println(isetup->iEmail.to))
    if(!isetup->iEmail.to[0])
      break;
    DEBUG_EMAIL(Serial.println(isetup->iEmail.port))
    if(!isetup->iEmail.port)
      break;
    DEBUG_EMAIL(Serial.println("Ok"))

    valid = true;
    } while(false);
}
//------------------------------------------------------------------
performSendEmail SendEmail;
iSenderEmail* getSenderEmail() { return &SendEmail; }
//-----------------------------------------------------------------
bool performSendEmail::send(const char* msg)
{
  if(!valid) {
    logSend("", false, msg, "Data not valid");
    return false;
    }
  Serial3.print(F("cmd=email:"));
  Serial3.write((byte*)&isetup->iEmail, sizeof(isetup->iEmail));
  Serial3.print(rtc.getDateStr());
  Serial3.print(" - ");
  Serial3.println(rtc.getTimeStr());

  Serial3.print(msg);
  Serial3.print(F("#"));
  for(int i = 0; i < 50; ++i) {
    if(Serial3.available())
      break;
    delay(100);
    }
  uint8_t result = 255;
  if(Serial3.find("r=email:")) {
    char buff[64] = {};
    int8_t len = Serial3.readBytesUntil('#', buff, sizeof(buff));
    buff[len] = 0;
    result = atoi(buff);
    }
  const char* err = "Err Unknow";
  const char* responceMsg[] = {
    "Err QUIT",
    "Err send Body",
    "Err DATA",
    "Err No Target",
    "Err MAIL FROM",
    "Err Send Psw",
    "Err Send User",
    "Err AUTH LOGIN",
    "Err HELO (EHLO) smtp",
    "Err REsponce on Open",
    };
  switch(result) {
    case 20:
      logSend("", false, msg, "Data not valid");
      break;
    case 21:
      logSend(isetup->iEmail.smtp, false, msg, "Failed open conn");
      break;
    case 0:
      logSend(isetup->iEmail.to, true, msg, 0);
      break;
    default:
      --result;
      if(result < SIZE_A(responceMsg))
        err = responceMsg[result];
      logSend(isetup->iEmail.to, false, msg, err);
      break;
    }
  return true;
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
