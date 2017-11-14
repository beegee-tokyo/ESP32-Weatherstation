#include "setup.h"
#include "declarations.h"

/** Build time */
const char compileDate[] = __DATE__ " " __TIME__;

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
 * Arduino setup function (called once after boot/reboot)
 */
/**************************************************************************/
void setup(void)
{
	Serial.begin(115200);
#ifdef ENA_DEBUG
	Serial.setDebugOutput(true);
#endif
#ifdef HAS_TFT
	/**********************************************************/
	// If TFT is connected, initialize it
	/**********************************************************/
	tft.init();
	tft.fillScreen(TFT_BLACK);
	tft.setCursor(0, 40);
	tft.setTextColor(TFT_WHITE);
#endif

#ifdef ENA_DEBUG
	Serial.print("Build: ");
	Serial.println(compileDate);
#endif
#ifdef HAS_TFT
	tft.println("Build: ");
	tft.setTextSize(1);
	tft.println(compileDate);
#endif

	/**********************************************************/
	// Create Access Point name & mDNS name
	// from MAC address
	/**********************************************************/
	createName(apName, apIndex);

	if (connectWiFi()) {
		// WiFi connection successfull
#ifdef HAS_TFT
		tft.println("Connected to ");
		tft.println(WiFi.SSID());
		tft.println("with IP address ");
		tft.println(WiFi.localIP());
#endif
#ifdef ENA_DEBUG
		Serial.print("Connected to " + String(WiFi.SSID()) + " with IP address ");
		Serial.println(WiFi.localIP());
#endif
	} else {
		// WiFi connection failed
#ifdef HAS_TFT
		tft.println("Failed to connect to WiFI");
		tft.println("Rebooting in 30 seconds");
#endif
#ifdef ENA_DEBUG
		Serial.println("Failed to connect to WiFi");
#endif
		// Decide what to do next
		delay(30000);
		esp_restart();
	}

	/**********************************************************/
	// Activate ArduinoOTA
	// and setup mDNS records
	/**********************************************************/
	activateOTA(MODULTYPE, MODULID);

	/**********************************************************/
	// Initialize other stuff from here on
	/**********************************************************/

	// Initialize touch interface
	initTouch();

	// Initialize Spiffs
	byte spiffsStatus = 0;
	if (!SPIFFS.begin(false,"/home",2)) {
		if (SPIFFS.format()){
			spiffsStatus = 1;
		} else {
			spiffsStatus = 2;
		}
	}

	switch (spiffsStatus) {
		case 1:
			tft.println("FS needed formatting");
			Serial.println("FS needed formatting");
		case 0:
			tft.println("FS: " + String(SPIFFS.totalBytes()/1024) + "kB used: " + (SPIFFS.usedBytes()/1024) + "kB");
			Serial.println("FS: " + String(SPIFFS.totalBytes()) + "Bytes used: " + String(SPIFFS.usedBytes()) + "Bytes");
			Serial.println("FS: " + String((float)SPIFFS.totalBytes()/1024,2) + "kB used: " + String((float)SPIFFS.usedBytes()/1024,2) + "kB");
			Serial.println("FS: " + String((float)SPIFFS.totalBytes()/1024/1024,2) + "MB used: " + String((float)SPIFFS.usedBytes()/1024/1024,2) + "MB");
			break;
		case 2:
			tft.println("FS formatting failed");
			Serial.println("FS formatting failed");
			break;
	}

	// Start FTP server
	if (spiffsStatus < 2) {
		ftpSrv.begin("esp32","esp32");
		Serial.println("FTP server initiated");
	}

	// Start UDP listener
	udpListener.begin(9997);

	// Start NTP listener
	initNTP();
	tryGetTime();

	// Initialize LED flashing
	initLed();
	startFlashing(1000);

	// Initialize MEEO connection
	initMeeo();

	// Initialize Light measurement
	byte lightInitResult = initLight();
	switch (lightInitResult) {
		case 0:
			Serial.println("[INFO] " + digitalTimeDisplaySec() + " Light sensors available and initialized");
			addMeeoMsg("", "[INFO] " + digitalTimeDisplaySec() + " Light sensors available and initialized", true);
			break;
		case 1:
			Serial.println("[ERROR] " + digitalTimeDisplaySec() + " Light sensors not available");
			addMeeoMsg("", "[ERROR] " + digitalTimeDisplaySec() + " Light sensors not available", true);
			break;
		case 2:
		default:
			Serial.println("[ERROR] " + digitalTimeDisplaySec() + " Failed to start timer for light measurement");
			addMeeoMsg("", "[ERROR] " + digitalTimeDisplaySec() + " Failed to start timer for light measurement", true);
			break;
	}

	// Initialize temperature measurements
	if (!initTemp()) {
		Serial.println("[ERROR] " + digitalTimeDisplaySec() + " Failed to start temperature measurement");
		addMeeoMsg("", "[ERROR] " + digitalTimeDisplaySec() + " Failed to start temperature measurement", true);
	}

	// Initialize Weather and NTP time updates
	if (!initUGWeather()) {
		Serial.println("[ERROR] " + digitalTimeDisplaySec() + " Failed to start weather & time updates");
		addMeeoMsg("", "[ERROR] " + digitalTimeDisplaySec() + " Failed to start weather & time updates", true);
	}
	// if (!initAccuWeather()) {
	// 	Serial.println("[ERROR] --:-- Failed to start weather & time updates");
	// 	addMeeoMsg("", "[ERROR] --:-- Failed to start weather & time updates", true);
	// }

	String resetReason = reset_reason(rtc_get_reset_reason(0));
	addMeeoMsg("", "[INFO] " + digitalTimeDisplaySec() + " Reset reason CPU0: " + resetReason, true);
	resetReason = reset_reason(rtc_get_reset_reason(1));
	addMeeoMsg("", "[INFO] " + digitalTimeDisplaySec() + " Reset reason CPU1: " + resetReason, true);
}
