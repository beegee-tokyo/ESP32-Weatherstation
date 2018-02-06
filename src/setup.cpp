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

	// Initialize MQTT connection
	initMqtt();

	// Get Partitionsizes
	size_t ul;
	esp_partition_iterator_t _mypartiterator;
	const esp_partition_t *_mypart;
	ul = spi_flash_get_chip_size(); Serial.print("Flash chip size: "); Serial.println(ul);
	Serial.println("Partition table:");
	char mqttMsg[1024];

	_mypartiterator = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
	if (_mypartiterator) {
		Serial.println("App Partition table:");
		addMqttMsg(debugLabel, infoLabel + digitalTimeDisplaySec() + " App partition table:", false);
		do {
			_mypart = esp_partition_get(_mypartiterator);
			printf("Type: %02x SubType %02x Address 0x%06X Size 0x%06X Encryption %i Label %s\n", _mypart->type, _mypart->subtype, _mypart->address, _mypart->size, _mypart->encrypted, _mypart->label);
			sprintf(mqttMsg,"Type: %02x SubType %x Address 0x%06X Size 0x%06X Encryption %i Label %s", _mypart->type, _mypart->subtype, _mypart->address, _mypart->size, _mypart->encrypted, _mypart->label);
			addMqttMsg(debugLabel, infoLabel + String(mqttMsg), false);
		} while (_mypartiterator = esp_partition_next(_mypartiterator));
	}
	esp_partition_iterator_release(_mypartiterator);
	_mypartiterator = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
	if (_mypartiterator) {
		Serial.println("Data Partition table:");
		addMqttMsg(debugLabel, infoLabel + digitalTimeDisplaySec() + " Data partition table:", false);
		do {
			_mypart = esp_partition_get(_mypartiterator);
			printf("Type: %02x SubType %02x Address 0x%06X Size 0x%06X Encryption %i Label %s\n", _mypart->type, _mypart->subtype, _mypart->address, _mypart->size, _mypart->encrypted, _mypart->label);
			sprintf(mqttMsg,"Type: %02x SubType %02x Address 0x%06X Size 0x%06X Encryption %i Label %s", _mypart->type, _mypart->subtype, _mypart->address, _mypart->size, _mypart->encrypted, _mypart->label);
			addMqttMsg(debugLabel, infoLabel + String(mqttMsg), false);
		} while (_mypartiterator = esp_partition_next(_mypartiterator));
	}
	esp_partition_iterator_release(_mypartiterator);

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
			addMqttMsg(debugLabel, infoLabel + digitalTimeDisplaySec() + " Light task initialized", false);
			break;
		case 1:
		default:
			// Serial.println(errorLabel + digitalTimeDisplaySec() + " Failed to start timer for light measurement");
			addMqttMsg(debugLabel, errorLabel + digitalTimeDisplaySec() + " Failed to start timer for light measurement", false);
			break;
	}

	// Initialize temperature measurements
	if (!initTemp()) {
		// Serial.println(errorLabel + digitalTimeDisplaySec() + " Failed to start temperature measurement");
		addMqttMsg(debugLabel, errorLabel + digitalTimeDisplaySec() + " Failed to start temperature measurement", false);
	}

	// Initialize Weather and NTP time updates
	if (!initUGWeather()) {
		// Serial.println(errorLabel + digitalTimeDisplaySec() + " Failed to start weather & time updates");
		addMqttMsg(debugLabel, errorLabel + digitalTimeDisplaySec() + " Failed to start weather & time updates", false);
	}

	// Get last reset reason and publish it
	String resetReason = reset_reason(rtc_get_reset_reason(0));
	addMqttMsg(debugLabel, infoLabel + digitalTimeDisplaySec() + " Reset reason CPU0: " + resetReason, false);
	resetReason = reset_reason(rtc_get_reset_reason(1));
	addMqttMsg(debugLabel, infoLabel + digitalTimeDisplaySec() + " Reset reason CPU1: " + resetReason, false);

	addMqttMsg(debugLabel, infoLabel + digitalTimeDisplaySec() + " SDK Version: " + ESP.getSdkVersion(), false);

	// Initialize SPI connection to ESP8266
	// Slave on ESP8266 not working atm !!!!!!!!!!!!!!!!
	// initSPI();
	// Serial.println("Result of SPI initializaton: " + String(initSPIidf(),HEX));
	// Serial.println("Result of SPI initializaton: 0x" + String(initSPIslave(),HEX));

	// Initialize I2C connection to ESP8266
	// initI2C();

	// Initialize BLE server
	initBLEserver();
}
