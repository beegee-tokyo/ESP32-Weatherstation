#include "setup.h"

/** Device identification */
String devID = "";
String devType = "";
String devLoc = "";

/** SSIDs of local WiFi networks */
String ssidPrim = "";
String ssidSec = "";
/** Password for local WiFi network */
String pwPrim = "";
String pwSec = "";

/** WiFiMulti class */
WiFiMulti wifiMulti;

/** Connection status */
volatile byte connStatus = CON_LOST;

/** UDP broadcast address */
IPAddress multiIP (192,	168, 0, 255);

/** Selected network 
    true = use primary network
		false = use secondary network
*/
bool usePrimAP = true;

/** Flag if two networks are found */
bool canSwitchAP = false;

/** Counter to measure connection time */
unsigned long wifiConnectStart;

/** Callback for connection loss */
void lostCon(system_event_id_t event) {
	if (connStatus == CON_GOTIP) {
		tft.fillRect(120, 32, 8, 9, TFT_RED);
		Serial.println("STA disconnect detected");
		connStatus = CON_LOST;
		// Switch on onboard LED to show status
		ledTicker.detach();
		digitalWrite(ledPin, LOW);
	}
}

/** Callback for connection to AP */
void gotCon(system_event_id_t event) {
	if (connStatus == CON_INIT) {
		tft.fillRect(120, 32, 8, 9, TFT_YELLOW);
		Serial.println("Got connection after " + String((millis()-wifiConnectStart)/1000) + "s");
		Serial.println("SSID: " + String(WiFi.SSID()));
		Serial.println("Channel: " + String(WiFi.channel()));
		Serial.println("RSSI: " + String(WiFi.RSSI()));
		connStatus = CON_START;
		// Flash onboard LED with 0.5Hz to show status
		startFlashing(250);
	}
}

/** Callback for receiving IP address from AP */
void gotIP(system_event_id_t event) {
	if (connStatus == CON_START) {
		tft.fillRect(120, 32, 8, 9, TFT_GREEN);
		Serial.print("Got IP after " + String((millis()-wifiConnectStart)/1000) + "s: ");
		Serial.println(WiFi.localIP());
		Serial.println("RSSI: " + String(WiFi.RSSI()));
		connStatus = CON_GOTIP;
		// Flash onboard LED with 1Hz to show status
		startFlashing(500);
		// Create UDP broadcast address
		IPAddress deviceIP = WiFi.localIP();
		multiIP[0] = deviceIP[0];
		multiIP[1] = deviceIP[1];
		multiIP[2] = deviceIP[2];
		multiIP[3] = 255;
	}
}

/** Callback for WiFi ready status */
void wifiReady(system_event_id_t event) {
	Serial.println("Wifi ready detected");
}

/**
 * Create Access Point name & mDNS name
 * from MAC address
 *
 * Created name is stored in the global char *apName[]
 **/
void createName() {
	uint8_t baseMac[6];
	// Get MAC address for WiFi station
	esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
	char baseMacChr[18] = {0};
	sprintf(baseMacChr, "%02X%02X%02X%02X", baseMac[0], baseMac[3], baseMac[4], baseMac[5]);

	memcpy((void*)&apName[apIndex],(void*)baseMacChr,8);
}

