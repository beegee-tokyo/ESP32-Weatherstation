#include "setup.h"
#include "ugweather.h"

bool getUGWeather();
void triggerGetUGWeather();

/** Underground Weather API key */
const String wgApiKey = "e062ec62675eb278";
/** Country for Underground Weather */
const String wgCountry = "PH";
/** City for Underground Weather */
const String wgCity = "Paranaque_City";
/** URL for Underground Weather */
const String wgURL = "http://api.wunderground.com/api/";
/** Current weather as readable string */
String ugWeatherText = "";

/**
 * initWeather
 * Setup task for repeated weather and time update
 * @return bool
 *		true if task is started
 *		false if task couldn't be started
 */
bool initUGWeather() {
	// Start NTP listener
	initNTP();
	tryGetTime();

	// Start task to get weather & time updates
	xTaskCreatePinnedToCore(
			ugWeatherTask,        /* Function to implement the task */
			"ugWeatherTask ",     /* Name of the task */
			8000,                 /* Stack size in words */
			NULL,                 /* Task input parameter */
			5,                    /* Priority of the task */
			&weatherTaskHandle,   /* Task handle. */
			0);                   /* Core where the task should run */

	if (weatherTaskHandle == NULL) {
		return false;
	}
	weatherTicker.attach(1800, triggerGetUGWeather);
	return true;
}

/**
 * triggerGetWeather
 *
 * Triggered by timer to get weather and time update every 30 minutes
 */
void triggerGetUGWeather() {
	addMqttMsg(debugLabel, infoLabel + digitalTimeDisplaySec() + " Weather update triggered", false);
	xTaskResumeFromISR(weatherTaskHandle);
}

/**
 * Task to get current weather from Underground Weather
 */
void ugWeatherTask(void *pvParameters) {
	// Serial.println("weatherTask loop started");
	while (1) // weatherTask loop
	{
		if (otaRunning)
		{
			vTaskDelete(NULL);
		}
		if (tasksEnabled) {
			// Update NTP time
			if (!tryGetTime()) {
				// Serial.println("Failed to get update from NTP");
				addMqttMsg(debugLabel, errorLabel + digitalTimeDisplaySec() + " NTP error", false);
			}
			// Get weather info
			if (getUGWeather()) {
				// Serial.println("Got weather conditions");
				// Serial.println(ugWeatherText);
				addMqttMsg("WEA", ugWeatherText, true);
			} else {
				// Serial.println("Failed to get weather conditions");
				addMqttMsg(debugLabel, errorLabel + digitalTimeDisplaySec() + " Weather error", false);
				// delay(60000); // try again in 60 seconds
			}
		}
		vTaskSuspend(NULL);
	}
}

/**
 * getUGWeather
 * Get weather conditions from WunderGround API
 *
 * @return <code>bool</bool>
 *				true if data was received and parsed
 *				false if error occurs
 **/
bool getUGWeather() {
	String url = wgURL;
	url += wgApiKey;
	url += "/conditions/q/";
	url += wgCountry;
	url += "/";
	url += wgCity;
	url += ".json";

	String payload = "";

	http.setReuse(false);
	http.setTimeout(10000);
	http.begin(url);
	int httpCode = http.GET();

	// httpCode will be negative on error
	if(httpCode > 0) {
			// file found at server
			if(httpCode == HTTP_CODE_OK) {
				payload = http.getString();
				http.end();
			} else {
				// payload = "[HTTP] GET error code: " + http.errorToString(httpCode);
				// Serial.println(payload);
				// addMqttMsg(debugLabel, errorLabel + digitalTimeDisplaySec() + " getWGWeather() http.getString() -> " + payload, false);
				http.end();
				return false;
			}
	} else {
		// payload = "[HTTP] GET error code: " + http.errorToString(httpCode);
		// Serial.println(payload);
		// addMqttMsg(debugLabel, errorLabel + digitalTimeDisplaySec() + " getWGWeather() http.GET() -> " + payload, false);
		http.end();
		return false;
	}

	payload = payload.substring(1,payload.length()-1);

	/** Buffer for incoming JSON string */
	DynamicJsonBuffer jsonInBuffer;
	/** Char buffer for incoming data */
	char json[payload.length()];
	payload.toCharArray(json, payload.length() + 1);
	/** Json object for incoming data */
	JsonObject& jsonIn = jsonInBuffer.parseObject(json);
	if (!jsonIn.success()) {
		return false;
	} else {
		JsonObject& current_observation = jsonIn.get<JsonObject>("current_observation");
		String temp = String(current_observation["temp_c"].as<double>());
		String humid = String(current_observation["relative_humidity"].as<double>());
		String heat = String(current_observation["heat_index_c"].as<double>());
		String weather = current_observation["weather"];
		String wind_dir = current_observation["wind_dir"];
		String wind_kph = current_observation["wind_kph"];
		String wind_gust_kph = current_observation["wind_gust_kph"];
		long observation_epoch = current_observation["observation_epoch"];
		int obsHourVal = ((observation_epoch	% 86400L) / 3600) + 8; // GMT+8
		if (obsHourVal >= 24) {
			obsHourVal -= 24;
		}
		String obsHour = String(obsHourVal);
		String obsMin = "";
		if ( ((observation_epoch % 3600) / 60) < 10 ) {
			// In the first 10 minutes of each hour, we'll want a leading '0'
			obsMin = "0";
		}
		obsMin += String((observation_epoch	% 3600) / 60);

		tft.setTextSize(1);
		tft.setCursor(0, 120);
		tft.fillRect(0, 120, 128, 40, TFT_DARKGREY);
		tft.println("Weather at " + obsHour + ":" + obsMin);
		ugWeatherText = "T " + temp + "'C H " + humid +"%";
		tft.println(ugWeatherText);
		tft.println(weather);
		ugWeatherText = wind_dir + " wind at " + wind_kph + "kph";
		tft.println(ugWeatherText);
		ugWeatherText = "gusting to " + wind_gust_kph + "kph";
		tft.println(ugWeatherText);

		const unsigned short *icon = unknown;
		if (current_observation.containsKey("icon")) {
			String iconName =	current_observation["icon"].as<char*>();
			// Serial.printf("Showing icon for %s\n",current_observation["icon"].asString());
			for (int index = 0; index < ugIconNums; index++) {
				if (strcmp(iconName.c_str(), ugIconName[index].c_str()) == 0) {
					icon = ugIconArray[index];
					break;
				}
			}
		// } else {
		// 	Serial.println("Could not find the icon");
		// 	addMqttMsg(debugLabel, errorLabel + digitalTimeDisplaySec() + "Could not find weather icon", false);
		}
		// drawIcon(icon,	(tft.width() -	ugIconWidth)/2, 88,	ugIconWidth,	ugIconHeight);
		drawIcon(icon,	5, 88,	ugIconWidth,	ugIconHeight);
		tft.setCursor(45,103);
		tft.setTextSize(1);
		tft.print("Light:");
		ugWeatherText = "Weather at " + String(obsHour) + ":" + String(obsMin) + "</br>";
		ugWeatherText += "T " + temp + "&deg;C H " + humid +"%</br>";
		ugWeatherText += weather + "</br>";
		ugWeatherText += wind_dir + " wind at " + wind_kph + "kph</br>";
		ugWeatherText += "gusting to " + wind_gust_kph + "kph</br>";

		return true;
	}
}
