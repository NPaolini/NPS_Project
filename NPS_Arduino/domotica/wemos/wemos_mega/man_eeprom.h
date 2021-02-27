#ifndef eeprom_H
#define eeprom_H
//--------------------------------------------------
#include "domotica.h"
//--------------------------------------------------
void initializeEE(infoSetup* is);
bool readMasterEE(infoSetup* is);
void writeMasterEE(void* buff, int len);
void readCronoEE(void* buff, int8_t dow);
void writeCronoEE(void* buff, int8_t dow);
void readCronoEESingle(void* buff, int8_t* adr, int8_t num, int8_t dow);
void writeCronoEESingle(void* buff, int8_t* adr, int8_t num, int8_t dow);
#endif
