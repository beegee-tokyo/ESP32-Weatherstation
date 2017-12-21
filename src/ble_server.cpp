#include "setup.h"

#define SERVICE_UUID        0x181A
#define NOTIFICATION_UUID   0x2A08
#define TEMP_UUID           0x2A1F
#define HUMID_UUID          0x2A6F
#define STATUS_UUID         0x2A3D

BLECharacteristic *pCharacteristicNotify;
BLECharacteristic *pCharacteristicTemp;
BLECharacteristic *pCharacteristicHumid;
BLECharacteristic *pCharacteristicStatus;

BLEUUID pNotifUUID;

BLEAdvertising* pAdvertising;
BLEService *pService;
BLEServer *pServer;

bool bleConnected = false;
float bleTemperature;
float bleHumidity;
String bleStatus;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      bleConnected = true;
      addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " BLE connected", false);
    };

    void onDisconnect(BLEServer* pServer) {
      bleConnected = false;
      addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " BLE disconnected", false);
    }
};

class MyCallbackHandler: public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic* pCharacteristic) {
    // addMqttMsg("debug", " " + digitalTimeDisplaySec() + " BLE read request", false);
    // addMqttMsg("debug", " " + digitalTimeDisplaySec() + " BLE sending: " + bleNewData, false);
    if (pCharacteristic == pCharacteristicNotify) {
      uint8_t notifData[8];
      time_t now;
      struct tm timeinfo;
    	time(&now); // get time (as epoch)
    	localtime_r(&now, &timeinfo); // update tm struct with current time
      uint16_t year = timeinfo.tm_year+1900;
      notifData[1] = year>>8;
      notifData[0] = year;
      notifData[2] = timeinfo.tm_mon+1;
      notifData[3] = timeinfo.tm_mday;
      notifData[4] = timeinfo.tm_hour;
      notifData[5] = timeinfo.tm_min;
      notifData[6] = timeinfo.tm_sec;
      pCharacteristic->setValue(notifData, 8);
    } else if (pCharacteristic == pCharacteristicTemp) {
      uint8_t tempData[2];
      uint16_t bleTemp100 = (uint16_t)(bleTemperature*10);
      tempData[1] = bleTemp100>>8;
      tempData[0] = bleTemp100;
      pCharacteristic->setValue(tempData, 2);
    } else if (pCharacteristic == pCharacteristicHumid) {
      uint8_t humidData[2];
      uint16_t bleHumid100 = (uint16_t)(bleHumidity*100);
      humidData[1] = bleHumid100>>8;
      humidData[0] = bleHumid100;
      pCharacteristic->setValue(humidData, 2);
    } else if (pCharacteristic == pCharacteristicStatus) {
      size_t dataLen = bleStatus.length();
      uint8_t bleData[dataLen+1];
      bleStatus.toCharArray((char *)bleData,dataLen+1);
      pCharacteristic->setValue(bleData, dataLen);
    }
  }

  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    int len = value.length();
    String strValue = "";

    if (value.length() > 0) {
      Serial.println("*********");
      Serial.print("New value: ");
      for (int i = 0; i < value.length(); i++) {
        Serial.print(value[i]);
        strValue += value[i];
      }
      Serial.println();
      Serial.println("*********");
      addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " BLE received: " + strValue, false);
    }
  }
};

void initBLE() {

  addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " BLE initBLE()", false);

  // Initialize BLE
  BLEDevice::init(apName);
  BLEDevice::setPower(ESP_PWR_LVL_P7);

  // Create BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create BLE Service
  pService = pServer->createService(BLEUUID((uint16_t)SERVICE_UUID));

  // Create BLE Characteristic for Alert
  pCharacteristicNotify = pService->createCharacteristic(
                      BLEUUID((uint16_t)NOTIFICATION_UUID),
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Create a BLE Descriptor for Alert
  pCharacteristicNotify->addDescriptor(new BLE2902());

  // Create BLE Characteristic for Temperature
  pCharacteristicTemp = pService->createCharacteristic(
                      BLEUUID((uint16_t)TEMP_UUID),
                      BLECharacteristic::PROPERTY_READ
                    );

  // Create BLE Characteristic for Humidity
  pCharacteristicHumid = pService->createCharacteristic(
                      BLEUUID((uint16_t)HUMID_UUID),
                      BLECharacteristic::PROPERTY_READ
                    );

  // Create BLE Characteristic for Status
  pCharacteristicStatus = pService->createCharacteristic(
                      BLEUUID((uint16_t)STATUS_UUID),
                      BLECharacteristic::PROPERTY_READ
                    );

  // Start the service
  pService->start();

  // Start advertising
  pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  // Setup callback handler
  pCharacteristicNotify->setCallbacks(new MyCallbackHandler());
  pCharacteristicTemp->setCallbacks(new MyCallbackHandler());
  pCharacteristicHumid->setCallbacks(new MyCallbackHandler());
  pCharacteristicStatus->setCallbacks(new MyCallbackHandler());

  addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " BLE active now", false);
}

void bleStop() {
  pAdvertising->stop();
}
