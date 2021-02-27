#ifndef WEB_SERVER_H
#define WEB_SERVER_H
//----------------------------------
#include <Ethernet.h>
//----------------------------------
class IFace_web_server
{
  public:
    IFace_web_server() {}
    virtual ~IFace_web_server() {}
    virtual void beginLan(byte ip[], byte dns[], byte gateway[], byte mac[]) = 0;
    virtual void beginServer(int listenPort) = 0;
    virtual bool webServerLoop() = 0;
};
//----------------------------------
IFace_web_server* getServer();
//----------------------------------
#endif
