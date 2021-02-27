//------ email_sender.h -------------------------------------------
#ifndef email_sender_H_
#define email_sender_H_
//-----------------------------------------------------------------
#include <WiFiClient.h>
//------------------------------
#define MAX_DIM_EMAIL 64
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
//-----------------------------------------------------------------
class iSenderEmail
{
  public:
    iSenderEmail(const infoEmail& ie);
    virtual ~iSenderEmail();
    virtual uint8_t send(const char* msg);
  protected:
    const infoEmail& iEmail;
};
//-----------------------------------------------------------------
#endif
