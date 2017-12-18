#include "setup.h"
#include "DHTesp.h"

void triggerGetTemp();
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
/** Comfort profile */
ComfortState cf;

/**
 * initTemp
 * Setup DHT library
 * Setup task and timer for repeated measurement
 * @return bool
 *    true if task and timer are started
 *    false if task or timer couldn't be started
 */
bool initTemp() {
  byte resultValue = 0;
  // Initialize temperature sensor
	dht.setup(dhtPin, DHTesp::DHT11);
	Serial.println("DHT initiated");

  // Start task to get temperature
	xTaskCreatePinnedToCore(
			tempTask,                       /* Function to implement the task */
			"tempTask ",                    /* Name of the task */
			4000,                           /* Stack size in words */
			NULL,                           /* Task input parameter */
			5,                              /* Priority of the task */
			&tempTaskHandle,                /* Task handle. */
			1);                             /* Core where the task should run */

  if (tempTaskHandle == NULL) {
    Serial.println("[ERROR] " + digitalTimeDisplaySec() + " Failed to start task for temperature update");
    addMqttMsg("debug", "[ERROR] " + digitalTimeDisplaySec() + " Failed to start task for temperature update", false);
    return false;
  } else {
    // Start update of environment data every 20 seconds
    tempTicker.attach(20, triggerGetTemp);
  }
  return true;
}

/**
 * triggerGetTemp
 * Sets flag dhtUpdated to true for handling in loop()
 * called by Ticker getTempTimer
 */
void triggerGetTemp() {
  if (tempTaskHandle != NULL) {
	   xTaskResumeFromISR(tempTaskHandle);
  }
}

/**
 * Task to reads temperature from DHT11 sensor
 * @param pvParameters
 *    pointer to task parameters
 */
void tempTask(void *pvParameters) {
	Serial.println("tempTask loop started");
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
 *    true if temperature could be aquired
 *    false if aquisition failed
*/
bool getTemperature() {
	tft.fillRect(0, 32, 128, 8, TFT_WHITE);
	tft.setCursor(0, 33);
	tft.setTextColor(TFT_BLACK);
	tft.setTextSize(0);
	tft.println("Getting temperature");
  tft.setTextColor(TFT_WHITE);

	// Reading temperature for humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
	float newHumidValue = dht.getHumidity();          // Read humidity (percent)
	float newTempValue = dht.getTemperature();     // Read temperature as Celsius
	// Check if any reads failed and exit early (to try again).
	if (dht.getStatus() != 0) {
		Serial.println("DHT11 error status: " + String(dht.getStatusString()));
    addMqttMsg("debug", "[ERROR] " + digitalTimeDisplaySec() + " DHT11 error status: " + String(dht.getStatusString()), false);
		tft.fillRect(0, 32, 128, 8, TFT_RED);
		tft.setCursor(0, 33);
		tft.setTextColor(TFT_BLACK);
		tft.setTextSize(0);
		tft.println("DHT11 failure");
    tft.setTextColor(TFT_WHITE);
		return false;
	}
	/******************************************************* */
	/* Trying to calibrate the humidity values               */
	/******************************************************* */
	// newHumidValue = 10*sqrt(newHumidValue);
	newHumidValue = (int)(20.0 + newHumidValue)*1.6;

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
  displayTxt = "I " + String(newTempValue,0) + "'C " + String(newHumidValue,0) + "%";
	tft.print(displayTxt);

	float heatIndex = dht.computeHeatIndex(newTempValue, newHumidValue);
  float dewPoint = dht.computeDewPoint(newTempValue, newHumidValue);
  String comfortStatus = comfortRatioString(newTempValue, newHumidValue);
  String humanPerception = computePerceptionString(newTempValue, newHumidValue);
  String dbgMessage = "[INFO] " + digitalTimeDisplaySec();
  dbgMessage += " T: " + String(newTempValue) + " H: " + String(newHumidValue);
  dbgMessage += " I: " + String(heatIndex) + " D: " + String(dewPoint);
  dbgMessage += " C: " + comfortStatus + " P: " + humanPerception;
  addMqttMsg("debug", dbgMessage, false);

  if (bleConnected) {
    bleNewData = dbgMessage;
    String notifString = digitalTimeDisplaySec();
    size_t dataLen = notifString.length();
    uint8_t notifData[dataLen+1];
    notifString.toCharArray((char *)notifData,dataLen+1);
    pCharacteristicNotify->setValue(notifData, dataLen);
    pCharacteristicNotify->notify();
  }

 	/** Buffer for outgoing JSON string */
	DynamicJsonBuffer jsonOutBuffer;
	/** Json object for outgoing data */
	JsonObject& jsonOut = jsonOutBuffer.createObject();

	jsonOut["de"] = "wei";

	jsonOut["te"] = newTempValue;
	jsonOut["hu"] = newHumidValue;
	jsonOut["hi"] = heatIndex;

	String udpMsg;
  jsonOut.printTo(udpMsg);
  addMqttMsg("WEI", udpMsg, false);

	IPAddress pcIP (192,	168, 0, 110);
	IPAddress multiIP (192,	168, 0, 255);
	udpSendMessage(pcIP, udpMsg, 9997);
	udpSendMessage(multiIP, udpMsg, 9997);
	return true;
}

String comfortRatioString(float newTempValue, float newHumidValue) {
  float cr = dht.getComfortRatio(cf, newTempValue, newHumidValue);
  switch(cf) {
    case Comfort_OK:
      return "Comfort OK";
      break;
    case Comfort_TooHot:
      return "Comfort Too Hot";
      break;
    case Comfort_TooCold:
      return "Comfort Too Cold";
      break;
    case Comfort_TooDry:
      return "Comfort Too Dry";
      break;
    case Comfort_TooHumid:
      return "Comfort Too Humid";
      break;
    case Comfort_HotAndHumid:
      return "Comfort Hot And Humid";
      break;
    case Comfort_HotAndDry:
      return "Comfort Hot And Dry";
      break;
    case Comfort_ColdAndHumid:
      return "Comfort Cold And Humid";
      break;
    case Comfort_ColdAndDry:
      return "Comfort Cold And Dry";
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
