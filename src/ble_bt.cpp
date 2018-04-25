// #include "setup.h"
// #include <BLEUtils.h>
// #include <BLEServer.h>
// #include <BLE2902.h>
// #include <BLEAdvertising.h>
// #include <preferences.h>

// // Environmental Condition Service and Characteristic UUIDs
// #define SERVICE_UUID        0x181A
// #define NOTIFICATION_UUID   0x2A08
// #define TEMP_UUID           0x2A6E
// #define HUMID_UUID          0x2A6F
// #define HEAT_UUID           0x2A7A
// #define DEW_UUID            0x2A7B
// #define COMFORT_UUID        "00002A3D-ead2-11e7-80c1-9a214cf093ae" // same as String characteristic 0x2A3D
// #define PERCEPTION_UUID     "10002A3D-ead2-11e7-80c1-9a214cf093ae" //	same as String characteristic 0x2A3D
// #define DEVICENAME_UUID     0x2A00

// // WiFi setup Service and  Characteristic UUIDs
// #define WIFI_SERVICE_UUID   "0000aaaa-ead2-11e7-80c1-9a214cf093ae"
// #define WIFI_UUID           "00005555-ead2-11e7-80c1-9a214cf093ae"

// /** Characteristic for client notification */
// BLECharacteristic *pCharacteristicNotify;
// /** Characteristic for temperature in celsius */
// BLECharacteristic *pCharacteristicTemp;
// /** Characteristic for humidity in percent */
// BLECharacteristic *pCharacteristicHumid;
// /** Characteristic for heat index in celsius */
// BLECharacteristic *pCharacteristicHeatIndex;
// /** Characteristic for dew point in celsius */
// BLECharacteristic *pCharacteristicDewPoint;
// /** Characteristic for environment comfort status */
// BLECharacteristic *pCharacteristicComfort;
// /** Characteristic for environment perception status */
// BLECharacteristic *pCharacteristicPerception;
// /** Characteristic for device name */
// BLECharacteristic *pCharacteristicDeviceName;

// /** Characteristic for WiFi setup */
// BLECharacteristic *pCharacteristicWiFi;

// /** BLE Advertiser */
// BLEAdvertising* pAdvertising;
// /** BLE Environment Service */
// BLEService *pServiceEnv;
// /** BLE WiFi setup Service */
// BLEService *pServiceWiFi;
// /** BLE Server */
// BLEServer *pServer;

// /** Flag if a client is connected */
// bool bleConnected = false;

// /** WiFi AP credentials received over BLE */
// String wifiAPCred = "";

// BluetoothSerial SerialBT;

// /** Max number of messages in buffer */
// #define sppMsgBufferSize 10
// /** Bluetooth Serial device name */
// String btSppName = "ESP32 Weather";

// void btSppTask(void *pvParameters);

// /** Structure for Bluetooth Serial message Buffer */
// struct btSppMsgStruct {
// 	String payload = "";
// };

// /** Queued messages for Bluetooth Serial */
// btSppMsgStruct btSppMsg[sppMsgBufferSize];
// /** Task handle for the Bluetooth Serial sender task */
// TaskHandle_t btSppTaskHandle = NULL;

// /**
//  * MyServerCallbacks
//  * Callbacks for client connection and disconnection
//  */
// class MyServerCallbacks: public BLEServerCallbacks {
// 	// TODO this doesn't take into account several clients being connected
// 	void onConnect(BLEServer* pServer) {
// 		bleConnected = true;
// 		// sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE connected #" + String(pServer->getConnId()), false);
// 		pAdvertising->start();
// 	};

// 	void onDisconnect(BLEServer* pServer) {
// 		if (pServer->getConnectedCount() == 0) {
// 			bleConnected = false;
// 		}
// 			// sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE disconnected #" + String(pServer->getConnId()), false);
// 	}
// };

// /**
//  * MyCallbackHandler
//  * Callbacks for client write requests
//  */
// class MyCallbackHandler: public BLECharacteristicCallbacks {
//   void onWrite(BLECharacteristic *pCharacteristic) {
//     std::string value = pCharacteristic->getValue();
// 		if (value.length() == 0) {
// 			return;
// 		}
// 		String dbgMsg = "Received over BLE: " + String((char *)&value[0]);
// 		sendDebug(debugLabel, dbgMsg, false);