/**
	 scanWiFi
	 Scans for available networks 
	 and decides if a switch between
	 allowed networks makes sense

	 @return <code>bool</code>
	        True if at least one allowed network was found
*/
bool scanWiFi() {
	/** RSSI for primary network */
	int8_t rssiPrim;
	/** RSSI for secondary network */
	int8_t rssiSec;
	/** Result of this function */
	bool result = false;

	Serial.println("Start scanning for networks");

	WiFi.disconnect(true);
	WiFi.setAutoReconnect(false);
	WiFi.enableSTA(true);
	WiFi.mode(WIFI_STA);

	wifiConnectStart = millis();
	// int apNum = WiFi.scanNetworks();
	int apNum = WiFi.scanNetworks(false,true,false,5000);
	if (apNum == 0) {
		Serial.println("Found no networks?????");
		return false;
	}
	
	byte foundAP = 0;
	bool foundPrim = false;

	for (int index=0; index<apNum; index++) {
		String ssid = WiFi.SSID(index);
		Serial.println("Found AP: " + ssid + " RSSI: " + WiFi.RSSI(index));
		tft.println(ssid + " - " + WiFi.RSSI(index));
		// tft.printf("%s - %d\n",ssid, WiFi.RSSI(index));
		if (!strcmp((const char*) &ssid[0], (const char*) &ssidPrim[0])) {
			Serial.println("Found primary AP");
			foundAP++;
			foundPrim = true;
			rssiPrim = WiFi.RSSI(index);
		}
		if (!strcmp((const char*) &ssid[0], (const char*) &ssidSec[0])) {
			Serial.println("Found secondary AP");
			foundAP++;
			rssiSec = WiFi.RSSI(index);
		}
	}

	Serial.printf("Found %d AP's\n", foundAP);

	switch (foundAP) {
		case 0:
			result = false;
			break;
		case 1:
			if (foundPrim) {
				usePrimAP = true;
			} else {
				usePrimAP = false;
			}
			result = true;
			break;
		default:
		tft.printf("RSSI Prim: %d Sec: %d\n", rssiPrim, rssiSec);
		Serial.printf("RSSI Prim: %d Sec: %d\n", rssiPrim, rssiSec);
			if (rssiPrim > rssiSec) {
				usePrimAP = true; // RSSI of primary network is better
			} else {
				usePrimAP = false; // RSSI of secondary network is better
			}
			result = true;
			break;
	}
	Serial.println("WiFi scan finished after " + String((millis()-wifiConnectStart)/1000) + "s");
	return result;
}

/**
 * Start connection to AP
 */
void connectWiFi() {
	WiFi.disconnect(true);
	// WiFi.setAutoReconnect(false);
	WiFi.enableSTA(true);
	WiFi.mode(WIFI_STA);

	Serial.println();
	Serial.print("Start connection to ");
	tft.print("Try ");
	if (usePrimAP) {
		Serial.println(ssidPrim);
		tft.println(ssidPrim);
		WiFi.begin(ssidPrim.c_str(), pwPrim.c_str());
	} else {
		Serial.println(ssidSec);
		tft.println(ssidSec);
		WiFi.begin(ssidSec.c_str(), pwSec.c_str());
	}
	connStatus = CON_INIT;

	wifiConnectStart = millis();
}

/**
 * Initiates connection to AP
 * Setup callback handlers
 * Scan WiFi network
 * Start connection to WiFi network
 *
 */
void connectInit() {
	// Set WiFi into STA mode
	WiFi.disconnect();
	WiFi.mode(WIFI_STA);
	// Setup callback function for WiFi is connectedy
	WiFi.onEvent(gotCon, SYSTEM_EVENT_STA_CONNECTED);
	// Setup callback function for WiFi received IP address
	WiFi.onEvent(gotIP, SYSTEM_EVENT_STA_GOT_IP);
	// Setup callback function for WiFi lost connection
	WiFi.onEvent(lostCon, SYSTEM_EVENT_STA_DISCONNECTED);
	// Setup callback function for WiFi is ready
	WiFi.onEvent(wifiReady, SYSTEM_EVENT_WIFI_READY);

	wifiMulti.addAP(ssidPrim.c_str(), pwPrim.c_str());
	wifiMulti.addAP(ssidSec.c_str(), pwSec.c_str());

	Serial.println();
	Serial.println("Start MultiWiFi with");
	Serial.println("Primary: " + ssidPrim + " " + pwPrim);
	Serial.println("Secondary: " + ssidSec + " " + pwSec);

	connStatus = CON_INIT;
	wifiMulti.run();
	wifiConnectStart = millis();

	// // Scan for available WiFi networks
	// if (!scanWiFi()) {
	// 	Serial.println("Scan failed");
	// 	connStatus = CON_LOST;
	// 	// delay(1000);
	// 	// esp_restart();
	// }

	// // Try to connect to a WiFi network
	// connectWiFi();
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
