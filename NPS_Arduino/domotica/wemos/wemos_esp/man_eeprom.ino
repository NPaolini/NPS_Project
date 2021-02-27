#include "common_def.h"
//--------------------------------------------------
void EEPROM_update(uint16_t addr, byte b)
{
  byte t = EEPROM.read(addr);
  if(t != b)
    EEPROM.write(addr, b);
}
//--------------------------------------------------
void initializeEE(infoSetup* is)
{
  for(int i = 0; i < DIM_HEADER; ++i)
    EEPROM_update(addr_EE_data + i, stdEE_data[i]);
  writeMasterEE((byte*)is, sizeof(*is));
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
void writeMasterEE(byte* buff, int len)
{
  for(int i = 0; i < len; ++i)
    EEPROM_update(addr_EE_data + OFFS_DATA + i, buff[i]);
}
//--------------------------------------------------
