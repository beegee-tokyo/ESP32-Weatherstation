#include "setup.h"

/**
 * Create Access Point name & mDNS name
 * from MAC address
 *
 * Created name is stored in the global char *apName[]
 **/
void createName(char *apName, int apIndex) {
	uint8_t baseMac[6];
	// Get MAC address for WiFi station
	esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
	char baseMacChr[18] = {0};
	sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);

	apName[apIndex] = baseMacChr[0];
	apName[apIndex+1] = baseMacChr[1];
	apName[apIndex+2] = baseMacChr[9];
	apName[apIndex+3] = baseMacChr[10];
	apName[apIndex+4] = baseMacChr[12];
	apName[apIndex+5] = baseMacChr[13];
	apName[apIndex+6] = baseMacChr[15];
	apName[apIndex+7] = baseMacChr[16];
}

/**
 * Connect to WiFi with defined method
 *
 */
/**
 * Connect with predefined SSID and password
 *
 * @param ssid
 *				SSID to connect to
 * @param password
 *				Password for this AP
 * @param timeout
 *				Time to wait for connection to succeed
 * @return <code>boolean</code>
 *				True if connection successfull
 *				False if connection failed after
 */
bool connDirect(const char *ssid, const char *password, uint32_t timeout) {
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);
	uint32_t startTime = millis();
	while (WiFi.waitForConnectResult() != WL_CONNECTED) {
		if (millis()-startTime > timeout) { // check if waiting time exceeded
			return false;
		}
	}
	return true;
}

/** Structure to keep the local time */
struct tm timeinfo;

/**
 * Initialize NTP client
 * @return <code>bool</code>
 *		true if time was updated within 10 seconds
 */
bool initNTP() {
	configTzTime("UTC-8:00","0.asia.pool.ntp.org", "1.asia.pool.ntp.org", "2.asia.pool.ntp.org");
	if (getLocalTime(&timeinfo, 10000)) {  // wait up to 10sec to sync
		return true;
	}
	return false;
}

/**
 * Try to get time from NTP server
 * @return <code>bool</code>
 *		true if time was updated
 */
bool tryGetTime() {
	if (getLocalTime(&timeinfo, 0)) {  // don't wait for  sync
		return true;
	}
	return false;
}

/**
 * Generate a string from a integer with leading zero if integer is smaller than 10
 * @param int
 *			Integer to be converted
 * @return <code>String</code>
 *			Integer as String
 */
String getDigits(int digits) {
	if (digits < 10) {
		return "0" + String(digits);
	} else {
		return String(digits);
	}
}

/**
 * Generate a string with formatted time
 * @return <code>String</code>
 *			String with current time as hh:mm
 */
String digitalTimeDisplay() {
	time_t now;
	time(&now); // get time (as epoch)
	localtime_r(&now, &timeinfo); // update tm struct with current time

	// digital clock display of the time as string
	String dateTime = String(timeinfo.tm_hour) + ":";
	dateTime += getDigits(timeinfo.tm_min);
	return dateTime;
}

/**
 * Generate a string with formatted time
 * @return <code>String</code>
 *			String with current time as hh:mm:ss
 */
String digitalTimeDisplaySec() {
	time_t now;
	time(&now); // get time (as epoch)
	localtime_r(&now, &timeinfo); // update tm struct with current time

	// digital clock display of the time as string
	String dateTime = String(timeinfo.tm_hour) + ":";
	dateTime += getDigits(timeinfo.tm_min) + ":";
	dateTime += getDigits(timeinfo.tm_sec);
	return dateTime;
}