// 		/** Buffer for incoming JSON string */
// 		DynamicJsonBuffer jsonInBuffer;
// 		/** Json object for incoming data */
// 		JsonObject& jsonIn = jsonInBuffer.parseObject((char *)&value[0]);
// 		if (jsonIn.success()) {
// 			if (jsonIn.containsKey(SSID_PRIM_KEY) &&
// 					jsonIn.containsKey(PW_PRIM_KEY) && 
// 					jsonIn.containsKey(SSID_SEC_KEY) &&
// 					jsonIn.containsKey(PW_SEC_KEY)) {
// 				ssidPrim = jsonIn[SSID_PRIM_KEY].as<String>();
// 				pwPrim = jsonIn[PW_PRIM_KEY].as<String>();
// 				ssidSec = jsonIn[SSID_SEC_KEY].as<String>();
// 				pwSec = jsonIn[PW_SEC_KEY].as<String>();

// 				Preferences preferences;
// 				preferences.begin(PREF_NAME, false);
// 				preferences.putString(SSID_PRIM_KEY, ssidPrim);
// 				preferences.putString(SSID_SEC_KEY, ssidSec);
// 				preferences.putString(PW_PRIM_KEY, pwPrim);
// 				preferences.putString(PW_SEC_KEY, pwSec);
// 				preferences.putBool(VALID_KEY, true);

// 				if (jsonIn.containsKey(DEV_ID_KEY)) {
// 					devID = jsonIn[DEV_ID_KEY].as<String>();
// 					preferences.putString(DEV_ID_KEY, devID);
// 				}
// 				if (jsonIn.containsKey(DEV_LOC_KEY)) {
// 					devLoc = jsonIn[DEV_LOC_KEY].as<String>();
// 					preferences.putString(DEV_LOC_KEY, devLoc);
// 				}
// 				if (jsonIn.containsKey(DEV_TYPE_KEY)) {
// 					devType = jsonIn[DEV_TYPE_KEY].as<String>();
// 					preferences.putString(DEV_TYPE_KEY, devType);
// 				}

// 				preferences.end();

// 				dbgMsg = "Received over bluetooth:";
// 				sendDebug(debugLabel, dbgMsg, false);
// 				dbgMsg = "primary SSID: "+ssidPrim+" password: "+pwPrim;
// 				sendDebug(debugLabel, dbgMsg, false);
// 				dbgMsg = "secondary SSID: "+ssidSec+" password: "+pwSec;
// 				sendDebug(debugLabel, dbgMsg, false);
// 			} else if (jsonIn.containsKey("erase")) {
// 				sendDebug(debugLabel, "Received erase command", false);
// 				Preferences preferences;
// 				preferences.begin(PREF_NAME, false);
// 				preferences.clear();
// 				preferences.putBool(VALID_KEY, false);
// 				preferences.end();
// 				ssidPrim = "";
// 				pwPrim = "";
// 				ssidSec = "";
// 				pwSec = "";
// 				return;
// 			} else if (jsonIn.containsKey("reset")) {
// 				WiFi.disconnect();
// 				esp_restart();
// 			}
// 		} else {
// 			sendDebug(debugLabel, errorLabel + digitalTimeDisplaySec() + "Received invalid JSON", false);
// 		}
// 	};

// 	void onRead(BLECharacteristic *pCharacteristic) {
//      sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE onRead", false);
// 		/** Buffer for incoming JSON string */
// 		DynamicJsonBuffer jsonBuffer;
// 		/** Json object for outgoing data */
// 		JsonObject& jsonOut = jsonBuffer.createObject();
// 		jsonOut[SSID_PRIM_KEY] = ssidPrim;
// 		jsonOut[PW_PRIM_KEY] = pwPrim;
// 		jsonOut[SSID_SEC_KEY] = ssidSec;
// 		jsonOut[PW_SEC_KEY] = pwSec;
// 		jsonOut[DEV_ID_KEY] = devID;
// 		jsonOut[DEV_TYPE_KEY] = devType;
// 		jsonOut[DEV_LOC_KEY] = devLoc;
// 	}
// };

// /**
//  * initBlueTooth
//  * @param which
//  *			selects what to initialize
//  *			0 init BLE server only
//  *			1 init Bluetooth serial only
//  *			2 init both BLE server and Bluetooth serial
//  */
// void initBlueTooth(byte which) {

// 	sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " initBlueTooth(" + String(which) + ")", false);

// 	if (which != 1) {
// 		// Initialize BLE
// 		BLEDevice::init(apName);
// 		BLEDevice::setPower(ESP_PWR_LVL_P7);
// 	}

