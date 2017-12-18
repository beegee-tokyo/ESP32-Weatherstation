#include "setup.h"

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
			addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " OTA_START", false);
			stopFlashing();
			lightTicker.detach();
			tempTicker.detach();
			weatherTicker.detach();
			touchTicker.detach();

			otaRunning = true;

			pinMode(16, OUTPUT);
			String type;
			if (ArduinoOTA.getCommand() == U_FLASH)
				type = "sketch";
			else // U_SPIFFS
				type = "filesystem";

			// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
			Serial.println("OTA Start updating " + type);

	#ifdef HAS_TFT
			tft.fillScreen(TFT_BLUE);
			tft.setTextDatum(MC_DATUM);
			tft.setTextColor(TFT_WHITE);
			tft.setTextSize(2);
			tft.drawString("OTA",64,50);
			tft.drawString("Progress:",64,75);
	#endif
    })
    .onEnd([]() {
			Serial.println("\n OTA End");

	#ifdef HAS_TFT
			tft.fillScreen(TFT_GREEN);
			tft.setTextDatum(MC_DATUM);
			tft.setTextColor(TFT_BLACK);
			tft.setTextSize(2);
			tft.drawString("OTA",64,50);
			tft.drawString("FINISHED!",64,80);
	#endif
			delay(10);
    })
#ifdef HAS_TFT
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
#endif
    .onError([](ota_error_t error) {
			#ifdef HAS_TFT
					tft.fillScreen(TFT_RED);
					tft.setTextDatum(MC_DATUM);
					tft.setTextColor(TFT_BLACK);
					tft.setTextSize(2);
					tft.drawString("OTA",64,30,2);
					tft.drawString("ERROR:",64,60,2);
			#endif

					Serial.printf("\nOTA Error[%u]: ", error);
					if (error == OTA_AUTH_ERROR) {
						Serial.println("Auth Failed");
			#ifdef HAS_TFT
						tft.drawString("Auth Failed",64,80,2);
			#endif
					}
					else if (error == OTA_BEGIN_ERROR) {
						Serial.println("Begin Failed");
			#ifdef HAS_TFT
						tft.drawString("Begin Failed",64,80,2);
			#endif
					}
					else if (error == OTA_CONNECT_ERROR) {
						Serial.println("Connect Failed");
			#ifdef HAS_TFT
						tft.drawString("Connect Failed",64,80,2);
			#endif
					}
					else if (error == OTA_RECEIVE_ERROR) {
						Serial.println("Receive Failed");
			#ifdef HAS_TFT
						tft.drawString("Receive Failed",64,80,2);
			#endif
					}
					else if (error == OTA_END_ERROR) {
						Serial.println("End Failed");
			#ifdef HAS_TFT
						tft.drawString("End Failed",64,80,2);
			#endif
					}
    });

	ArduinoOTA.begin();

	const char * mhcTxtData[7] = {
			"board=" ARDUINO_BOARD,
			"tcp_check=no",
			"ssh_upload=no",
			"auth_upload=no",
			MODULTYPE,
			MODULID,
			"service=MHC"
  };
	MDNS.addMultiServiceTxt("_arduino", "_tcp", mhcTxtData, 7);
}
