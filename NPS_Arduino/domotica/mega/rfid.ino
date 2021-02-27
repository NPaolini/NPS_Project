#include "rfid.h"
/*
 * Write personal data of a MIFARE RFID card using a RFID-RC522 reader
 * Uses MFRC522 - Library to use ARDUINO RFID MODULE KIT 13.56 MHZ WITH TAGS SPI W AND R BY COOQROBOT.
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             49         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
*/
//---------------------------------
MFRC522 mfrc522(RFID_SS_PIN, RFID_RST_PIN);   // Create MFRC522 instance
//---------------------------------
const byte myKey[] = { 'n' + 1, 'p' + 2, 'e' + 3, 's' + 5, 's' + 8, 'e' + 13 };
//--------------------------------------------------
void makeMyKey(MFRC522::MIFARE_Key& key)
{
  for (byte i = 0; i < RFID_DIM_KEY; i++)
    key.keyByte[i] = myKey[i];
}
//---------------------------------
rfid::rfid() {}
//---------------------------------
rfid::~rfid() {}
//---------------------------------
void rfid::begin()
{
  enableRFID();
  mfrc522.PCD_Init();
}
//---------------------------------
#define TIME_TO_RESET (1000L * 60)
//---------------------------------
void rfid::loop()
{
  enableRFID();
  static unsigned long old;
  if(millis() - old > TIME_TO_RESET) {
    old = millis();
    mfrc522.PCD_Init();
    }
  if(!mfrc522.PICC_IsNewCardPresent())
    return;
  // Select one of the cards
  if(!mfrc522.PICC_ReadCardSerial())
    return;
  DEBUG_RFID(Serial.println("Enter read"))
  runRead();
  enableRFID();
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
}
//--------------------------------------------------
//--------------------------------------------------
#define SHOW_ERR_READ
#ifdef SHOW_ERR_READ
#define PrintErr(a, b) printErr((a), (b))
template<typename T> void printErr(T info, MFRC522::StatusCode status)
{
  mfrc522.PICC_HaltA();
  if(useDebug & D_BIT_RFID) {
    Serial.print(F("#"));
    Serial.print(info);
    Serial.print(mfrc522.GetStatusCodeName(status));
    Serial.println(F("#"));
    }
}
#else
#define PrintErr(a, b) printErr()
void printErr()
{
// in lettura card deve tornare il codice letto o un codice non valido
  mfrc522.PICC_HaltA();
  DEBUG_RFID(Serial.println(F("##")))
}
#endif
//--------------------------------------------------
const char* extractCode(char* rfid_code, const char* rfid)
{
  strcpy(rfid_code, rfid + 3);
  rfid_code[10] = 0;
  return rfid_code;
}
//--------------------------------------------------
const char* fromWhereToChar(uint8_t fromWhere, const char* rfid)
{
  static char rfid_code[12];
  switch(fromWhere) {
    case 1:
      return "from Web";
    case 2:
      return rfid;
    case 3:
      return extractCode(rfid_code, rfid);
    }
  return "???";
}
//--------------------------------------------------
// fromWhere -> 1 = web, 2 = rfid, 3 = keyb
void writeLogAccess(uint8_t ix_user, uint8_t fromWhere)
{
  infoSetup* isetup = getISetup();
  const char* pu = ix_user >= MAX_USER ? "???" : isetup->users[ix_user].u;
  const char* ena = Alarm.isEnabled() ? "enabled by" : "disabled by";
  const char* clog[] = { ena, pu, ix_user >= MAX_USER ? "???" : fromWhereToChar(fromWhere, isetup->users[ix_user].rfid_code) };
  man_log ml(man_log::eL_Access);
  ml.logSend(clog, SIZE_A(clog));
}
//--------------------------------------------------
bool isValidKey(const char* code, const char* enter)
{
  for(uint8_t i = 0; i < RFID_DIM_CODE; ++i)
    if(code[i] != enter[i])
      return false;
  return true;
}
//--------------------------------------------------
//--------------------------------------------------
bool isValidKeyReduced(const char* code, const char* enter)
{
  for(uint8_t i = 0; i < RFID_DIM_ONLY_CODE; ++i)
    if(code[i + RFID_OFFS_ONLY_CODE] != enter[i])
      return false;
  return true;
}
//--------------------------------------------------
bool runRead()
{
  byte block = RFID_BLOCK_4_CODE;
  MFRC522::StatusCode status;
  MFRC522::MIFARE_Key key;
  makeMyKey(key);

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if(status != MFRC522::STATUS_OK) {
    PrintErr(F("PCD_Authenticate() failed: "), status);
    return false;
    }
  byte size = RFID_DIM_CODE + 2;
  byte buffer[RFID_DIM_CODE + 2];
  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if(status != MFRC522::STATUS_OK) {
    PrintErr(F("MIFARE_Read() failed: "), status);
    return false;
    }
  infoSetup* isetup = getISetup();
  DEBUG_RFID(Serial.write(buffer, RFID_DIM_CODE))
  DEBUG_RFID(Serial.println())
  uint8_t j = 0;
  for(; j < MAX_USER; ++j) {
    if(isValidKey(isetup->users[j].rfid_code, buffer))
      break;
    }
  if(MAX_USER == j)
    return false;
  Alarm.enable(!Alarm.isEnabled());
  writeLogAccess(j, 2);
  return true;
}
