#include "setup.h"
#include "declarations.h"

/** Build time */
const char compileDate[] = __DATE__ " " __TIME__;
/** OTA progress */
int otaStatus = 0;
/**********************************************************/
// Fill these with your WiFi AP credentials
/**********************************************************/
/** Predefined SSID used for WiFi connection */
const char *ssid = "YOUR_NETWORK_SSID_HERE";
/** Predefined password used for WiFi connection */
const char *password = "YOUR_NETWORK_PASSWORD_HERE";
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

/**************************************************************************/
/*
    Arduino setup function (automatically called at startup)
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

	// tft.fillRect(0, 0, 128, 8, TFT_WHITE);
	// delay(2000);
	// tft.fillRect(0, 8, 128, 8, TFT_YELLOW);
	// delay(2000);
	// tft.fillRect(0, 16, 128, 8, TFT_GREEN);
	// delay(2000);
	// tft.fillRect(0, 24, 128, 8, TFT_RED);
	// delay(2000);
	// tft.fillRect(0, 32, 128, 8, TFT_BLUE);
	// delay(2000);
	// tft.fillRect(0, 40, 128, 8, TFT_ORANGE);
	// delay(30000);

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

	// Initialize MEEO connection
	initMeeo();

	// Initialize Light measurement
	byte lightInitResult = initLight();
	switch (lightInitResult) {
		case 0:
			Serial.println("[INFO] --:-- Light sensors available and initialized");
			addMeeoMsg("", "[INFO] --:-- Light sensors available and initialized", true);
			break;
		case 1:
			Serial.println("[ERROR] --:-- Failed to start task for light measurement");
			addMeeoMsg("", "[ERROR] --:-- Failed to start task for light measurement", true);
			break;
		case 2:
		default:
			Serial.println("[ERROR] --:-- Failed to start timer for light measurement");
			addMeeoMsg("", "[ERROR] --:-- Failed to start timer for light measurement", true);
			break;
	}

	// Initialize temperature measurements
	if (!initTemp()) {
		Serial.println("[ERROR] --:-- Failed to start temperature measurement");
		addMeeoMsg("", "[ERROR] --:-- Failed to start temperature measurement", true);
	}

	// Initialize Weather and NTP time updates
	if (!initWeather()) {
		Serial.println("[ERROR] --:-- Failed to start weather & time updates");
		addMeeoMsg("", "[ERROR] --:-- Failed to start weather & time updates", true);
	}
}

/**
 * Activate OTA
 *
 * requires
 *	#define MODULTYPE (e.g. aircon, light, ....)
 *	#define MODULID (e.g. ac1, lb1, ...)
 */
