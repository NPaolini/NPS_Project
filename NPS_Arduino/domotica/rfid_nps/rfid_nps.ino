/*
 * Write personal data of a MIFARE RFID card using a RFID-RC522 reader
 * Uses MFRC522 - Library to use ARDUINO RFID MODULE KIT 13.56 MHZ WITH TAGS SPI W AND R BY COOQROBOT.
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno/101       Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 *
 * Hardware required:
 * Arduino
 * PCD (Proximity Coupling Device): NXP MFRC522 Contactless Reader IC
 * PICC (Proximity Integrated Circuit Card): A card or tag using the ISO 14443A interface, eg Mifare or NTAG203.
 * The reader can be found on eBay for around 5 dollars. Search for "mf-rc522" on ebay.com.
 */

#include <SPI.h>
#include <MFRC522.h>
#include <EEPROM.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above

#define MAX_UID 16

#define PIN_LED_READ  7
#define PIN_LED_WRITE 6

#define BLOCK_4_CODE 8
#define DIM_CODE 16
const byte myKey[] = { 'n' + 1, 'p' + 2, 'e' + 3, 's' + 5, 's' + 8, 'e' + 13 };

#define DIM_KEY 6
#define DIM_PERM 4

#define NUM_CHAR_4_RESET 4

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance
byte onProgr = 0;
byte stdEE_data[32] = { 'N', 'P', 'S', 0 };
int addr_EE_data = 0;

#define DIM_HEADER 3 // -> NPS
#define OFFS_DIM_UID DIM_HEADER
#define OFFS_DATA_UID (OFFS_DIM_UID + 1)

#define FALSE_LEN 1

#define CHAR_TO_SAVE_CARD  '$'
#define CHAR_TO_EMU_CARD   '@'
#define CHAR_TO_RESET_CARD '#'
#define CHAR_TO_RESET_KEY  '&'
#define CHAR_TO_GET_VER    '^'

