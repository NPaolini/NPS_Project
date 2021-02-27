#ifndef DHT_SENSOR_H
#define DHT_SENSOR_H
//---------------------------------
#include "DHT.h"
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//---------------------------------
class DHT_Sensor
{
  public:
    DHT_Sensor(bool use_fahrenheit, pin_t pin, unsigned delayTime = 3000);
    ~DHT_Sensor();
    void begin();
    void loop(bool alsoHeat);
    float getdHumidity();
    float getTemperature();
    float getHeatIndex();
    void debug();
  private:
    bool useFahrenheit;
    unsigned delayTime;
    DHT* dht;
    float buff[3];
};
//---------------------------------
#endif
