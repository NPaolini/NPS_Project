#ifndef pin_def_H
#define pin_def_H
//------------------------------
typedef uint16_t pin_t;
//------------------------------
#define RFID_RST_PIN         49           // Configurable, see typical pin layout above
#define RFID_SS_PIN          53          // Configurable, see typical pin layout above

#define RFID_BLOCK_4_CODE 8
#define RFID_DIM_CODE 16
#define RFID_DIM_KEY 6

#define RFID_DIM_ONLY_CODE (RFID_DIM_CODE - 6)
#define RFID_OFFS_ONLY_CODE 3
//------------------------------
#define PIN_SD_CARD1  4
#define PIN_SD_CARD2  12
#define PIN_ET_CARD  10
//------------------------------
#define PIN_SHIFT_REG_LATCH 45
#define PIN_SHIFT_REG_DATA  51
#define PIN_SHIFT_REG_CLK   52
//----Adafruit -----------------
#define TFT_CS        48
#define TFT_RST       47 // Or set to -1 and connect to Arduino RESET pin
#define TFT_DC        46
// sono di default per comunicazione hardware
//#define TFT_MOSI      51
//#define TFT_SCLK      52
// se si usa la comunicazione software ...
//#define TFT_MOSI      12
//#define TFT_SCLK      13
//------------------------------
#define RTC_ce_pin    26
#define RTC_data_pin  24
#define RTC_sclk_pin  22
//------------------------------
// se il pin da utilizzare è minore di OFFS_MPX_1 allora ci si riferisce all'arduino,
// altrimenti, fatti gli opportuni calcoli, al multiplexer corrispondente
// max 16 -> 0-15 per ogni blocco
//------------------------------
#define OFFS_MPX_1 100
#define DELTA_MPX  40
#define OFFS_MPX_2 (OFFS_MPX_1 + DELTA_MPX)
#define OFFS_MPX_3 (OFFS_MPX_2 + DELTA_MPX)
#define OFFS_MPX_4 (OFFS_MPX_3 + DELTA_MPX)
//------------------------------
#define O_MPX_1(a) ((a) + OFFS_MPX_1)
#define O_MPX_2(a) ((a) + OFFS_MPX_2)
#define O_MPX_3(a) ((a) + OFFS_MPX_3)
#define O_MPX_4(a) ((a) + OFFS_MPX_4)
//------------------------------
#define REM_MPX_1(a) ((a) - OFFS_MPX_1)
#define REM_MPX_2(a) ((a) - OFFS_MPX_2)
#define REM_MPX_3(a) ((a) - OFFS_MPX_3)
//------------------------------
// come sopra ma per gli shift register, ne serve uno blocco solo perché possono essere messi in cascata
#define OFFS_SR_1 OFFS_MPX_1
#define O_SR_1(a) ((a) + OFFS_SR_1)
#define REM_SR_1(a) ((a) - OFFS_SR_1)

#define O_SR_2(a) (O_SR_1(a) + 8)
#define O_SR_3(a) (O_SR_2(a) + 8)
#define O_SR_4(a) (O_SR_3(a) + 8)
#define O_SR_5(a) (O_SR_4(a) + 8)
#define O_SR_6(a) (O_SR_5(a) + 8)
#define O_SR_7(a) (O_SR_6(a) + 8)
#define O_SR_8(a) (O_SR_7(a) + 8)
//------------------------------
// sensore infrarosso PIR HC-SR501
#define ALARM_SENSOR_INP_1 O_MPX_1(0)
#define ALARM_SENSOR_INP_2 O_MPX_1(1)
#define ALARM_SENSOR_INP_3 O_MPX_1(2)
#define ALARM_SENSOR_INP_4 O_MPX_1(3)
#define ALARM_SENSOR_INP_5 O_MPX_1(4)
#define ALARM_SENSOR_INP_6 O_MPX_1(5)
#define ALARM_SENSOR_INP_7 O_MPX_1(6)
#define ALARM_SENSOR_INP_8 O_MPX_1(7)

#define ALARM_PIN_LED1 O_SR_2(0)
#define ALARM_PIN_LED2 O_SR_2(1)
#define ALARM_PIN_LED3 O_SR_2(2)

