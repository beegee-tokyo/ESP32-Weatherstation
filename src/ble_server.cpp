#include "setup.h"
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <BLEAdvertising.h>

// List of Service and Characteristic UUIDs
#define SERVICE_UUID        0x181A
#define NOTIFICATION_UUID   0x2A08
#define TEMP_UUID           0x2A6E
#define HUMID_UUID          0x2A6F
#define HEAT_UUID           0x2A7A
#define DEW_UUID            0x2A7B
#define COMFORT_UUID        "00002A3D-ead2-11e7-80c1-9a214cf093ae" // same as String characteristic 0x2A3D
#define PERCEPTION_UUID     "10002A3D-ead2-11e7-80c1-9a214cf093ae" //	same as String characteristic 0x2A3D
#define OUTPUT_UUID         0x2A57
#define DEVICENAME_UUID     0x2A00

/** Characteristic for client notification */
BLECharacteristic *pCharacteristicNotify;
/** Characteristic for temperature in celsius */
BLECharacteristic *pCharacteristicTemp;
/** Characteristic for humidity in percent */
BLECharacteristic *pCharacteristicHumid;
/** Characteristic for heat index in celsius */
BLECharacteristic *pCharacteristicHeatIndex;
/** Characteristic for dew point in celsius */
BLECharacteristic *pCharacteristicDewPoint;
/** Characteristic for environment comfort status */
BLECharacteristic *pCharacteristicComfort;
/** Characteristic for environment perception status */
BLECharacteristic *pCharacteristicPerception;
/** Characteristic for digital output */
BLECharacteristic *pCharacteristicOutput;
/** Characteristic for device name */
BLECharacteristic *pCharacteristicDeviceName;

/** BLE Advertiser */
BLEAdvertising* pAdvertising;
/** BLE Service */
BLEService *pService;
/** BLE Server */
BLEServer *pServer;

/** Flag if a client is connected */
bool bleConnected = false;

/** Digital output value received from the client */
uint8_t digitalOut = 0;
/** Flag for change in digital output value */
bool digOutChanged = false;

/**
 * MyServerCallbacks
 * Callbacks for client connection and disconnection
 */
class MyServerCallbacks: public BLEServerCallbacks {
	// TODO this doesn't take into account several clients being connected
		void onConnect(BLEServer* pServer) {
			bleConnected = true;
			// addMqttMsg(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE connected #" + String(pServer->getConnId()), false);
			pAdvertising->start();
		};

		void onDisconnect(BLEServer* pServer) {
			if (pServer->getConnectedCount() == 0) {
				bleConnected = false;
			}
			// addMqttMsg(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE disconnected #" + String(pServer->getConnId()), false);
		}
};

/**
 * MyCallbackHandler
 * Callbacks for client write requests
 */
class MyCallbackHandler: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    int len = value.length();

    String strValue = "";

    if (value.length() > 0) {
			digitalOut = (uint8_t) value[0];
			digOutChanged = true;

      Serial.println("*********");
      Serial.print("New value: ");
      for (int i = 0; i < value.length(); i++) {
        Serial.print(String(value[i]));
        strValue += value[i];
      }
      Serial.println();
      Serial.println("*********");
      addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " BLE received: " + strValue, false);
    }
  }
};

/**
 * initBLEserver
 * Setup BLE server
 * Setup callbacks for server and pCharacteristicStatus
 * Start advertising the BLE service
 */
void initBLEserver() {

	addMqttMsg(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE initBLEserver()", false);

	// Initialize BLE
	BLEDevice::init(apName);
	// BLEDevice::setPower(ESP_PWR_LVL_P7);

	// Create BLE Server
	pServer = BLEDevice::createServer();

	// Set server callbacks
	pServer->setCallbacks(new MyServerCallbacks());

	// Create BLE Service
	pService = pServer->createService(BLEUUID((uint16_t)SERVICE_UUID),20);

	// Create BLE Characteristic for Alert
	
	pCharacteristicNotify = pService->createCharacteristic(
		BLEUUID((uint16_t)NOTIFICATION_UUID),
		BLECharacteristic::PROPERTY_READ	 |
		// BLECharacteristic::PROPERTY_INDICATE |
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

	// Create BLE Characteristic for Dew Point
	pCharacteristicDewPoint = pService->createCharacteristic(
		BLEUUID((uint16_t)DEW_UUID),
		BLECharacteristic::PROPERTY_READ
	);

	// Create BLE Characteristic for Perception
	pCharacteristicPerception = pService->createCharacteristic(
		PERCEPTION_UUID,
		BLECharacteristic::PROPERTY_READ
	);

	// Create BLE Characteristic for Comfort
	pCharacteristicComfort = pService->createCharacteristic(
		COMFORT_UUID,
		BLECharacteristic::PROPERTY_READ
	);

	// Create BLE Characteristic for Heat Index
	pCharacteristicHeatIndex = pService->createCharacteristic(
		BLEUUID((uint16_t)HEAT_UUID),
		BLECharacteristic::PROPERTY_READ
	);

	// Create BLE Characteristic for Digital output
	pCharacteristicOutput = pService->createCharacteristic(
		BLEUUID((uint16_t)OUTPUT_UUID),
		BLECharacteristic::PROPERTY_WRITE
	);
	pCharacteristicOutput->setCallbacks(new MyCallbackHandler());

	// Create BLE Characteristic for Digital output
	pCharacteristicDeviceName = pService->createCharacteristic(
		BLEUUID((uint16_t)DEVICENAME_UUID),
		BLECharacteristic::PROPERTY_READ
	);
	pCharacteristicDeviceName->setValue((uint8_t*)apName,16);

	// Start the service
	pService->start();

	// Start advertising
	pAdvertising = pServer->getAdvertising();

	pAdvertising->start();

	addMqttMsg(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE active now", false);
}

/**
 * stopBLE
 * Stop advertising the BLE service
 */
void stopBLE() {
	pAdvertising->stop();
}
