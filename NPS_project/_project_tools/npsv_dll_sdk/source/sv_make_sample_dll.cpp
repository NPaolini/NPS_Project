//--------- sv_make_sample_dll.cpp ---------------------------------
//------------------------------------------------------------------
#include "sv_make_dll.h"
//------------------------------------------------------------------
#define ID_TEXT_RELOAD      10
#define ID_TEXT_VAR_TEST    20
//------------------------------------------------------------------
struct infoData
{
  int Prph;
  SV_prphData Data;
  DWORD Mask;
  uint offs;
  infoData() : Prph(0), Mask(0), offs(0) { ZeroMemory(&Data, sizeof(Data)); }
  void clear() { Prph = 0; Mask = 0; offs = 0; ZeroMemory(&Data, sizeof(Data)); }
};
//------------------------------------------------------------------
static
LPCTSTR find_next_param(LPCTSTR p, TCHAR sep = _T(','))
{
  if(!p || !*p)
    return 0;
  while(*p && *p != sep)
    ++p;
  if(!*p)
    return 0;
  ++p;
  while(*p && *p <= _T(' '))
    ++p;
  return *p ? p : 0;
}
//------------------------------------------------------------------
template <typename T>
LPCTSTR getNextValue(LPCTSTR p, T& val)
{
  if(!p)
    return 0;
  val = _ttoi(p);
  return find_next_param(p);
}
//------------------------------------------------------------------
static LPCTSTR makeInfoData(LPCTSTR p, infoData& target)
{
  target.clear();
  if(!p)
    return 0;
  bool ok = false;
  do {
    p = getNextValue(p, target.Prph);
    if(!p)
      break;
    p = getNextValue(p, target.Data.lAddr);
    if(!p)
      break;
    p = getNextValue(p, target.Data.typeVar);
    if(!target.Data.typeVar)
      break;
    int nBit = 0;
    p = getNextValue(p, nBit);
    p = getNextValue(p, target.offs);
    if(SV_prphData::tBitData == target.Data.typeVar) {
      target.Data.U.dw = MAKELONG(nBit, target.offs);
      target.Mask = (DWORD)-1;
      }
    else {
      target.Mask = nBit << target.offs;
      if(!target.Mask)
        target.Mask = (DWORD)-1;
      }
    ok = true;
    } while(false);
  if(!ok)
    target.Prph = 0;
  return p;
}
//------------------------------------------------------------------
static LPCTSTR makeInfoData(uint id, infoData& target)
{
  LPCTSTR p = npSV_GetLocalString(id, getHinstance());
  return makeInfoData(p, target);
}
//------------------------------------------------------------------
static void setBit(const infoData& idata, bool set)
{
  if(!idata.Prph)
    return;
  SV_prphData t = idata.Data;
  int nBit = LOWORD(t.U.dw);
  int offs = HIWORD(t.U.dw);
  t.U.dw = MAKELONG(0 - set, MAKEWORD(offs, nBit));
  npSV_Set(idata.Prph, &t);
}
//------------------------------------------------------------------
static bool isChanged(const infoData& idata)
{
  if(!idata.Prph)
    return false;
  SV_prphData t = idata.Data;
  npSV_Get(idata.Prph, &t);
  if(t.U.dw & idata.Mask) {
    if(SV_prphData::tBitData == idata.Data.typeVar)
      setBit(idata, false);
    else {
      t.U.dw &= ~idata.Mask;
      npSV_Set(idata.Prph, &t);
      }
    return true;
    }
  return false;
}
//------------------------------------------------------------------
static infoData iDataReload;
static infoData iDataTest;
//------------------------------------------------------------------
static bool makeAddrReload()
{
  LPCTSTR p = npSV_GetLocalString(ID_TEXT_RELOAD, getHinstance());
  makeInfoData(p, iDataReload);
  p = npSV_GetLocalString(ID_TEXT_VAR_TEST, getHinstance());
  makeInfoData(p, iDataTest);
  return true;
}
//------------------------------------------------------------------
static bool needReload()
{
  return isChanged(iDataReload);
}
//------------------------------------------------------------------
static bool runTest(infoData& idata)
{
  if(!idata.Prph)
    return false;
  SV_prphData t = idata.Data;
  npSV_Get(idata.Prph, &t);
  if(t.U.dw != idata.Data.U.dw) {
    idata.Data.U.dw = t.U.dw;
    t.U.dw *= 2;
    ++t.lAddr;
    npSV_Set(idata.Prph, &t);
    }
  return true;
}
//------------------------------------------------------------------
ExpImp_DLL
void npSV_System(DWORD msg)
{
  switch(msg) {
    case eINIT_DLL:
      break;
    case eEND_DLL:
      break;
    }
}
//------------------------------------------------------------------
ExpImp_DLL
void npSV_Refresh()
{
  static int checked = 0;
  static bool running = false;
  if(running)
    return;
  running = true;
  if(needReload()) {
    npSV_GetBodyRoutine(eSV_RELOAD_TEXT_SET, LPDWORD(getHinstance()), 0);
    checked = 0;
    }

  if(!checked) {
    checked = -1;
    if(makeAddrReload())
      checked = 1;
    }
  if(-1 == checked) {
    running = false;
    return;
    }
  checked = runTest(iDataTest) ? 1 : -1;
  running = false;
}
//------------------------------------------------------------------
