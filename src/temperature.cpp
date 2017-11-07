#include "setup.h"

void triggerGetTemp();
void tempTask(void *pvParameters);
bool getTemperature();
float computeHeatIndex(float temperature, float percentHumidity);

/** Initialize DHT sensor */
DHT dht;
/** Task handle for the light value read task */
TaskHandle_t tempTaskHandle = NULL;
/** Timer to collect temperature & humidity information */
hw_timer_t *getTempTimer = NULL;
/** Pin number for DHT11 data pin */
int dhtPin = 17;
/** Counter to update local weather and time */
byte weatherCountDown = 0;

/**
	initTemp
	Setup DHT library
	Setup task and timer for repeated measurement
  @return bool
      true if task and timer are started
      false if task or timer couldn't be started
*/
bool initTemp() {
  byte resultValue = 0;
  // Initialize temperature sensor
	dht.setup(dhtPin, DHT::DHT11);
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
    return false;
  } else {
    // Start update of environment data every 20 seconds
  	getTempTimer = startTimerSec(20, triggerGetTemp, true);
  	if (getTempTimer == NULL) {
      vTaskDelete(tempTaskHandle);
      return false;
  	}
  }

  return true;

	// Start update of environment data every 20 seconds
	getTempTimer = startTimerSec(20, triggerGetTemp, true);
	if (getTempTimer == NULL) {
		Serial.println("[ERROR] --:-- Failed to start timer for temperature update");
		addMeeoMsg("", "[ERROR] --:-- Failed to start timer for temperature update", true);
	}
}

/**
	triggerGetTemp
	Sets flag dhtUpdated to true for handling in loop()
	called by Ticker getTempTimer
*/
void triggerGetTemp() {
  if (tempTaskHandle != NULL) {
	   xTaskResumeFromISR(tempTaskHandle);
  }
	weatherCountDown++;
	if ((weatherCountDown > 90) && (weatherTaskHandle != NULL)) { // 30 min = 1800 sec / 20 sec update rate = 90 triggers
		addMeeoMsg("", "[INFO] " + digitalTimeDisplay() + " Weather triggered", true);
		weatherCountDown = 0;
		xTaskResumeFromISR(weatherTaskHandle);
	}
}

/**
 * Task to reads temperature from DHT11 sensor
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
			if (!getTemperature()) {
				Serial.println("Could not get temperature");
				addMeeoMsg("", "[ERROR] " + digitalTimeDisplaySec() + " Could not get temperature", true);
			// } else {
			// 	addMeeoMsg("", "[INFO] " + digitalTimeDisplaySec() + " Got temperature", true);
			}
		}
		vTaskSuspend(NULL);
	}
}

/**
	getTemperature
	Reads temperature from DHT11 sensor
*/
bool getTemperature() {
	tft.fillRect(0, 32, 128, 8, TFT_WHITE);
	tft.setCursor(0, 33);
	tft.setTextColor(TFT_BLACK);
	tft.setTextSize(0);
	tft.println("Getting temperature");

	// Reading temperature for humidity takes about 250 milliseconds!
	// Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
	float newHumidValue = dht.getHumidity();          // Read humidity (percent)
	float newTempValue = dht.getTemperature();     // Read temperature as Celsius
	// Check if any reads failed and exit early (to try again).
	if (dht.getStatus() != 0) {
		Serial.println("DHT11 error status: " + String(dht.getStatusString()));
		addMeeoMsg("", "[ERROR] " + digitalTimeDisplay() + " DHT11 error status: " + String(dht.getStatusString()), true);
		tft.fillRect(0, 32, 128, 8, TFT_RED);
		tft.setCursor(0, 33);
		tft.setTextColor(TFT_BLACK);
		tft.setTextSize(0);
		tft.println("DHT11 failure");
		return false;
	}
	/******************************************************* */
	/* Trying to calibrate the humidity values               */
	/******************************************************* */
	// newHumidValue = 10*sqrt(newHumidValue);
	newHumidValue = 20+newHumidValue;

	String displayTxt = "";

  tft.fillRect(0, 32, 128, 8, TFT_DARKGREY);
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

	addMeeoMsg("temp", String(newTempValue)+"C", false);
	addMeeoMsg("humid", String(newHumidValue), false);
	float heatIndex = computeHeatIndex(newTempValue, newHumidValue);
	addMeeoMsg("heat", String(heatIndex), false);

	String udpMsg = "{\"de\":\"wei\",\"te\":" + String(newTempValue) + ",\"hu\":" + String(newHumidValue) + ",\"he\":" + String(heatIndex) + "}";
	IPAddress pcIP (192,	168, 0, 110);
	IPAddress multiIP (192,	168, 0, 255);
	udpSendMessage(pcIP, udpMsg, 9997);
	udpSendMessage(multiIP, udpMsg, 9997);
	return true;
}

float computeHeatIndex(float temperature, float percentHumidity) {
  // Using both Rothfusz and Steadman's equations
  // http://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml
  float hi;

  hi = 0.5 * (temperature + 61.0 + ((temperature - 68.0) * 1.2) + (percentHumidity * 0.094));

  if (hi > 79) {
    hi = -42.379 +
             2.04901523 * temperature +
            10.14333127 * percentHumidity +
            -0.22475541 * temperature*percentHumidity +
            -0.00683783 * pow(temperature, 2) +
            -0.05481717 * pow(percentHumidity, 2) +
             0.00122874 * pow(temperature, 2) * percentHumidity +
             0.00085282 * temperature*pow(percentHumidity, 2) +
            -0.00000199 * pow(temperature, 2) * pow(percentHumidity, 2);

    if((percentHumidity < 13) && (temperature >= 80.0) && (temperature <= 112.0))
      hi -= ((13.0 - percentHumidity) * 0.25) * sqrt((17.0 - abs(temperature - 95.0)) * 0.05882);

    else if((percentHumidity > 85.0) && (temperature >= 80.0) && (temperature <= 87.0))
      hi += ((percentHumidity - 85.0) * 0.1) * ((87.0 - temperature) * 0.2);
  }

  return hi;
}