#define ALARM_PIN_OUT O_SR_3(1)
#define ALARM_PIN_LED_DELAY O_SR_3(2)
//------------------------------
// interruttori luci
#define RELE_INP_1 O_MPX_2(0)
#define RELE_INP_2 O_MPX_2(1)
#define RELE_INP_3 O_MPX_2(2)
#define RELE_INP_4 O_MPX_2(3)
#define RELE_INP_5 O_MPX_2(4)
#define RELE_INP_6 O_MPX_2(5)
#define RELE_INP_7 O_MPX_2(6)
#define RELE_INP_8 O_MPX_2(7)
#define RELE_INP_9 O_MPX_2(8)
//------------------------------
#define KEYB_READ1 O_MPX_2(11)
#define KEYB_READ2 O_MPX_2(12)
#define KEYB_READ3 O_MPX_2(13)
#define KEYB_READ4 O_MPX_2(14)
#define KEYB_READ5 O_MPX_2(15)

#define KEYB_WRITE1 O_SR_2(4)
#define KEYB_WRITE2 O_SR_2(5)
#define KEYB_WRITE3 O_SR_2(6)
#define KEYB_WRITE4 O_SR_2(7)
//------------------------------
// input tapparelle
// pulsanti
#define MOTOR_INP_1_U O_MPX_1(8) // up
#define MOTOR_INP_1_D O_MPX_1(9) // down
#define MOTOR_INP_2_U O_MPX_1(10)
#define MOTOR_INP_2_D O_MPX_1(11)
#define MOTOR_INP_3_U O_MPX_1(12)
#define MOTOR_INP_3_D O_MPX_1(13)
#define MOTOR_INP_4_U O_MPX_1(14)
#define MOTOR_INP_4_D O_MPX_1(15)
//------------------------------
// output tapparelle
#define MOTOR_OUT_1_U O_SR_4(0)
#define MOTOR_OUT_1_D O_SR_4(1)
#define MOTOR_OUT_2_U O_SR_4(2)
#define MOTOR_OUT_2_D O_SR_4(3)
#define MOTOR_OUT_3_U O_SR_4(4)
#define MOTOR_OUT_3_D O_SR_4(5)
#define MOTOR_OUT_4_U O_SR_4(6)
#define MOTOR_OUT_4_D O_SR_4(7)
//------------------------------
// rele luci o altro
#define RELE_OUT_1 O_SR_1(0)
#define RELE_OUT_2 O_SR_1(1)
#define RELE_OUT_3 O_SR_1(2)
#define RELE_OUT_4 O_SR_1(3)
#define RELE_OUT_5 O_SR_1(4)
#define RELE_OUT_6 O_SR_1(5)
#define RELE_OUT_7 O_SR_1(6)
#define RELE_OUT_8 O_SR_1(7)

#define CRONO_TERM_OUT O_SR_3(0)
//------------------------------
#define SENSOR_ONWIRE_1 23
#define SENSOR_ONWIRE_2 25
#define SENSOR_ONWIRE_3 27

#define SENSOR_DHT_1  36

//#define LCD_SDA 20
//#define LCD_SCL 21
//------------------------------
#define MPLEX_1_EN  29
#define MPLEX_1_S1  28
#define MPLEX_1_S2  30
#define MPLEX_1_S3  32
#define MPLEX_1_S4  34
#define MPLEX_1_Z   31

#define MPLEX_2_EN  33
#define MPLEX_2_S1  MPLEX_1_S1
#define MPLEX_2_S2  MPLEX_1_S2
#define MPLEX_2_S3  MPLEX_1_S3
#define MPLEX_2_S4  MPLEX_1_S4
#define MPLEX_2_Z   35
/*
#define MPLEX_3_EN  46
#define MPLEX_3_S1  MPLEX_1_S1
#define MPLEX_3_S2  MPLEX_1_S2
#define MPLEX_3_S3  MPLEX_1_S3
#define MPLEX_3_S4  MPLEX_1_S4
#define MPLEX_3_Z   7
*/
//------------------------------
#endif
