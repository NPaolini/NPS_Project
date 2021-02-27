#include "man_eeprom.h"
//--------------------------------------------------
void initializeEE(infoSetup* is)
{
  for(int i = 0; i < DIM_HEADER; ++i)
    EEPROM.update(addr_EE_data + i, stdEE_data[i]);
  writeMasterEE(is, sizeof(*is));
}
//--------------------------------------------------
bool readMasterEE(infoSetup* is)
{
  for(int i = 0; i < DIM_HEADER; ++i) {
    if(EEPROM.read(addr_EE_data + i) != stdEE_data[i])
      return false;
    }
  byte* buff = (byte*)is;
  for(int i = 0; i < sizeof(*is); ++i)
    buff[i] = EEPROM.read(addr_EE_data + OFFS_DATA + i);
  return true;
}
//--------------------------------------------------
void writeMasterEE(void* buff, int len)
{
  byte* p = (byte*)buff;
  for(int i = 0; i < len; ++i)
    EEPROM.update(addr_EE_data + OFFS_DATA + i, p[i]);
}
//--------------------------------------------------
void readCronoEE(void* buff, int8_t dow)
{
  byte* p = (byte*)buff;
  int addr = addr_EE_crono + DIM_CRONO * (dow - 1);
  for(byte i = 0; i < DIM_CRONO; ++i)
    p[i] = EEPROM.read(addr + i);
}
//--------------------------------------------------
void writeCronoEE(void* buff, int8_t dow)
{
  byte* p = (byte*)buff;
  int addr = addr_EE_crono + DIM_CRONO * (dow - 1);
  for(byte i = 0; i < DIM_CRONO; ++i)
    EEPROM.update(addr + i, p[i]);
}
//--------------------------------------------------
void readCronoEESingle(void* buff, int8_t* adr, int8_t num, int8_t dow)
{
  byte* p = (byte*)buff;
  int addr = addr_EE_crono + DIM_CRONO * (dow - 1);
  for(byte i = 0; i < num; ++i)
    p[i] = EEPROM.read(addr + adr[i]);
}
//--------------------------------------------------
void writeCronoEESingle(void* buff, int8_t* adr, int8_t num, int8_t dow)
{
  byte* p = (byte*)buff;
  int16_t addr = addr_EE_crono + DIM_CRONO * (dow - 1);
  for(byte i = 0; i < num; ++i)
    EEPROM.update(addr + adr[i], p[i]);
}
//--------------------------------------------------