void activateOTA(const char *MODULTYPE, const char *MODULID) {

	// ArduinoOTA.setHostname(apName);
	ArduinoOTA
		.setHostname(apName)
    .onStart([]() {
			addMeeoMsg("", "[INFO] " + digitalTimeDisplaySec() + " OTA_START", true);
			stopAllTimers();
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
    .onProgress([](unsigned int progress, unsigned int total) {
			unsigned int achieved = progress / (total / 100);
			if (otaStatus == 0 || achieved == otaStatus + 1) {
				digitalWrite(16, !digitalRead(16));
				otaStatus = achieved;
				// tft.fillScreen(TFT_BLUE);
				tft.setTextDatum(MC_DATUM);
				// tft.setTextColor(TFT_WHITE);
				tft.setTextSize(2);
				// tft.drawString("OTA",64,50);
				// tft.drawString("Progress:",64,75);

				tft.fillRect(32,91,64,28,TFT_BLUE);
				String progVal = String(achieved) + "%";
				tft.drawString(progVal,64,105);
			}
    })
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

	// ArduinoOTA.setHostname(apName);
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

// 	ArduinoOTA.onStart([]() {
// 		addMeeoMsg("", "[INFO] " + digitalTimeDisplaySec() + " OTA_START", true);
// 		stopAllTimers();
// 		otaRunning = true;
//
// 		pinMode(16, OUTPUT);
// 		String type;
// 		if (ArduinoOTA.getCommand() == U_FLASH)
// 			type = "sketch";
// 		else // U_SPIFFS
// 			type = "filesystem";
//
// 		// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
// 		Serial.println("OTA Start updating " + type);
//
// #ifdef HAS_TFT
// 		// tft.fillScreen(TFT_RED);
// 		// tft.setTextDatum(MC_DATUM);
// 		// tft.setTextColor(TFT_BLACK);
// 		// tft.setTextSize(3);
// 		// tft.drawString("OTA!",64,64);
//
// 		tft.fillScreen(TFT_BLUE);
// 		tft.setTextDatum(MC_DATUM);
// 		tft.setTextColor(TFT_WHITE);
// 		tft.setTextSize(2);
// 		tft.drawString("OTA",64,50);
// 		tft.drawString("Progress:",64,75);
// #endif
// 	});
//
// 	ArduinoOTA.onEnd([]() {
// 		Serial.println("\n OTA End");
//
// #ifdef HAS_TFT
// 		tft.fillScreen(TFT_GREEN);
// 		tft.setTextDatum(MC_DATUM);
// 		tft.setTextColor(TFT_BLACK);
// 		tft.setTextSize(2);
// 		tft.drawString("OTA",64,50);
// 		tft.drawString("FINISHED!",64,80);
// #endif
// 		delay(10);
// 	});
//
// #ifdef HAS_TFT
// 	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
// 		unsigned int achieved = progress / (total / 100);
// 		if (otaStatus == 0 || achieved == otaStatus + 1) {
// 			digitalWrite(16, !digitalRead(16));
// 			otaStatus = achieved;
// 			// tft.fillScreen(TFT_BLUE);
// 			tft.setTextDatum(MC_DATUM);
// 			// tft.setTextColor(TFT_WHITE);
// 			tft.setTextSize(2);
// 			// tft.drawString("OTA",64,50);
// 			// tft.drawString("Progress:",64,75);
//
// 			tft.fillRect(32,91,64,28,TFT_BLUE);
// 			String progVal = String(achieved) + "%";
// 			tft.drawString(progVal,64,105);
// 		}
// 	});
// #endif
//
// 	ArduinoOTA.onError([](ota_error_t error) {
// #ifdef HAS_TFT
// 		tft.fillScreen(TFT_RED);
// 		tft.setTextDatum(MC_DATUM);
// 		tft.setTextColor(TFT_BLACK);
// 		tft.setTextSize(2);
// 		tft.drawString("OTA",64,30,2);
// 		tft.drawString("ERROR:",64,60,2);
// #endif
//
// 		Serial.printf("\nOTA Error[%u]: ", error);
// 		if (error == OTA_AUTH_ERROR) {
// 			Serial.println("Auth Failed");
//
// #ifdef HAS_TFT
// 			tft.drawString("Auth Failed",64,80,2);
// #endif
// 		}
// 		else if (error == OTA_BEGIN_ERROR) {
// 			Serial.println("Begin Failed");
//
// #ifdef HAS_TFT
// 			tft.drawString("Begin Failed",64,80,2);
// #endif
// 		}
// 		else if (error == OTA_CONNECT_ERROR) {
// 			Serial.println("Connect Failed");
// #ifdef HAS_TFT
// 			tft.drawString("Connect Failed",64,80,2);
// #endif
// 		}
// 		else if (error == OTA_RECEIVE_ERROR) {
// 			Serial.println("Receive Failed");
// #ifdef HAS_TFT
// 			tft.drawString("Receive Failed",64,80,2);
// #endif
// 		}
// 		else if (error == OTA_END_ERROR) {
// 			Serial.println("End Failed");
// #ifdef HAS_TFT
// 			tft.drawString("End Failed",64,80,2);
// #endif
// 		}
// 	});
}

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
