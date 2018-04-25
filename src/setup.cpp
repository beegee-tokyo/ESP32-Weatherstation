#include "setup.h"
#include "declarations.h"
#include "esp_now.h"

#include "preferences.h"

/** Build time */
const char compileDate[] = __DATE__ " " __TIME__;

/**
 * Arduino setup function (called once after boot/reboot)
 */
/**************************************************************************/
void setup(void)
{
	// Setup labels
	debugLabel = "debug";
	infoLabel = "[I] ";
	errorLabel = "[E] ";

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
	tft.setTextSize(1);

#ifdef ENA_DEBUG
	Serial.print("Build: ");
	Serial.println(compileDate);
#endif
	tft.println("Build: ");
	tft.println(compileDate);

	// Create Access Point name & mDNS name from MAC address
	createName();

	Preferences preferences;
	preferences.begin(PREF_NAME, false);
	bool hasPref = preferences.getBool(VALID_KEY, false);
	if (hasPref) {
		ssidPrim = preferences.getString(SSID_PRIM_KEY,"");
		ssidSec = preferences.getString(SSID_SEC_KEY,ssidPrim);
		pwPrim = preferences.getString(PW_PRIM_KEY,"");
		pwSec = preferences.getString(PW_SEC_KEY,pwPrim);
		devLoc = preferences.getString(DEV_LOC_KEY,"Office");
		devID = preferences.getString(DEV_ID_KEY,"tb1");
		devType = preferences.getString(DEV_TYPE_KEY,"Tst");

		ssidPrim = preferences.getString(SSID_PRIM_KEY,"");
		ssidSec = preferences.getString(SSID_SEC_KEY,ssidPrim);
		pwPrim = preferences.getString(PW_PRIM_KEY,"");
		pwSec = preferences.getString(PW_SEC_KEY,pwPrim);
		devLoc = preferences.getString(DEV_LOC_KEY,"Office");
		devID = preferences.getString(DEV_ID_KEY,"tb1");
		devType = preferences.getString(DEV_TYPE_KEY,"Tst");

		String dbgMsg = "Read from preferences:";
		sendDebug(debugLabel, dbgMsg, false);
		dbgMsg = "primary SSID: "+ssidPrim+" password: "+pwPrim;
		sendDebug(debugLabel, dbgMsg, false);
		dbgMsg = "secondary SSID: "+ssidSec+" password: "+pwSec;
		sendDebug(debugLabel, dbgMsg, false);

		// Show status
		tft.fillRect(120, 32, 8, 9, TFT_RED);

		// Initialize WiFi connection
		connectInit();

	} else {
		sendDebug(debugLabel, "Could not find preferences, need send data over BLE", false);
		ssidPrim = "-";
		ssidSec = "-";
		pwPrim = "-";
		pwSec = "-";
		devLoc = "Office";
		devID = "tb1";
		devType = "Test";
	}
	preferences.end();

	tft.println("Waiting for");
	tft.println("connection");

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

	// At the moment either BLE or Bluetooth Serial can be used.
	// They are not working both at the same time.
	// Initialize only one of the two!!!!!!

	// Initialize BLE server
	// param 0 for BLE only
	// param 1 for Serial BT only
	// param 2 for both
	// initBlueTooth(0);

	// If there are no WiFi credentials stored, start Bluetooth to receive them
	if (!hasPref) {
		initBLE();
	} else {
		stopBLE();
	}
	// Initialize touch interface
	initTouch();

	// Initialize LED flashing
	initLed();
	startFlashing(1000);
}
