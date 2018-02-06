#include "setup.h"
#include "esp32-hal-bt.c"

/** OTA progress */
int otaStatus = 0;

/**
 * Activate OTA
 *
 * @param MODULTYPE
 *		Module type as char* e.g. 'aircon', 'security', 'light'
 * @param MODULID
 *		Module ID used by other apps to recognize this device
 *		e.g. ac1, lb1, sf1, ...
 */
void activateOTA(const char *MODULTYPE, const char *MODULID) {

	ArduinoOTA
		.setHostname(apName)
		.onStart([]() {
			addMqttMsg(debugLabel, infoLabel + digitalTimeDisplaySec() + " OTA_START", false);
			// Set OTA flag
			otaRunning = true;

			// Stop LED from flashing
			stopFlashing();

			// Stop all timers
			stopLight();
			stopTemp();
			stopUGWeather();

			// Just in case the touch pad timers are active (should not happen!)
			disableTouch();

			// Stop SPI
			// stopSPI();
			
			// Stop BLE advertising
			stopBLE();

			// Stop MQTT WiFi client
			mqttClient.disconnect();
			
			// Stop UDP listener
			udpListener.stop();

			// Give the tasks some time to shutdown
			delay(500);

			pinMode(16, OUTPUT);
			String type;
			if (ArduinoOTA.getCommand() == U_FLASH)
				type = "sketch";
			else // U_SPIFFS
				type = "filesystem";

			// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
			Serial.println("OTA Start updating " + type);

			tft.fillScreen(TFT_BLUE);
			tft.setTextDatum(MC_DATUM);
			tft.setTextColor(TFT_WHITE);
			tft.setTextSize(2);
			tft.drawString("OTA",64,50);
			tft.drawString("Progress:",64,75);
		})
		.onEnd([]() {
			Serial.println("\n OTA End");

			tft.fillScreen(TFT_GREEN);
			tft.setTextDatum(MC_DATUM);
			tft.setTextColor(TFT_BLACK);
			tft.setTextSize(2);
			tft.drawString("OTA",64,50);
			tft.drawString("FINISHED!",64,80);
			delay(10);
		})
		.onProgress([](unsigned int progress, unsigned int total) {
			unsigned int achieved = progress / (total / 100);
			if (otaStatus == 0 || achieved == otaStatus + 1) {
				digitalWrite(16, !digitalRead(16));
				otaStatus = achieved;
				tft.setTextDatum(MC_DATUM);
				tft.setTextSize(2);
				tft.fillRect(32,91,64,28,TFT_BLUE);
				String progVal = String(achieved) + "%";
				tft.drawString(progVal,64,105);
			}
		})
		.onError([](ota_error_t error) {
			tft.fillScreen(TFT_RED);
			tft.setTextDatum(MC_DATUM);
			tft.setTextColor(TFT_BLACK);
			tft.setTextSize(2);
			tft.drawString("OTA",64,30,2);
			tft.drawString("ERROR:",64,60,2);

			Serial.printf("\nOTA Error[%u]: ", error);
			if (error == OTA_AUTH_ERROR) {
				Serial.println("Auth Failed");
				tft.drawString("Auth Failed",64,80,2);
			} else if (error == OTA_BEGIN_ERROR) {
				Serial.println("Begin Failed");
				tft.drawString("Begin Failed",64,80,2);
			} else if (error == OTA_CONNECT_ERROR) {
				Serial.println("Connect Failed");
				tft.drawString("Connect Failed",64,80,2);
			} else if (error == OTA_RECEIVE_ERROR) {
				Serial.println("Receive Failed");
				tft.drawString("Receive Failed",64,80,2);
			} else if (error == OTA_END_ERROR) {
				Serial.println("End Failed");
				tft.drawString("End Failed",64,80,2);
			}
		});

	ArduinoOTA.begin();

	MDNS.addServiceTxt("_arduino", "_tcp", "service", "MHC");
	MDNS.addServiceTxt("_arduino", "_tcp", "type", "TestBoard");
	MDNS.addServiceTxt("_arduino", "_tcp", "id", "ESP32-Test");

  MDNS.addService("_mhc_cmd","_tcp",9998);
  MDNS.addService("_mhc_brc","_udp",9997);
  MDNS.addService("_mhc_dbg","_tcp",9999);
}
