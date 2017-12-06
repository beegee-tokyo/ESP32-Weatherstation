#include <setup.h>

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

	/** Handle MQTT subscribed */
	mqttClient.loop();

	if(!mqttClient.connected()) {
		// Handle OTA updates
		ArduinoOTA.handle();
		// Try to reconnect to MQTT
		connectMQTT();
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
}
