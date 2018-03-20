#ifndef setup_h
#define setup_h

/**********************************************************/
// Uncomment if the serial debug output is required
/**********************************************************/
#define ENA_DEBUG

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

// App specific includes
#include "Wire.h"
#include "MQTTClient.h"
#include "ArduinoJson.h"
#include "HTTPClient.h"
#include "WiFiClientSecure.h"
#include "Ticker.h"

#include <BLEDevice.h>
#include "BluetoothSerial.h"

// My libraries
#include "globals.h"

#endif
