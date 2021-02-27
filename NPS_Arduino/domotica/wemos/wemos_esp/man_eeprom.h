#ifndef eeprom_H
#define eeprom_H
//------------------------------
#define DIM_HEADER 3 // -> NPD
#define OFFS_DATA DIM_HEADER
//------------------------------
#define addr_EE_data 0
//--------------------------------------------------
#define DIM_SSID 32
#define DIM_PSW  32
//--------------------------------------------------
struct infoSetup
{
  byte ip[4];
  byte dns[4];
  byte gateway[4];
  int16_t port;
  char ssid[DIM_SSID];
  char password[DIM_PSW];
};
//--------------------------------------------------
void initializeEE(infoSetup* is);
bool readMasterEE(infoSetup* is);
void writeMasterEE(byte* buff, int len);
//--------------------------------------------------
#endif
