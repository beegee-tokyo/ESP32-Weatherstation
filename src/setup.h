#ifndef setup_h
#define setup_h

/**********************************************************/
// Uncomment if the serial debug output is required
/**********************************************************/
#define ENA_DEBUG
/**********************************************************/
// Select one WiFi connection mode
// If changes to the standard are needed change in
// bgConnect.cpp
/**********************************************************/
#define CONNDIRECT // connect with pre-defined SSID and password
// #define CONNWIFIMANAGER // connect using the WiFiManager
// #define CONNSMARTCONFIG // connect using SmartConfig

// Standard libraries
#include <Arduino.h>
#include "WiFi.h"
#include "ESPmDNS.h"
#include "WiFiUdp.h"
#include "ArduinoOTA.h"
#include "esp_system.h"
#include "rom/rtc.h"

// App specific includes
#include "TFT_eSPI.h"
#ifdef CONNWIFIMANAGER // WiFiManager used for intial WiFi connection?
	#include "DNSServer.h"
	#include "WebServer.h"
	#include "WiFiManager.h"
#endif

// App specific includes
#include "Wire.h"
#include "MQTTClient.h"
// #include "FS.h"
// #include "SPIFFS.h"
// #include "ESP8266FtpServer.h"
#include "ArduinoJson.h"
#include "HTTPClient.h"
#include "WiFiClientSecure.h"

#include <BLEDevice.h>

// My libraries
#include "myLib.h"
#include "ESP32Ticker.h"
#include "globals.h"

#endif
