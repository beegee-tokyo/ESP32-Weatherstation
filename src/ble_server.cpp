#include "setup.h"

#define SERVICE_UUID        "1f7d75a9-d854-4559-a025-779546e79b71"
#define CHARACTERISTIC_UUID "2da23a2c-e324-423b-a23d-859366d24947"
#define NOTIFICATION_UUID "2da23a2c-e324-423b-a23d-859366d24948"
// #define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
// #define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLECharacteristic *pCharacteristic;
BLECharacteristic *pCharacteristicNotify;
BLEService *pService;
BLEServer *pServer;
bool bleConnected = false;
String bleNewData;

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
    addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " BLE read request", false);
    size_t dataLen = bleNewData.length();
    uint8_t bleData[dataLen+1];
    bleNewData.toCharArray((char *)bleData,dataLen+1);
    pCharacteristic->setValue(bleData, dataLen);
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
  uint32_t properties;
  // Initialize BLE
  BLEDevice::init(apName);
  // Create BLE Server
  BLEDevice::setPower(ESP_PWR_LVL_N14);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create BLE Service
  pService = pServer->createService(SERVICE_UUID);
  // Create BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ
                    );

  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  pCharacteristicNotify = pService->createCharacteristic(
                      NOTIFICATION_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  // Create a BLE Descriptor
  pCharacteristicNotify->addDescriptor(new BLE2902());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising* pAdvertising = pServer->getAdvertising();
  pAdvertising->start();
  // Setup callback handler
  pCharacteristic->setCallbacks(new MyCallbackHandler());

  addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " BLE active now", false);
}
