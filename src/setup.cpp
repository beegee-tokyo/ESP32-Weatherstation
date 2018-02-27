#include "setup.h"
#include "declarations.h"
#include "esp_now.h"

/** Initialize OTA function */
void activateOTA(const char *MODULTYPE, const char *MODULID);

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
	// Setup labels
	debugLabel = "debug";
	infoLabel = "[INFO] ";
	errorLabel = "[ERROR] ";

	Serial.begin(115200);
#ifdef ENA_DEBUG
	Serial.setDebugOutput(true);
	debugOn = true;
#endif

	/**********************************************************/
	// If TFT is connected, initialize it
	/**********************************************************/
	tft.init();
	tft.fillScreen(TFT_BLACK);
	tft.setCursor(0, 40);
	tft.setTextColor(TFT_WHITE);

#ifdef ENA_DEBUG
	Serial.print("Build: ");
	Serial.println(compileDate);
#endif
	tft.println("Build: ");
	tft.setTextSize(1);
	tft.println(compileDate);

	/**********************************************************/
	// Create Access Point name & mDNS name
	// from MAC address
	/**********************************************************/
	createName(apName, apIndex);

	if (connDirect("MHC2", "teresa1963", 20000)) {
		// WiFi connection successfull
		tft.println("Connected to ");
		tft.println(WiFi.SSID());
		tft.println("with IP address ");
		tft.println(WiFi.localIP());
#ifdef ENA_DEBUG
		Serial.print("Connected to " + String(WiFi.SSID()) + " with IP address ");
		Serial.println(WiFi.localIP());
#endif
	} else {
		// WiFi connection failed
		tft.println("Failed to connect to WiFI");
		tft.println("Rebooting in 30 seconds");
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

	// At the moment either BLE or Bluetooth Serial can be used.
	// They are not working both at the same time.
	// Initialize only one of the two!!!!!!

	// Initialize BLE server
	// param 0 for BLE only
	// param 1 for Serial BT only
	// param 2 for both
	initBlueTooth(1);

	// Initialize MQTT connection
	initMqtt();

	// Initialize touch interface
	initTouch();

	// Start UDP listener
	udpListener.begin(9997);

	// Start NTP listener
	initNTP();
	tryGetTime();

	// Initialize LED flashing
	initLed();
	startFlashing(1000);

	// Initialize Light measurement
	byte lightInitResult = initLight();
	switch (lightInitResult) {
		case 0:
			// Serial.println(infoLabel + digitalTimeDisplaySec() + " Light sensors available and initialized");
			sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " Light task initialized", false);
			break;
		case 1:
		default:
			// Serial.println(errorLabel + digitalTimeDisplaySec() + " Failed to start timer for light measurement");
			sendDebug(debugLabel, errorLabel + digitalTimeDisplaySec() + " Failed to start timer for light measurement", false);
			break;
	}

	// Initialize temperature measurements
	if (!initTemp()) {
		// Serial.println(errorLabel + digitalTimeDisplaySec() + " Failed to start temperature measurement");
		sendDebug(debugLabel, errorLabel + digitalTimeDisplaySec() + " Failed to start temperature measurement", false);
	}

	// Initialize Weather and NTP time updates
	if (!initUGWeather()) {
		// Serial.println(errorLabel + digitalTimeDisplaySec() + " Failed to start weather & time updates");
		sendDebug(debugLabel, errorLabel + digitalTimeDisplaySec() + " Failed to start weather & time updates", false);
	}

	// Start the TCP server to receive commands
	tcpServer.begin();
}