#define SOFTWARE_VER "1.0.0"
//--------------------------------------------------
void checkCommandBySerial();
void writeMyKey();
void lampLed();
bool isMaster();
bool runProgr();
bool runRead();
byte readMasterEE(byte* buff);
void writeMasterEE(byte* buff, byte len);
//--------------------------------------------------
void setup()
{
  Serial.begin(9600);        // Initialize serial communications with the PC
  SPI.begin();               // Init SPI bus
  mfrc522.PCD_Init();        // Init MFRC522 card
  pinMode(PIN_LED_WRITE, OUTPUT);
  pinMode(PIN_LED_READ, OUTPUT);
  digitalWrite(PIN_LED_WRITE, LOW);
  digitalWrite(PIN_LED_READ, LOW);
}
//--------------------------------------------------
void loop()
{
  if(Serial.available() > 0) {
    checkCommandBySerial();
    mfrc522.PICC_HaltA(); // Halt PICC
    mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
    return;
    }
  // Look for new cards
  if(!mfrc522.PICC_IsNewCardPresent()) return;

  // Select one of the cards
  if(!mfrc522.PICC_ReadCardSerial()) return;

  if(isMaster()) {
    if(onProgr) {
      digitalWrite(PIN_LED_WRITE, LOW);
      Serial.println(F("----------"));
      Serial.println(F("Fine programmazione"));
      }
    else {
      digitalWrite(PIN_LED_WRITE, HIGH);
      Serial.println(F("Inizio ciclo di programmazione"));
      Serial.println(F("Accostare card da programmare al lettore"));
      }
    onProgr = !onProgr;
    }
  else {
    digitalWrite(PIN_LED_READ, HIGH);
    bool onError = false;
    if(onProgr)
      onError = !runProgr();
    else
      onError = !runRead();
    if(onError) {
      lampLed();
      delay(1000L);
      }
    else {
      delay(500L);
      digitalWrite(PIN_LED_READ, LOW);
      }
    }
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1();  // Stop encryption on PCD
}
//--------------------------------------------------
void makeMyKey(MFRC522::MIFARE_Key& key)
{
  for (byte i = 0; i < DIM_KEY; i++)
    key.keyByte[i] = myKey[i];
}
//--------------------------------------------------
bool writeGenKey(MFRC522::MIFARE_Key& keyAuth, MFRC522::MIFARE_Key& keyNew, bool restart)
{
  byte block = BLOCK_4_CODE + 3;
  byte buffer[DIM_CODE * 2];
  byte size = DIM_CODE * 2;

  MFRC522::StatusCode status;
  if(restart)
    mfrc522.PICC_HaltA();
  int i = 0;
  for(; i < 10; ++i) {
    if(mfrc522.PICC_IsNewCardPresent() && mfrc522.PICC_ReadCardSerial())
      break;
    delay(100L);
    }
  if(i >= 10) {
    Serial.println(F("No card ..."));
    return false;
    }
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &keyAuth, &(mfrc522.uid));
  if(status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_AuthenticateKeyA() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
    }

  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if(status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_ReadKeyByKeyA() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
    }
  for(int i = 0; i < DIM_KEY; ++i) {
    buffer[i] = keyNew.keyByte[i];
    buffer[i + DIM_KEY + DIM_PERM] = keyNew.keyByte[i];
    }
  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, DIM_CODE);
  if(status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_WriteKeyByKeyA() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
    }

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, block, &keyNew, &(mfrc522.uid));
  if(status != MFRC522::STATUS_OK) {
    Serial.print(F("PCD_AuthenticateKeyB() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
    }
  // Write block
  status = mfrc522.MIFARE_Write(block, buffer, DIM_CODE);
  if(status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_WriteKeyByKeyB() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
    }

  Serial.println(F("MIFARE_WriteKey() success: "));
  return true;
}
//--------------------------------------------------
void writeMyKey()
{
  MFRC522::MIFARE_Key keyAuth;
  for (byte i = 0; i < DIM_KEY; i++)
    keyAuth.keyByte[i] = 0xFF;
  MFRC522::MIFARE_Key keyNew;
  makeMyKey(keyNew);
  writeGenKey(keyAuth, keyNew, true);
}
//--------------------------------------------------
bool reset_key()
{
  MFRC522::MIFARE_Key keyNew;
  MFRC522::MIFARE_Key keyAuth;
  for (byte i = 0; i < DIM_KEY; i++)
    keyNew.keyByte[i] = 0xFF;
  makeMyKey(keyAuth);
  Serial.setTimeout(10000L);
  Serial.println(F("Accostare card da resettare e premere il tasto '#' ..."));
  byte buffer[32];
  Serial.readBytesUntil('#', buffer, sizeof(buffer) - 1) ;
  bool success = writeGenKey(keyAuth, keyNew, false);
  Serial.println(F("Allontanare card e premere il tasto '#' ..."));
  Serial.readBytesUntil('#', buffer, sizeof(buffer) - 1) ;
  Serial.setTimeout(1000L);
  return success;
}
//--------------------------------------------------
void set_emu_master()
{
  stdEE_data[OFFS_DIM_UID] = FALSE_LEN;
}
//--------------------------------------------------
void remove_emu_master()
{
  byte val;
  for(int i = 0; i < DIM_HEADER; ++i) {
    val = EEPROM.read(addr_EE_data + i);
    if(val != stdEE_data[i]) {
      stdEE_data[OFFS_DIM_UID] = 0;
      return;
      }
    }
  stdEE_data[OFFS_DIM_UID] = EEPROM.read(addr_EE_data + OFFS_DIM_UID);
}
//--------------------------------------------------
void reset_master()
{
  EEPROM.update(addr_EE_data + OFFS_DIM_UID, 0);
  stdEE_data[OFFS_DIM_UID] = 0;
}
//--------------------------------------------------
void save_master()
{
  for(int i = 0; i < DIM_HEADER; ++i)
    EEPROM.update(addr_EE_data + i, stdEE_data[i]);
  EEPROM.update(addr_EE_data + OFFS_DIM_UID, FALSE_LEN);
  stdEE_data[OFFS_DIM_UID] = FALSE_LEN;
}
//--------------------------------------------------
void lampLed()
{
  for(int i = 0; i < 5; ++i) {
    delay(100L);
    digitalWrite(PIN_LED_WRITE, LOW);
    digitalWrite(PIN_LED_READ, HIGH);
    delay(100L);
    digitalWrite(PIN_LED_READ, LOW);
    digitalWrite(PIN_LED_WRITE, HIGH);
    }
  if(!onProgr)
    digitalWrite(PIN_LED_WRITE, LOW);
}
//--------------------------------------------------
bool checkCommon(byte* buff, size_t dim, byte match)
{
  for(size_t i = 0; i < dim; ++i)
    if(match != buff[i])
      return false;
  return true;
}
//--------------------------------------------------
bool checkSetMaster(byte* buff, size_t dim)
{
  if(checkCommon(buff, dim, CHAR_TO_SAVE_CARD)) {
    save_master();
    Serial.println(F("Impostata Master Card fittizia."));
    return true;
    }
  return false;
}
//--------------------------------------------------
bool checkReset(byte* buff, size_t dim)
{
  if(checkCommon(buff, dim, CHAR_TO_RESET_CARD)) {
    reset_master();
    Serial.println(F("Master Card resettata. La prima card letta diventera' la nuova Master Card."));
    return true;
    }
  return false;
}
//--------------------------------------------------
bool checkProgr(byte* buff, size_t dim)
{
  if(checkCommon(buff, dim, CHAR_TO_EMU_CARD)) {
    if(onProgr) {
      remove_emu_master();
      Serial.println(F("Rimossa emulazione Master Card. Modalita' lettura."));
      digitalWrite(PIN_LED_WRITE, LOW);
      }
    else {
      digitalWrite(PIN_LED_WRITE, HIGH);
      set_emu_master();
      Serial.println(F("Impostata emulazione Master Card. Modalita' programmazione."));
      }
    onProgr = !onProgr;
    return true;
    }
  return false;
}
//--------------------------------------------------
bool checkResetKey(byte* buff, size_t dim)
{
  if(checkCommon(buff, dim, CHAR_TO_RESET_KEY)) {
    if(reset_key())
      Serial.println(F("Reset Key, impostata key di default."));
    return true;
    }
  return false;
}
//--------------------------------------------------
bool checkGetVer(byte* buff, size_t dim)
{
  if(checkCommon(buff, dim, CHAR_TO_GET_VER)) {
    Serial.print(F("NPS System RFID reader, Ver. "));
    Serial.println(SOFTWARE_VER);
    return true;
    }
  return false;
}
//--------------------------------------------------
void checkCommandBySerial()
{
  for(int i = 0; i < 10; ++i) {
    if(Serial)
      break;
    delay(10);
    }
  Serial.setTimeout(2000L);
  byte vals[NUM_CHAR_4_RESET];
  if(sizeof(vals) == Serial.readBytes(vals, sizeof(vals))) {
    do {
      if(checkProgr(vals, sizeof(vals)))
        break;
      if(checkReset(vals, sizeof(vals)))
        break;
      if(checkSetMaster(vals, sizeof(vals)))
        break;
      if(checkResetKey(vals, sizeof(vals)))
        break;
      if(checkGetVer(vals, sizeof(vals)))
        break;
      } while(false);

    // vuota eventuali byte in piu', o casuali
    for(;;) {
      if(Serial.available() <= 0)
        break;
      Serial.read();
      }
    }
}
//--------------------------------------------------
bool isMaster()
{
  byte buff[64];
  byte len = readMasterEE(buff);
  if(len != mfrc522.uid.size)
    return false;
  for(int i = 0; i < len; ++i)
    if(buff[i] != mfrc522.uid.uidByte[i])
      return false;
  return true;
}
//--------------------------------------------------
bool runProgr()
{
  Serial.setTimeout(10000L);
  Serial.println(F("----------"));
  Serial.print(F("Inviare codice da scrivere ("));
  Serial.print(DIM_CODE);
  Serial.println(F(" byte):##"));
  byte buff[DIM_CODE + 4];
  if(Serial.readBytes(buff, DIM_CODE) != DIM_CODE) {
    Serial.println(F("Errore in lettura codice"));
    return false;
    }
  Serial.print(F("Codice: "));
  Serial.write(buff, DIM_CODE);
  Serial.println();

  byte block = BLOCK_4_CODE;
  MFRC522::StatusCode status;
  MFRC522::MIFARE_Key key;
  makeMyKey(key);
  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if(status != MFRC522::STATUS_OK) {
    writeMyKey();
    makeMyKey(key);
    status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
    if(status != MFRC522::STATUS_OK) {
      Serial.print(F("PCD_Authenticate() failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
      return false;
      }
    }
  // Write block
  status = mfrc522.MIFARE_Write(block, buff, DIM_CODE);
  if(status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    return false;
    }
  Serial.println(F("MIFARE_Write(): success"));
  Serial.setTimeout(1000L);
  return true;
}
//--------------------------------------------------
#define SHOW_ERR_READ
#ifdef SHOW_ERR_READ
#define PrintErr(a, b) printErr((a), (b))
void printErr(const char* info, MFRC522::StatusCode status)
{
  mfrc522.PICC_HaltA();
  Serial.print(F("#"));
  Serial.print(info);
  Serial.print(mfrc522.GetStatusCodeName(status));
  Serial.println(F("#"));
}
#else
#define PrintErr(a, b) printErr()
void printErr()
{
// in lettura card deve tornare il codice letto o un codice non valido
  mfrc522.PICC_HaltA();
  Serial.println(F("##"));
}
#endif
//--------------------------------------------------
bool runRead()
{
  byte block = BLOCK_4_CODE;
  MFRC522::StatusCode status;
  MFRC522::MIFARE_Key key;
  makeMyKey(key);

  status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if(status != MFRC522::STATUS_OK) {
    PrintErr("PCD_Authenticate() failed: ", status);
    return false;
    }
  byte size = DIM_CODE*2;
  byte buffer[DIM_CODE*2];
  status = mfrc522.MIFARE_Read(block, buffer, &size);
  if(status != MFRC522::STATUS_OK) {
    PrintErr("MIFARE_Read() failed: ", status);
    return false;
    }
  Serial.write(buffer, DIM_CODE);
  Serial.println();
  return true;
}
//--------------------------------------------------
byte initializeEE(byte* buff)
{
  for(int i = 0; i < DIM_HEADER; ++i)
    EEPROM.update(addr_EE_data + i, stdEE_data[i]);
  byte len = mfrc522.uid.size;
  for(int i = 0; i < len; ++i)
    buff[i] =  mfrc522.uid.uidByte[i];
  writeMasterEE(buff, len);
  return len;
}
//--------------------------------------------------
byte readMasterEE(byte* buff)
{
  byte len = stdEE_data[OFFS_DIM_UID];
  if(len) {
    for(int i = 0; i < len; ++i)
      buff[i] = stdEE_data[OFFS_DATA_UID + i];
    return len;
    }

  byte val;
  for(int i = 0; i < DIM_HEADER; ++i) {
    val = EEPROM.read(addr_EE_data + i);
    if(val != stdEE_data[i]) {
      return initializeEE(buff);
      }
    }
  val = EEPROM.read(addr_EE_data + OFFS_DIM_UID);
  if(!val || val > MAX_UID)
    return initializeEE(buff);
  // anziche' leggere sempre dalla EEPROM si usa 'stdEE_data'
  stdEE_data[OFFS_DIM_UID] = val;
  for(int i = 0; i < val; ++i) {
    buff[i] = EEPROM.read(addr_EE_data + OFFS_DATA_UID + i);
    stdEE_data[OFFS_DATA_UID + i] = buff[i];
    }
  return val;
}
//--------------------------------------------------
void writeMasterEE(byte* buff, byte len)
{
  EEPROM.update(addr_EE_data + OFFS_DIM_UID, len);
  stdEE_data[OFFS_DIM_UID] = len;
  for(int i = 0; i < len; ++i) {
    EEPROM.update(addr_EE_data + OFFS_DATA_UID + i, buff[i]);
    stdEE_data[OFFS_DATA_UID + i] = buff[i];
    }
}
//--------------------------------------------------
