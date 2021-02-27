#include "sensor.h"
//---------------------------------
Sensors::Sensors(pin_t* bus, int8_t nbus, unsigned delayTime) :
    nLines(nbus), delayTime(delayTime), sensors(0), oneWires(0), currReading(0)
{
  if(nLines > MAX_DEV_WIRES)
    nLines = MAX_DEV_WIRES;
  oneWires = new OneWire*[nLines];
  sensors = new DallasTemperature*[nLines];
  for(int8_t i = 0; i < nLines; ++i) {
    oneWires[i] = new OneWire(bus[i]);
    sensors[i] = new DallasTemperature(oneWires[i]);
    }
  for(int8_t i = 0; i < MAX_DEV_WIRES; ++i)
    Buff[i] = 0.0;
}
//---------------------------------
Sensors::~Sensors()
{
  for(int8_t i = 0; i < nLines; ++i) {
    delete sensors[i];
    delete oneWires[i];
    }
  delete []sensors;
  delete []oneWires;
}
//---------------------------------
void Sensors::begin()
{
  for(int8_t i = 0; i < nLines; ++i)
    sensors[i]->begin();
  delay(100);

  for(int8_t i = 0; i < nLines; ++i)
    numberOfDevices[i] = sensors[i]->getDeviceCount();
  requestTemperatures();
}
//---------------------------------
void Sensors::requestTemperatures()
{
  for(int i = 0; i < nLines; ++i)
    sensors[i]->requestTemperatures();
}
//---------------------------------
int8_t Sensors::getNumberOfLine()
{
  return nLines;
}
//---------------------------------
int8_t Sensors::getNumberOfDevices(int8_t line)
{
  return numberOfDevices[line];
}
//---------------------------------
bool Sensors::getAddr(DeviceAddress& da, int8_t ix, int8_t line)
{
  return sensors[line]->getAddress(da, ix);
}
//---------------------------------
float Sensors::getTempCByIndex(int8_t ix, int8_t line)
{
  return sensors[line]->getTempCByIndex(ix);
}
//---------------------------------
#define SIZE_ADDRESS 8
//---------------------------------
void printAddrSensor(DeviceAddress da)
{
  for(int8_t i = 0; i < SIZE_ADDRESS; ++i) {
    Serial.print(da[i], HEX);
    Serial.print(F(" "));
    }
  Serial.println();
}
//---------------------------------
void Sensors::debug(bool begin)
{
  int8_t nline = getNumberOfLine();
  if(begin) {
    Serial.print(F("Trovate "));
    Serial.print(nline);
    Serial.println(F(" linee di sensori!"));

    for(int8_t i = 0; i < nline; ++i) {
      int8_t numberOfDevices = getNumberOfDevices(i);
      Serial.print(F("Linea["));
      Serial.print(i);
      if(1 == numberOfDevices) {
        Serial.println(F("] Trovato un sensore"));
        }
      else {
        Serial.print(F("] Trovati "));
        Serial.print(numberOfDevices);
        Serial.println(F(" sensori!"));
        }
      for(int8_t j = 0; j < numberOfDevices; ++j) {
        DeviceAddress da;
        if(getAddr(da, j, i))
          printAddrSensor(da);
        }
      }
    }
  else {
    requestTemperatures(); // Comando per misurare la temp.
    for(int8_t i = 0; i < nline; ++i) {
      int8_t numberOfDevices = getNumberOfDevices(i);
      for(int8_t j = 0; j < numberOfDevices; ++j) {
        Serial.print(F("Linea["));
        Serial.print(i);
        Serial.print(F("] Sensore "));
        Serial.print(j);
        Serial.print(F(": "));
        Serial.print(getTempCByIndex(j, i));
        Serial.print(F(" °C"));
        Serial.println();
        }
      }
  }
}
//---------------------------------
int8_t Sensors::getTot()
{
  return getNumberOfLine();
}
//---------------------------------
void Sensors::loadData()
{
  static unsigned long lastTime;
  if(millis() - lastTime < delayTime)
    return;
  int8_t nline = getNumberOfLine();
  if(currReading < nline) {
    sensors[currReading]->requestTemperatures();
    if(++currReading == nline)
      currReading = 0;
    }
  for(int8_t i = 0; i < nline; ++i) {
    int8_t numberOfDevices = getNumberOfDevices(i);
    Buff[i] = 0;
    if(numberOfDevices) {
      float t = 0;
      for(int8_t j = 0; j < numberOfDevices; ++j)
        t += getTempCByIndex(j, i);
      Buff[i] = t / numberOfDevices;
      }
    }
  lastTime = millis();
}
//---------------------------------
void Sensors::fillData(float* buff)
{
  int8_t tot = getTot();
  for(int8_t i = 0; i < tot; ++i) {
    buff[i] = Buff[i];
    }
}
