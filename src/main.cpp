#include <setup.h>

bool btSerialActive = false;

/**
 * loop
 * Arduino loop function, called once 'setup' is complete (your own code
 * should go here)
 */
void loop(void)
{
	ArduinoOTA.handle();
	if (otaRunning) {
		return;
	}

	/**********************************************************/
	// Do other stuff from here on
	/**********************************************************/
	if (!tasksEnabled) {
		delay(2000); // Wait two seconds to let things settle down

		shortTouchPad1 = false;
		longTouchPad1 = false;
		shortTouchPad2 = false;
		longTouchPad2 = false;
		shortTouchPad3 = false;
		longTouchPad3 = false;
		
		tft.fillRect(0, 0, 128, 120, TFT_BLACK); // Clear screen
		tasksEnabled = true;
		if (tempTaskHandle != NULL) {
			vTaskResume(tempTaskHandle);
		}
		if (lightTaskHandle != NULL) {
			vTaskResume(lightTaskHandle);
		}
		if (weatherTaskHandle != NULL) {
			vTaskResume(weatherTaskHandle);
		}
	}

	/** Handle MQTT subscribtions */
	mqttClient.loop();

	if(!mqttClient.connected()) {
		// Handle OTA updates
		ArduinoOTA.handle();
		// Try to reconnect to MQTT
		connectMQTT();
	}

	WiFiClient tcpClient = tcpServer.available();
	if (tcpClient) {
		getTCPPacket(tcpClient);
	}

	// Check if LDR light values are updated
	if (newLDRValue != 0) {
		tft.setCursor(85,97);
		tft.fillRect(80, 89, 48, 16, TFT_DARKGREEN);
		tft.setTextSize(1);
		tft.print(String(newLDRValue));
		newLDRValue = 0;
	}

	// Check if TSL light values are updated
	if (newTSLValue != 0) {
		tft.setCursor(85,108);
		tft.fillRect(80, 105, 48, 15, TFT_DARKGREEN);
		tft.setTextSize(1);
		tft.print(String(newTSLValue) + "lux");
		newTSLValue = 0;
	}

	// Check if broadcast arrived
	udpMsgLength = udpListener.parsePacket();
	if (udpMsgLength != 0) {
		getUDPbroadcast(udpMsgLength);
	}

	// Check if new digital output value was received on BLE
	if (digOutChanged) {
		digOutChanged = false;
		if (((digitalOut & 0x01) == 0x01) && (lightTaskHandle != NULL)) {
			xTaskResumeFromISR(lightTaskHandle);
		}
		if (((digitalOut & 0x02) == 0x02) && (tempTaskHandle != NULL)) {
			xTaskResumeFromISR(tempTaskHandle);
		}
		if (((digitalOut & 0x04) == 0x04) && (weatherTaskHandle != NULL)) {
			xTaskResumeFromISR(weatherTaskHandle);
		}
	}

	// Check if Pad 1 (T8) was touched
	if (shortTouchPad1) {
		shortTouchPad1 = false;
		reStartBtSerial();
		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec()
						 + " BT Serial restart", false);
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
