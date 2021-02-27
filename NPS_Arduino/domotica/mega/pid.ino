//--------- pid.cpp ---------------------------------------
#include "domotica.h"
//---------------------------------------------------------
#define ISNAN(X) (!((X)==(X)))
//---------------------------------------------------------
#ifdef SIMPLE_AVERAGE
//---------------------------------------------------------
#define MAX_NUM_AVERAGE 10
//---------------------------------------------------------
PID::PID(typePidData setPoint) : SetPoint(setPoint), Average(0), Num(0)
{}
//---------------------------------------------------------
typePidData PID::getResult(typePidData curr)
{
  if(useDebug & D_BIT_PID) {
    Serial.print(F("Prima - SetPoint:"));
    Serial.print(SetPoint);
    Serial.print(F(" - Average:"));
    Serial.print(Average);
    Serial.print(F(" - Num:"));
    Serial.print(Num);
    }
  Average *= Num;
  Average += curr;
  Average /= Num + 1;
  if(MAX_NUM_AVERAGE > Num)
    ++Num;
  if(useDebug & D_BIT_PID) {
    Serial.print(F(" -- Dopo - Average:"));
    Serial.print(Average);
    Serial.print(F(" - Num:"));
    Serial.println(Num);
    }
  return SetPoint - Average;
}
#else
//---------------------------------------------------------
PID::PID(typePidData ratioP, typePidData ratioI, typePidData ratioD, typePidData setPoint) : prevErr(0), sumI(0),
    ratioP(ratioP), ratioI(ratioI), ratioD(ratioD), SetPoint(setPoint)
{}
//---------------------------------------------------------
void PID::setRatio(PID::eRatio etype, typePidData value)
{
  switch(etype) {
    case erP:
      ratioP = value;
      break;
    case erI:
      ratioI = value;
      break;
    case erD:
      ratioD = value;
      break;
    }
}
//---------------------------------------------------------
typePidData PID::getLimited(typePidData curr, typePidData maxV, typePidData ratio)
{
  if(ratio > 0) {
    typePidData t = maxV * ratio;
    if(curr > t)
      curr = t;
    else if(curr < -t)
      curr = -t;
    }
  return curr;
}
//---------------------------------------------------------
typePidData PID::getResult(typePidData curr)
{
  if(ISNAN(curr))
    curr = get_setPoint() - prevErr;

  typePidData err = get_setPoint() - curr;
  typePidData deriv = getLimited(err - prevErr, get_setPoint(), getRatioD());
  sumI = getLimited(sumI + err + deriv, get_setPoint(), getRatioI());
  prevErr = err;
  if(useDebug & D_BIT_PID) {
    Serial.print(F("PID: err="));
    Serial.print(err);
    typePidData p = err * getRatioP();
    Serial.print(F(", prop="));
    Serial.print(p);
    typePidData i = sumI * getRatioI();
    Serial.print(F(", integr="));
    Serial.print(i);
    typePidData d = deriv * getRatioD();
    Serial.print(F(", deriv="));
    Serial.print(d);
    Serial.print(F(", result="));
    Serial.println(p + i + d);
    return p + i + d;
    }
  else {
    typePidData res = err * getRatioP() + sumI * getRatioI() + deriv * getRatioD();
    return res;
    }
}
//---------------------------------------------------------
typePidData PID::getNormalizedResult(typePidData curr)
{
  return getResult(curr) / get_setPoint();
}
//---------------------------------------------------------
int PID::getNormalizedResult(typePidData curr, int left, int right)
{
  typePidData res = getNormalizedResult(curr);
  res *=  right - left;
  res += left;
  int result = (int)res;
  if(result < left)
    result = left;
  else if(result > right)
    result = right;
  return result;
}
#endif
