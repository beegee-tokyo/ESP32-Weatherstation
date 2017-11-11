#include "setup.h"

/**********************************************************/
// Fill these with your WiFi AP credentials
/**********************************************************/
/** Predefined SSID used for WiFi connection */
const char *ssid = "YOUR_NETWORK_SSID_HERE";
/** Predefined password used for WiFi connection */
const char *password = "YOUR_NETWORK_PASSWORD_HERE";

/**
 * Connect to WiFi with pre-defined method
 *
 * @return <code>bool</code>
 *			true if connection was successful
 *			false if connection failed
 **/
bool connectWiFi() {
#ifdef CONNDIRECT
#ifdef ENA_DEBUG
	Serial.println("Connect with predefined SSID and password");
#endif
	// /** Connect with predefined SSID and password */
	if (connDirect(ssid, password, 20000)) {
		return true;
	} else {
		return false;
	}
#elif defined CONNWIFIMANAGER
#ifdef ENA_DEBUG
	Serial.println("Connect with WiFiManager");
#endif
	/** Connect with WiFiManager */
	if (connWiFiManager(apName, 10000, 0, true)) {
		return true;
	} else {
		return false;
	}
#elif defined CONNSMARTCONFIG
#ifdef ENA_DEBUG
	Serial.println("Connect with SmartConfig");
#endif
	/** Connect with SmartConfig */
	WiFi.begin();
	WiFi.reconnect();
	uint32_t startTime = millis();
	while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		if (millis()-startTime > 10000) { // check if waiting time exceeded
			if (connSmartConfig(60000, 10000)) {
				return false;
			}
		}
	}
	return true;
#else
	// We end up here if no connection method was defined !!!!!
	WiFi.begin();
	WiFi.reconnect();
	uint32_t startTime = millis();
	while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		if (millis()-startTime > 10000) { // check if waiting time exceeded
			return false;
		}
	}
	return true;
#endif
}