// 	if (which != 0) {
// 		if (!SerialBT.begin(apName)) {
// 			sendDebug(debugLabel, errorLabel + digitalTimeDisplaySec() + " Failed to start BTSerial", false);
// 		} else {
// 			// Clear message structure
// 			for (int index = 0; index < sppMsgBufferSize; index++) {
// 				btSppMsg[index].payload = "";
// 			}
// 			// Start task for Serial Bluetooth
// 			xTaskCreatePinnedToCore(
// 					btSppTask,             /* Function to implement the task */
// 					"BtSpp ",              /* Name of the task */
// 					2000,                  /* Stack size in words */
// 					NULL,                  /* Task input parameter */
// 					5,                     /* Priority of the task */
// 					&btSppTaskHandle,      /* Task handle. */
// 					1);                    /* Core where the task should run */		
// 		}
// 	}

// 	if (which != 1) {
// 		// Create BLE Server
// 		pServer = BLEDevice::createServer();

// 		// Set server callbacks
// 		pServer->setCallbacks(new MyServerCallbacks());

// 		// Create BLE Service
// 		pServiceEnv = pServer->createService(BLEUUID((uint16_t)SERVICE_UUID),20);

// 		// Create BLE Characteristic for Alert
// 		pCharacteristicNotify = pServiceEnv->createCharacteristic(
// 			BLEUUID((uint16_t)NOTIFICATION_UUID),
// 			BLECharacteristic::PROPERTY_READ	 |
// 			// BLECharacteristic::PROPERTY_INDICATE |
// 			BLECharacteristic::PROPERTY_NOTIFY
// 		);

// 		// Create a BLE Descriptor for Alert
// 		pCharacteristicNotify->addDescriptor(new BLE2902());

// 		// Create BLE Characteristic for Temperature
// 		pCharacteristicTemp = pServiceEnv->createCharacteristic(
// 			BLEUUID((uint16_t)TEMP_UUID),
// 			BLECharacteristic::PROPERTY_READ
// 		);

// 		// Create BLE Characteristic for Humidity
// 		pCharacteristicHumid = pServiceEnv->createCharacteristic(
// 			BLEUUID((uint16_t)HUMID_UUID),
// 			BLECharacteristic::PROPERTY_READ
// 		);

// 		// Create BLE Characteristic for Dew Point
// 		pCharacteristicDewPoint = pServiceEnv->createCharacteristic(
// 			BLEUUID((uint16_t)DEW_UUID),
// 			BLECharacteristic::PROPERTY_READ
// 		);

// 		// Create BLE Characteristic for Perception
// 		pCharacteristicPerception = pServiceEnv->createCharacteristic(
// 			PERCEPTION_UUID,
// 			BLECharacteristic::PROPERTY_READ
// 		);

// 		// Create BLE Characteristic for Comfort
// 		pCharacteristicComfort = pServiceEnv->createCharacteristic(
// 			COMFORT_UUID,
// 			BLECharacteristic::PROPERTY_READ
// 		);

// 		// Create BLE Characteristic for Heat Index
// 		pCharacteristicHeatIndex = pServiceEnv->createCharacteristic(
// 			BLEUUID((uint16_t)HEAT_UUID),
// 			BLECharacteristic::PROPERTY_READ
// 		);

// 		// Create BLE Characteristic for Device Name
// 		pCharacteristicDeviceName = pServiceEnv->createCharacteristic(
// 			BLEUUID((uint16_t)DEVICENAME_UUID),
// 			BLECharacteristic::PROPERTY_READ
// 		);
// 		pCharacteristicDeviceName->setValue((uint8_t*)apName,16);

// 		// Create BLE Service
// 		pServiceWiFi = pServer->createService(BLEUUID(WIFI_SERVICE_UUID),10);

// 		// Create BLE Characteristic for WiFi settings
// 		pCharacteristicWiFi = pServiceWiFi->createCharacteristic(
// 			BLEUUID(WIFI_UUID),
// 			// WIFI_UUID,
// 			BLECharacteristic::PROPERTY_READ |
// 			BLECharacteristic::PROPERTY_WRITE
// 		);
// 		pCharacteristicWiFi->setCallbacks(new MyCallbackHandler());
// 		String wifiCredentials;
// 	 	/** Buffer for outgoing JSON string */
// 		DynamicJsonBuffer jsonOutBuffer;
// 		/** Json object for outgoing data */
// 		JsonObject& jsonOut = jsonOutBuffer.createObject();

