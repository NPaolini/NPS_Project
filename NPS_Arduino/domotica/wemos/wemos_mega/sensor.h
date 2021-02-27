#ifndef SENSOR_H
#define SENSOR_H
//---------------------------------
#include <OneWire.h>
#include <DallasTemperature.h>
//---------------------------------
#define MAX_DEV_WIRES 5
//---------------------------------
/*
  i sensori sono raggruppati per linea, ciscuna col proprio pin. Se ci sono pi� sensori
  sulla stessa linea, ne viene fatta la media.
*/
class Sensors
{
  public:
    Sensors(pin_t* bus, int8_t nbus, unsigned delayTime = 5000);
    ~Sensors();
    void begin();
    int8_t getNumberOfLine();
    int8_t getNumberOfDevices(int8_t line);
    bool getAddr(DeviceAddress& da, int8_t ix, int8_t line);
    float getTempCByIndex(int8_t ix, int8_t line);
    void requestTemperatures();
    void debug(bool begin);
    int8_t getTot();
    void fillData(float* buff);
    void loadData();

  private:
    int8_t nLines;
    int8_t numberOfDevices[MAX_DEV_WIRES];
    DallasTemperature** sensors;
    OneWire** oneWires;
    float Buff[MAX_DEV_WIRES];
    unsigned delayTime;
    uint8_t currReading;
};
//---------------------------------
#endif
