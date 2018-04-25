#include <setup.h>

// Flag if OTA was initiated
bool otaInitDone = false;

// Flag if btSerial is activated
bool btSerialActive = false;

/**********************************************************/
// Give the board a type and an ID
/**********************************************************/
/** Module type used for mDNS */
const char* MODULTYPE = "type=TestBoard"; // e.g. aircon, light, TestBoard, ...
/** Module id used for mDNS */
const char* MODULID = "id=ESP32-Test"; // e.g. ac1, lb1, ESP32-Test, ...
/** mDNS and Access point name */
char apName[] = "ESP32-Test-xxxxxx";
/** Index to add module ID to apName */
int apIndex = 11; // position of first x in apName[]

/**
 * loop
 * Arduino loop function, called once 'setup' is complete (your own code
 * should go here)
 */
void loop(void)	{
	if (otaInitDone) {
		ArduinoOTA.handle();
		if (otaRunning) {
			return;
		}
	}
	/**********************************************************/
	// Do other stuff from here on
	/**********************************************************/

	// Check if connection initialization was successfull
	if ((connStatus == CON_INIT) || (connStatus == CON_START)) {
		if ((millis() - wifiConnectStart) > 30000) {
			Serial.println("WiFi connection failed for more than 30 seconds");
			// if (!scanWiFi()) {
			// 	Serial.println("Scan failed");
			// 	connStatus = CON_LOST;
			// 	// delay(1000);
			// 	// esp_restart();
			// }
			// wifiConnect();

			wifiMulti.run();
			wifiConnectStart = millis();
		}
	}
	else if (connStatus == CON_GOTIP) {
		// Check if ArduinoOTA is active
		if (!otaInitDone) {
			if (connStatus == CON_GOTIP) {
				activateOTA(MODULTYPE, MODULID);
				otaInitDone = true;
			}
		}
		// Check if tasks are running
		if (!tasksEnabled) {
			delay(5000);
			shortTouchPad1 = false;
			longTouchPad1 = false;
			shortTouchPad2 = false;
			longTouchPad2 = false;
			shortTouchPad3 = false;
			longTouchPad3 = false;
			
			tft.fillRect(0, 0, 128, 120, TFT_BLACK); // Clear screen
			tft.fillScreen(TFT_BLACK);
			tft.setCursor(0, 40);
			tft.setTextColor(TFT_WHITE);
			tft.setTextSize(1);
			tft.fillRect(120, 32, 8, 9, TFT_GREEN);

			// Start UDP listener
			udpListener.begin(9997);

			// Initialize MQTT connection
			// initMqtt();

			// Start NTP listener
			initNTP();
			tryGetTime();

			// Initialize Weather and NTP time updates
			if (!initUGWeather()) {
				// Serial.println(errorLabel + digitalTimeDisplaySec() + " Failed to start weather & time updates");
				sendDebug(debugLabel, errorLabel + digitalTimeDisplaySec() + " Failed to start weather & time updates", false);
			}

			// Start the TCP server to receive commands
			tcpServer.begin();

			// printPartitions();

			if (tempTaskHandle != NULL) {
				vTaskResume(tempTaskHandle);
			}
			if (lightTaskHandle != NULL) {
				vTaskResume(lightTaskHandle);
			}
			if (weatherTaskHandle != NULL) {
				vTaskResume(weatherTaskHandle);
			}

			tasksEnabled = true;

		}
		// Do stuff that makes only sense if we are connected
		/** Handle MQTT subscribtions */
		// mqttClient.loop();

		// if(!mqttClient.connected()) {
		// 	// Handle OTA updates
		// 	ArduinoOTA.handle();
		// 	// Try to reconnect to MQTT
		// 	connectMQTT();
		// }

		WiFiClient tcpClient = tcpServer.available();
		if (tcpClient) {
			getTCPPacket(tcpClient);
		}

		// Check if broadcast arrived
		if (udpListener.peek() != 0) {
		udpMsgLength = udpListener.parsePacket();
		if (udpMsgLength > 0) {
			getUDPbroadcast(udpMsgLength);
		}
		}
	}
	// Check if connection was lost
	// if (connStatus == CON_LOST) {
	else {
		// tft.fillScreen(TFT_BLACK);
		// tft.setCursor(0, 40);
		// tft.setTextColor(TFT_WHITE);
		// tft.setTextSize(1);
		// tft.println("WiFi connection lost");
		// Serial.println("WiFi connection lost");
		tft.fillRect(120, 32, 8, 9, TFT_RED);

		// Stop LED from flashing
		stopFlashing();
		// Stop all timers
		stopUGWeather();
		// Stop MQTT WiFi client
		// mqttClient.disconnect();
		// Stop UDP listener
		udpListener.stop();

		// Set initialization mode
		tasksEnabled = false;

		// Trigger restart of ArduinoOTA on next connection
		ArduinoOTA.end();
		otaInitDone = false;

		// Try to reconnect
		wifiMulti.run();

		// if (scanWiFi()) {
		// 	Serial.println("Scan failed");
		// 	connStatus = CON_LOST;
		// 	// delay(1000);
		// 	// esp_restart();
		// }
		// connectWiFi();
	}

	// Check if LDR light values are updated
	// if (newLDRValue != 0) {
	// 	tft.setCursor(85,97);
	// 	tft.fillRect(80, 89, 48, 16, TFT_DARKGREEN);
	// 	tft.setTextSize(1);
	// 	tft.print(String(newLDRValue));
	// 	newLDRValue = 0;
	// }

	// Check if TSL light values are updated
	// if (newTSLValue != 0) {
	// 	tft.setCursor(85,108);
	// 	tft.fillRect(80, 105, 48, 15, TFT_DARKGREEN);
	// 	tft.setTextSize(1);
	// 	tft.print(String(newTSLValue) + "lux");
	// 	newTSLValue = 0;
	// }

	// Check if Pad 1 (T8) was touched
	if (shortTouchPad1) {
		shortTouchPad1 = false;
		// reStartBtSerial();
		// sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec()
		// 				 + " BT Serial restart", false);

		// if (!isScanning) {
		// 	if (pClient != NULL && pClient->isConnected()) {
		// 		pClient->disconnect();
		// 		connected = false;
		// 	}
		// 	Serial.println(infoLabel + digitalTimeDisplaySec() + " BLE Scan started");
		// 	scanBLEdevices();
		// }
	}

	// Check if Pad 1 (T8) was long touched
	if (longTouchPad1) {
		longTouchPad1 = false;
		// if (connected) {
		// 	std::string value = pRemoteCharacteristic->readValue();
		//	 sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec()
		//				 + " BLE characteristic value was: " + value.c_str(), false);
		//	 Serial.print("The characteristic value was: ");
		//	 Serial.println(value.c_str());
		// }
	}

	// Check if we found a BLE server
	// if (doConnect) {
	// 	if (connectToServer(*pServerAddress)) {
	//		 Serial.println("We are now connected to the BLE Server.");
	// 		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec()
	//					 + " BLE Connected to server", false);
	//		 connected = true;
	//	 } else {
	//		 Serial.println("We have failed to connect to the server; there is nothin more we will do.");
	// 		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec()
	//					 + " BLE Connection to server failed", false);
	//	 }
	//	 doConnect = false;
	// }

	// Check if Pad 3 (T7) was touched
	if (shortTouchPad3) {
		shortTouchPad3 = false;
		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " Pad 3 was touched", false);
		// if (!btSerialActive) {
		// 	// Initialize BT serial interface
		// 	initBtSerial();
		// 	btSerialActive = true;
		// 	Serial.println("Started Bluetooth Serial");
		// 	restartBLE();
		// } else {
		// 	stopBtSerial();
		// 	stopBLE();
		// 	// initBlueTooth();
		// 	restartBLE();
		// 	btSerialActive = false;
		// 	Serial.println("Started BLE");
		// }
	}

	// Check if Pad 3 (T7) was long touched
	if (longTouchPad3) {
		longTouchPad3 = false;
	}
}