// 	jsonOut[SSID_PRIM_KEY] = ssidPrim;
// 	jsonOut[PW_PRIM_KEY] = pwPrim;
// 	jsonOut[SSID_SEC_KEY] = ssidSec;
// 	jsonOut[PW_SEC_KEY] = pwSec;
// 	jsonOut[DEV_ID_KEY] = devID;
// 	jsonOut[DEV_TYPE_KEY] = devType;
// 	jsonOut[DEV_LOC_KEY] = devLoc;
// 		// Convert JSON object into a string
// 		jsonOut.printTo(wifiCredentials);
// 		pCharacteristicWiFi->setValue((uint8_t*)&wifiCredentials[0],wifiCredentials.length());

// 		// Start the services
// 		pServiceEnv->start();
// 		pServiceWiFi->start();

// 		// Start advertising
// 		pAdvertising = pServer->getAdvertising();

// 		pAdvertising->start();
// 	}

// 	sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " Bluetooth active now", false);
// }

// /**
//  * stopBLE
//  * Stop advertising the BLE service
//  */
// void stopBLE() {
// 	if (pAdvertising != NULL) {
// 		pAdvertising->stop();
// 	}
// }

// void stopBtSerial() {
// 	if (btSppTaskHandle != NULL) {
// 		// Stop Bluetooth Serial sending task
// 		vTaskDelete(btSppTaskHandle);
// 		btSppTaskHandle = NULL;
// 		// Stop Bluetooth Serial
// 		SerialBT.end();
// 	}
// }

// void reStartBtSerial() {
// 	stopBtSerial();
// 	delay(500);
// 	// Try to restart Serial Bluetooth
// 	SerialBT.begin(apName);

// 	// Restart the task
// 	xTaskCreatePinnedToCore(
// 			btSppTask,             /* Function to implement the task */
// 			"BtSpp ",              /* Name of the task */
// 			2000,                  /* Stack size in words */
// 			NULL,                  /* Task input parameter */
// 			5,                     /* Priority of the task */
// 			&btSppTaskHandle,      /* Task handle. */
// 			1);                    /* Core where the task should run */		
// }

// bool sendBtSerial(String message) {
// 	bool queueResults = false;
// 	// Check if we have a client
// 	if (SerialBT.hasClient()) {
// 		for (byte index = 0; index < sppMsgBufferSize; index ++) {
// 			if (btSppMsg[index].payload.length() == 0) { // found an empty slot?
// 				btSppMsg[index].payload = message;
// 				queueResults = true;
// 				break;
// 			}
// 		}
// 		if (tasksEnabled) {
// 			vTaskResume(btSppTaskHandle);
// 		}
// 	}
// 	return queueResults;
// }

// /**
//  * Task to send data from btSppMsg buffer over Bluetooth Serial
//  * @param pvParameters
//  *		pointer to task parameters
//  */
// void btSppTask(void *pvParameters) {
// 	// Serial.println("btSppTask loop started");
// 	while (1) // btSppTask loop
// 	{
// 		if (otaRunning) {
// 			vTaskDelete(NULL);
// 		}
// 		for (byte index = 0; index < sppMsgBufferSize; index ++) {
// 			if (btSppMsg[index].payload.length() != 0) {
// 				// Check if we have a client
// 				if (SerialBT.hasClient()) {
// 					// Try to send the message
// 					// int result = SerialBT.println(btSppMsg[index].payload);
// 					// int result = SerialBT.write(btSppMsg[index].payload);
// 					int result = SerialBT.write((uint8_t*)&btSppMsg[index].payload[0],btSppMsg[index].payload.length()); 
// 					if (result == -1) { 
// 						// Sending failed, try to reinitialize the Bluetooth Serial
// 						SerialBT.end();
// 						delay(1000);
// 						SerialBT.begin(btSppName);
// 						// Retry to send the message
// 						// SerialBT.println(btSppMsg[index].payload);
// 						// int result = SerialBT.write(btSppMsg[index].payload);
// 						int result = SerialBT.write((uint8_t*)&btSppMsg[index].payload[0],btSppMsg[index].payload.length()); 
// 					}
// 					SerialBT.println(""); // Write a new line character
// 					// Flush the send buffer
// 					SerialBT.flush();
// 					delay(250);
// 					// Discard the message
// 					btSppMsg[index].payload = "";
// 				}
// 			}
// 		}
// 		bool queueIsEmpty = true;
// 		for (byte index = 0; index < sppMsgBufferSize; index ++) {
// 			if (btSppMsg[index].payload.length() != 0) {
// 				queueIsEmpty = false;
// 			}
// 		}
// 		if (queueIsEmpty) {
// 			vTaskSuspend(NULL);
// 		}
// 		delay(500);
// 	}
// }