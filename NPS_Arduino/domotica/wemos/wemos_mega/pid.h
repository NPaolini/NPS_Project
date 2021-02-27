//--------- pid.h -----------------------------------------
#ifndef PID_H_
#define PID_H_
//---------------------------------------------------------
#include "domotica.h"
//------------------------------------------------------------------------
typedef float typePidData;
//---------------------------------------------------------
#define SIMPLE_AVERAGE
//---------------------------------------------------------
#ifdef SIMPLE_AVERAGE
class PID
{
  public:
    PID(typePidData setPoint = 20);
    virtual ~PID() {}

    typePidData getResult(typePidData curr);
    void set_setPoint(typePidData value) { SetPoint = value; }
  protected:
    typePidData SetPoint;
    typePidData Average;
    uint8_t Num;
};
#else
class PID
{
  public:
    PID(typePidData ratioP = 0.9, typePidData ratioI = 0.1, typePidData ratioD = 0.05, typePidData setPoint = 20);
    virtual ~PID() {}

    typePidData getResult(typePidData curr);
    typePidData getNormalizedResult(typePidData curr);
    int getNormalizedResult(typePidData curr, int left, int right);

    void reset() { prevErr = 0.0; sumI = 0.0; }

    enum eRatio{ erP, erI, erD };
    void setRatio(eRatio etype, typePidData value);

    void setRatio(typePidData vP, typePidData vI, typePidData vD) { ratioP = vP; ratioI = vI; ratioD = vD; }
    void set_setPoint(typePidData value) { SetPoint = value; }

    virtual typePidData getRatioP() { return ratioP; }
    virtual typePidData getRatioI() { return ratioI; }
    virtual typePidData getRatioD() { return ratioD; }
    virtual typePidData get_setPoint() { return SetPoint; }
  protected:

    typePidData ratioP;
    typePidData ratioI;
    typePidData ratioD;
    typePidData SetPoint;

    typePidData prevErr;
    typePidData sumI;

    typePidData getLimited(typePidData curr, typePidData maxV, typePidData ratio);
};
#endif
//---------------------------------------------------------
#endif
