//------ email_sender.h -------------------------------------------
#ifndef email_sender_H_
#define email_sender_H_
//-----------------------------------------------------------------
/* caricati da eeprom
comando da seriale:
cmd=email:k,v,k,v,k,v
dove k-> chiave, v -> valore
chiave: s(mtp), p(ort), f(rom), t(o), u(ser), (pass)w(ord) #
non si usano cc n� bcc, subject e xmailer sono codificati
*/
//-----------------------------------------------------------------
class iSenderEmail
{
  public:
    iSenderEmail();
    virtual ~iSenderEmail();
    virtual bool send(const char* msg) = 0; // in ASCII
    virtual void begin() = 0;
  protected:
};
//-----------------------------------------------------------------
iSenderEmail* getSenderEmail();
//-----------------------------------------------------------------
#endif
