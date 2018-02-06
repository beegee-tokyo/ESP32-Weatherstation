#include "setup.h"
#include "DHTesp.h"

extern BLECharacteristic *pCharacteristicNotify;
extern BLECharacteristic *pCharacteristicTemp;
extern BLECharacteristic *pCharacteristicHumid;
extern BLECharacteristic *pCharacteristicHeatIndex;
extern BLECharacteristic *pCharacteristicDewPoint;
extern BLECharacteristic *pCharacteristicComfort;
extern BLECharacteristic *pCharacteristicPerception;
// extern BLEAdvertising* pAdvertising;

void triggerGetTemp();
void triggerSendTemp();
void tempTask(void *pvParameters);
bool getTemperature();
String comfortRatioString(float newTempValue, float newHumidValue);
String computePerceptionString(float newTempValue, float newHumidValue);

/** Initialize DHT sensor */
DHTesp dht;
/** Task handle for the light value read task */
TaskHandle_t tempTaskHandle = NULL;
/** Pin number for DHT11 data pin */
int dhtPin = 17;
/** Ticker for temperature reading */
Ticker tempTicker;
/** Ticker for MQTT weather update every minute */
Ticker mqttTicker;
/** JSON as string to be sent to MQTT broker and UDP listeners */
String tempMsg;
/** Comfort profile */
ComfortState cf;

/**
 * initTemp
 * Setup DHT library
 * Setup task and timer for repeated measurement
 * @return bool
 *		true if task and timer are started
 *		false if task or timer couldn't be started
 */
bool initTemp() {
	byte resultValue = 0;
	// Initialize temperature sensor
	dht.setup(dhtPin, DHTesp::DHT11);
	// Serial.println("DHT initiated");

	// Start task to get temperature
	xTaskCreatePinnedToCore(
			tempTask,            /* Function to implement the task */
			"tempTask ",         /* Name of the task */
			4000,                /* Stack size in words */
			NULL,                /* Task input parameter */
			5,                   /* Priority of the task */
			&tempTaskHandle,     /* Task handle. */
			1);                  /* Core where the task should run */

	if (tempTaskHandle == NULL) {
		Serial.println(errorLabel + digitalTimeDisplaySec() + " Failed to start task for temperature update");
		addMqttMsg(debugLabel, errorLabel + digitalTimeDisplaySec() + " Failed to start task for temperature update", false);
		return false;
	} else {
		// Start update of environment data every 20 seconds
		tempTicker.attach(20, triggerGetTemp);
		// Start sending update of environment data by MQTT every 60 seconds
		mqttTicker.attach(60, triggerSendTemp);
	}
	return true;
}

/**
 * stopTemp
 * Detach ticker for temperature measurement
 * Detach ticker for mqtt
 * Stop temperature measurement task
 */
void stopTemp() {
	tempTicker.detach();
	mqttTicker.detach();
	if (tempTaskHandle != NULL) {
		vTaskSuspend(tempTaskHandle);
	}
}

/**
 * triggerGetTemp
 * Sets flag dhtUpdated to true for handling in loop()
 * called by Ticker tempTicker
 */
void triggerGetTemp() {
	if (tempTaskHandle != NULL) {
		 xTaskResumeFromISR(tempTaskHandle);
	}
}

/**
 * triggerSendTemp
 * Creates WEI JSON to be send to MQTT broker
 * and UDP listeners every 60 seconds
 * called by Ticker mqttTicker
 */
void triggerSendTemp() {
	addMqttMsg("WEI", tempMsg, false);
}

/**
 * Task to reads temperature from DHT11 sensor
 * @param pvParameters
 *		pointer to task parameters
 */
void tempTask(void *pvParameters) {
	// Serial.println("tempTask loop started");
	while (1) // tempTask loop
	{
		if (otaRunning)
		{
			vTaskDelete(NULL);
		}
		if (tasksEnabled) {
			getTemperature();
		}
		vTaskSuspend(NULL);
	}
}

