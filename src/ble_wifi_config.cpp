#include "setup.h"
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEAdvertising.h>

// WiFi setup Service and  Characteristic UUIDs
#define WIFI_SERVICE_UUID   "0000aaaa-ead2-11e7-80c1-9a214cf093ae"
#define WIFI_UUID           "00005555-ead2-11e7-80c1-9a214cf093ae"

/** Characteristic for WiFi setup */
BLECharacteristic *pCharacteristicWiFi;

/** BLE Advertiser */
BLEAdvertising* pAdvertising;
/** BLE Service */
BLEService *pService;
/** BLE Server */
BLEServer *pServer;

/**
 * MyServerCallbacks
 * Callbacks for client connection and disconnection
 */
class MyServerCallbacks: public BLEServerCallbacks {
	void onConnect(BLEServer* pServer) {
		if (debugOn) {
			sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE client con", false);
		}
	};

	void onDisconnect(BLEServer* pServer) {
		if (debugOn) {
			sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE client discon", false);
		}
		pAdvertising->start();
	}
};

/**
 * MyCallbackHandler
 * Callbacks for client write requests
 */
class MyCallbackHandler: public BLECharacteristicCallbacks {
	void onWrite(BLECharacteristic *pCharacteristic) {
		std::string value = pCharacteristic->getValue();
		if (value.length() == 0) {
			return;
		}

		// Decode data
		int keyIndex = 0;
		for (int index = 0; index < value.length(); index ++) {
			value[index] = (char) value[index] ^ (char) apName[keyIndex];
			keyIndex++;
			if (keyIndex >= strlen(apName)) keyIndex = 0;
		}

		// if (debugOn) {
			sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " Rcvd BLE: " + String((char *)&value[0]), false);
		// }

		/** Buffer for JSON string */
		StaticJsonBuffer<400> jsonBuffer;

		/** Json object for incoming data */
		JsonObject& jsonIn = jsonBuffer.parseObject((char *)&value[0]);
		if (jsonIn.success()) {
			if (jsonIn.containsKey(SSID_PRIM_KEY) &&
					jsonIn.containsKey(PW_PRIM_KEY) && 
					jsonIn.containsKey(SSID_SEC_KEY) &&
					jsonIn.containsKey(PW_SEC_KEY)) {
				ssidPrim = jsonIn[SSID_PRIM_KEY].as<String>();
				pwPrim = jsonIn[PW_PRIM_KEY].as<String>();
				ssidSec = jsonIn[SSID_SEC_KEY].as<String>();
				pwSec = jsonIn[PW_SEC_KEY].as<String>();

				Preferences preferences;
				preferences.begin(PREF_NAME, false);
				preferences.putString(SSID_PRIM_KEY, ssidPrim);
				preferences.putString(SSID_SEC_KEY, ssidSec);
				preferences.putString(PW_PRIM_KEY, pwPrim);
				preferences.putString(PW_SEC_KEY, pwSec);
				preferences.putBool(VALID_KEY, true);

				if (jsonIn.containsKey(DEV_ID_KEY)) {
					devID = jsonIn[DEV_ID_KEY].as<String>();
					preferences.putString(DEV_ID_KEY, devID);
				}
				if (jsonIn.containsKey(DEV_LOC_KEY)) {
					devLoc = jsonIn[DEV_LOC_KEY].as<String>();
					preferences.putString(DEV_LOC_KEY, devLoc);
				}
				if (jsonIn.containsKey(DEV_TYPE_KEY)) {
					devType = jsonIn[DEV_TYPE_KEY].as<String>();
					preferences.putString(DEV_TYPE_KEY, devType);
				}

				preferences.end();

				// #endif
			} else if (jsonIn.containsKey("erase")) {
				if (debugOn) {
					sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE erase command", false);
				}
				Preferences preferences;
				preferences.begin(PREF_NAME, false);
				preferences.clear();
				preferences.putBool(VALID_KEY, false);
				preferences.end();
				ssidPrim = "";
				pwPrim = "";
				ssidSec = "";
				pwSec = "";
			} else if (jsonIn.containsKey("reset")) {
				if (debugOn) {
					sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE reset command", false);
				}
				WiFi.disconnect();
				esp_restart();
			}
		} else {
			if (debugOn) {
				sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE rcvd bad JSON", false);
			}
		}
		jsonBuffer.clear();
	};

	void onRead(BLECharacteristic *pCharacteristic) {
		if (debugOn) {
			sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE onRead", false);
		}
		String wifiCredentials;

		/** Buffer for JSON string */
		StaticJsonBuffer<400> jsonBuffer;

		/** Json object for outgoing data */
		JsonObject& jsonOut = jsonBuffer.createObject();
		jsonOut[SSID_PRIM_KEY] = ssidPrim;
		jsonOut[PW_PRIM_KEY] = pwPrim;
		jsonOut[SSID_SEC_KEY] = ssidSec;
		jsonOut[PW_SEC_KEY] = pwSec;
		jsonOut[DEV_ID_KEY] = devID;
		jsonOut[DEV_TYPE_KEY] = devType;
		jsonOut[DEV_LOC_KEY] = devLoc;
		// Convert JSON object into a string
		jsonOut.printTo(wifiCredentials);

		// encode the data
		int keyIndex = 0;
		for (int index = 0; index < wifiCredentials.length(); index ++) {
			wifiCredentials[index] = (char) wifiCredentials[index] ^ (char) apName[keyIndex];
			keyIndex++;
			if (keyIndex >= strlen(apName)) keyIndex = 0;
		}
		pCharacteristicWiFi->setValue((uint8_t*)&wifiCredentials[0],wifiCredentials.length());
		jsonBuffer.clear();
	}
};

