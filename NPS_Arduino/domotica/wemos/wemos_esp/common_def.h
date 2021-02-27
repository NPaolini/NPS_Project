//--------- common_def.h ----------------------
#ifndef common_def_H
#define common_def_H
//---------------------------------------------
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Time.h>
#include <TimeLib.h>

#ifndef STASSID
#define STASSID "YOUR_SSID"
#define STAPSK  "YOUR_PASSWORD"
#endif

#include "man_eeprom.h"
#include "email_sender.h"
#include "ping.h"
#include "cookie.h"
//---------------------------------------------
#endif
