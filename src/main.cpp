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

	// Check Meeo channels
	Meeo.run();

	// Check if LDR light values are updated
	if (newLDRValue != 0) {
		tft.setCursor(85,102);
		tft.fillRect(80, 89, 48, 31, TFT_DARKGREEN);
		tft.setTextSize(1);
		addMeeoMsg("ldr", String(newLDRValue));
		tft.print(String(newLDRValue));
		// addMeeoMsg("", "[INFO] " + digitalTimeDisplaySec() + " Got light value from LDR", true);
		newLDRValue = 0;
	}

	// Check if TSL light values are updated
	if (newTSLValue != 0) {
		tft.setCursor(0,102);
		tft.fillRect(0, 89, 48, 31, TFT_DARKGREEN);
		tft.setTextSize(1);
		addMeeoMsg("light", String(newTSLValue));
		tft.print(String(newTSLValue) + "lux");
		// addMeeoMsg("", "[INFO] " + digitalTimeDisplaySec() + " Got light value from TSL2561", true);
		newTSLValue = 0;
	}

	// Check FTP server requests
	ftpSrv.handleFTP();

	// Check if broadcast arrived
	udpMsgLength = udpListener.parsePacket();
	if (udpMsgLength != 0) {
		getUDPbroadcast(udpMsgLength);
	}
}