void initBLE() {
	if (debugOn) {
		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " Init BLE", false);
	}
	// Initialize BLE
	BLEDevice::init(apName);
	BLEDevice::setPower(ESP_PWR_LVL_P7);

	// Create BLE Server
	pServer = BLEDevice::createServer();

	// Set server callbacks
	pServer->setCallbacks(new MyServerCallbacks());

	// Create BLE Service for WiFi setup
	pService = pServer->createService(BLEUUID(WIFI_SERVICE_UUID));

	// Create BLE Characteristic for WiFi settings
	pCharacteristicWiFi = pService->createCharacteristic(
		BLEUUID(WIFI_UUID),
		// WIFI_UUID,
		BLECharacteristic::PROPERTY_READ |
		BLECharacteristic::PROPERTY_WRITE
	);
	pCharacteristicWiFi->setCallbacks(new MyCallbackHandler());
	String wifiCredentials;

	/** Buffer for JSON string */
	StaticJsonBuffer<400> jsonBuffer;

	/** Json object for outgoing data */
	JsonObject& jsonOut = jsonBuffer.createObject();

	jsonOut[SSID_PRIM_KEY] = ssidPrim;
	jsonOut[PW_PRIM_KEY] = pwPrim;
	jsonOut[SSID_SEC_KEY] = ssidSec;
	jsonOut[PW_SEC_KEY] = pwSec;
	jsonOut[DEV_ID_KEY] = devID;
	jsonOut[DEV_TYPE_KEY] = devType;
	jsonOut[DEV_LOC_KEY] = devLoc;
	// Convert JSON object into a string
	jsonOut.printTo(wifiCredentials);
	pCharacteristicWiFi->setValue((uint8_t*)&wifiCredentials[0],wifiCredentials.length());

	// Start the services
	pService->start();

	// Start advertising
	pAdvertising = pServer->getAdvertising();
	pAdvertising->start();
}

/**
 * stopBLE
 * Stop advertising the BLE service
 */
void stopBLE() {
	if (debugOn) {
		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE stop", false);
	}
	if (pAdvertising != NULL) {
		pAdvertising->stop();
	}
	pService->dump();
}
