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

	/** Handle MQTT subscribtions */
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

		// String spiAnswer;
		// uint32_t esp8266Status = spiGetStatus();
		// addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI status: " + String(esp8266Status), false);
		// spiWriteData((uint8_t*) "Hello Slave!",(size_t)12);
		// delay(10);
		// spiAnswer = spiReadData();
		// if (spiAnswer[0] != 'H') {
		// 	addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI Hello Slave! got no response", false);
		// } else {
			// addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI Hello Slave! response: " + spiAnswer, false);
		// }
		// spiWriteData("Are you alive?");
		// delay(10);
		// spiAnswer = spiReadData();
		// if (spiAnswer[0] != 'A') {
		// 	addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI Are you alive? got no response", false);
		// } else {
		// 	addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI Are you alive? response: " + spiAnswer, false);
		// }
		// spiWriteData("Invalid question");
		// delay(10);
		// spiAnswer = spiReadData();
		// if (spiAnswer[0] != 'S') {
		// 	addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI Invalid question got no response", false);
		// } else {
		// 	addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI Invalid question response: " + spiAnswer, false);
		// }
	}

	// Check if broadcast arrived
	udpMsgLength = udpListener.parsePacket();
	if (udpMsgLength != 0) {
		getUDPbroadcast(udpMsgLength);
	}
}
