#include "dht_sensor.h"
//---------------------------------
DHT_Sensor::DHT_Sensor(bool use_fahrenheit, pin_t pin, unsigned delayTime) :
    delayTime(delayTime), useFahrenheit(use_fahrenheit), dht(new DHT(pin, DHTTYPE))
{
  buff[0] = buff[1] = buff[2] = 0;
}
//---------------------------------
DHT_Sensor::~DHT_Sensor()
{
  delete dht;
}
//---------------------------------
void DHT_Sensor::begin()
{
  dht->begin();
}
//---------------------------------
void DHT_Sensor::loop(bool alsoHeat)
{
  static unsigned long lastTime = millis() - delayTime / 2;
  if(millis() - lastTime < delayTime)
    return;

  buff[0] = dht->readTemperature(useFahrenheit);
  buff[1] = dht->readHumidity();
  if(alsoHeat)
    buff[2] = dht->computeHeatIndex(buff[0], buff[1], useFahrenheit);
  lastTime = millis();
}
//---------------------------------
float DHT_Sensor::getdHumidity()
{
  return buff[1];
}
//---------------------------------
float DHT_Sensor::getTemperature()
{
  return buff[0];
}
//---------------------------------
float DHT_Sensor::getHeatIndex()
{
  return buff[2];
}
//---------------------------------
void DHT_Sensor::debug()
{
  loop(true);
  float t = getTemperature();
  Serial.print("Temperatura: ");
  Serial.print(t);
  float h = getdHumidity();
  Serial.print("°C - Umidità: ");
  Serial.print(h);
  Serial.print("% - Percepita: ");
  Serial.print(getHeatIndex());
  Serial.println("°C");
}