/**
 * getTemperature
 * Reads temperature from DHT11 sensor
 * @return bool
 *		true if temperature could be aquired
 *		false if aquisition failed
*/
bool getTemperature() {
	tft.fillRect(0, 32, 128, 8, TFT_WHITE);
	tft.setCursor(0, 33);
	tft.setTextColor(TFT_BLACK);
	tft.setTextSize(0);
	tft.println("Getting temperature");
	tft.setTextColor(TFT_WHITE);

	// Reading temperature and humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
	TempAndHumidity lastValues = dht.getTempAndHumidity();

	// Check if any reads failed and exit early (to try again).
	if (dht.getStatus() != 0) {
		Serial.println("DHT11 error status: " + String(dht.getStatusString()));
		addMqttMsg(debugLabel, errorLabel + digitalTimeDisplaySec() + " DHT11 error status: " + String(dht.getStatusString()), false);
		tft.fillRect(0, 32, 128, 8, TFT_RED);
		tft.setCursor(0, 33);
		tft.setTextColor(TFT_BLACK);
		tft.setTextSize(0);
		tft.println("DHT11 failure");
		tft.setTextColor(TFT_WHITE);
		return false;
	}
	/******************************************************* */
	/* Trying to calibrate the humidity values							 */
	/******************************************************* */
	lastValues.humidity =	(int)(lastValues.humidity * 4.2);
	String displayTxt = "";

	tft.fillRect(0, 32, 128, 16, TFT_DARKGREY);
	tft.setCursor(0, 33);
	tft.setTextSize(1);
	tft.setTextColor(TFT_WHITE);
	tft.println("Local data at " + digitalTimeDisplay());

	tft.fillRect(0, 48, 128, 14, TFT_BLACK);
	tft.setTextSize(2);
	tft.setCursor(0,48);
	tft.setTextColor(TFT_WHITE);
	displayTxt = "I " + String(lastValues.temperature,0) + "'C " + String(lastValues.humidity,0) + "%";
	tft.print(displayTxt);

	float heatIndex = dht.computeHeatIndex(lastValues.temperature, lastValues.humidity);
	float dewPoint = dht.computeDewPoint(lastValues.temperature, lastValues.humidity);
	String comfortStatus = comfortRatioString(lastValues.temperature, lastValues.humidity);
	String humanPerception = computePerceptionString(lastValues.temperature, lastValues.humidity);
	// String dbgMessage = infoLabel + digitalTimeDisplaySec();
	// dbgMessage += " T: " + String(newTempValue) + " H: " + String(newHumidValue);
	// dbgMessage += " I: " + String(heatIndex) + " D: " + String(dewPoint);
	// dbgMessage += " C: " + comfortStatus + " P: " + humanPerception;
	// addMqttMsg(debugLabel, dbgMessage, false);


	// Send notification if any BLE client is connected
	if ((pServer != NULL) && (pServer->getConnectedCount() != 0)) {
		uint8_t tempData[2];
		uint16_t tempValue;
		tempValue = (uint16_t)(lastValues.temperature*100);
		tempData[1] = tempValue>>8;
		tempData[0] = tempValue;

		pCharacteristicTemp->setValue(tempData, 2);

		tempValue = (uint16_t)(lastValues.humidity*100);
		tempData[1] = tempValue>>8;
		tempData[0] = tempValue;
		pCharacteristicHumid->setValue(tempData, 2);

		tempValue = (uint16_t)(dewPoint);
		tempData[1] = 0;
		tempData[0] = tempValue;
		pCharacteristicDewPoint->setValue(tempData, 2);

		tempValue = (uint16_t)(heatIndex);
		tempData[1] = 0;
		tempData[0] = tempValue;
		pCharacteristicHeatIndex->setValue(tempData, 2);

		String bleStatus = "Comfort: " + comfortStatus;
		size_t dataLen = bleStatus.length();
		pCharacteristicComfort->setValue((uint8_t*)&bleStatus[0], dataLen);

		bleStatus = "Perception: " + humanPerception;
		dataLen = bleStatus.length();
		pCharacteristicPerception->setValue((uint8_t*)&bleStatus[0], dataLen);

		// Send notification to connected clients
		uint8_t notifData[8];
		time_t now;
		struct tm timeinfo;
		time(&now); // get time (as epoch)
		localtime_r(&now, &timeinfo); // update tm struct with current time
		uint16_t year = timeinfo.tm_year+1900;
		notifData[1] = year>>8;
		notifData[0] = year;
		notifData[2] = timeinfo.tm_mon+1;
		notifData[3] = timeinfo.tm_mday;
		notifData[4] = timeinfo.tm_hour;
		notifData[5] = timeinfo.tm_min;
		notifData[6] = timeinfo.tm_sec;
		pCharacteristicNotify->setValue(notifData, 8);

		pCharacteristicNotify->notify();
	}

 	/** Buffer for outgoing JSON string */
	DynamicJsonBuffer jsonOutBuffer;
	/** Json object for outgoing data */
	JsonObject& jsonOut = jsonOutBuffer.createObject();

	jsonOut["de"] = "wei";

	jsonOut["te"] = lastValues.temperature;
	jsonOut["hu"] = lastValues.humidity;
	jsonOut["hi"] = heatIndex;
	jsonOut["dp"] = dewPoint;
	jsonOut["cr"] = dht.getComfortRatio(cf, lastValues.temperature, lastValues.humidity);
	jsonOut["pe"] = dht.computePerception(lastValues.temperature, lastValues.humidity);

	tempMsg = "";
	// Message will be broadcasted every 60 seconds by triggerSendTemp
	jsonOut.printTo(tempMsg);
	return true;
}

String comfortRatioString(float newTempValue, float newHumidValue) {
	float cr = dht.getComfortRatio(cf, newTempValue, newHumidValue);
	switch(cf) {
		case Comfort_OK:
			return "OK";
			break;
		case Comfort_TooHot:
			return "Too Hot";
			break;
		case Comfort_TooCold:
			return "Too Cold";
			break;
		case Comfort_TooDry:
			return "Too Dry";
			break;
		case Comfort_TooHumid:
			return "Too Humid";
			break;
		case Comfort_HotAndHumid:
			return "Hot And Humid";
			break;
		case Comfort_HotAndDry:
			return "Hot And Dry";
			break;
		case Comfort_ColdAndHumid:
			return "Cold And Humid";
			break;
		case Comfort_ColdAndDry:
			return "Cold And Dry";
			break;
		default:
			return "Unknown:";
			break;
	};
}

String computePerceptionString(float newTempValue, float newHumidValue) {
	byte perception = dht.computePerception(newTempValue, newHumidValue);
	switch(perception) {
		case Perception_Dry:
			return "Dry";
			break;
		case Perception_VeryComfy:
			return "Very comfortable";
			break;
		case Perception_Comfy:
			return "Comfortable";
			break;
		case Perception_Ok:
			return "Just OK";
			break;
		case Perception_UnComfy:
			return "Somehow uncomfortable";
			break;
		case Perception_QuiteUnComfy:
			return "Uncomfortable, too humid";
			break;
		case Perception_VeryUnComfy:
			return "Very uncomfortable, too humid";
			break;
		case Perception_SevereUncomfy:
			return "Very uncomfortabl, much too humid";
			break;
		default:
			return "Unknown:";
			break;
	};
}
