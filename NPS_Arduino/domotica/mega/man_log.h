//------ man_log.h ---------------------------
//--------------------------------------------
#ifndef man_log_H_
#define man_log_H_
//--------------------------------------------
#include "domotica.h"
//--------------------------------------------
class man_log
{
  public:
    enum eFile { eL_Email = 1, eL_Access, eL_Alarm };
    man_log(eFile e);
    ~man_log();
    void logSend(const char* info[], uint8_t num);
    void printLog();
    void deleteLog();
  private:
    eFile eF;
    const char* getFilename();
};
#endif
