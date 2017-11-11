#include "setup.h"
#include "accuweather.h"

bool getAccuWeather();
void triggerGetAccuWeather();

// http://dataservice.accuweather.com/currentconditions/v1/{locationKey}
// http://dataservice.accuweather.com/currentconditions/v1/6OkNw6wWuLAba7TFdUlhNKkzo8t6g5GT
//
// http://api.accuweather.com/locations/v1/search?q=san&apikey={6OkNw6wWuLAba7TFdUlhNKkzo8t6g5GT}
// http://dataservice.accuweather.com/locations/v1/countries/{ASI}&apikey={6OkNw6wWuLAba7TFdUlhNKkzo8t6g5GT}
//
// Location key:      2-264880_1_AL

/** Accu Weather API key */
const String accuApiKey = "6OkNw6wWuLAba7TFdUlhNKkzo8t6g5GT";
/** City for Accu Weather */
const String accuCity = "2-264880_1_AL";
/** URL for Accu Weather */
const String accuURL = "http://dataservice.accuweather.com/";
/** Current weather as readable string */
String accuWeatherText = "";

/**
 * initWeather
 * Setup task for repeated weather and time update
 * @return bool
 *    true if task is started
 *    false if task couldn't be started
 */
bool initAccuWeather() {
  // Start NTP listener
	initNTP();
	tryGetTime();

  // Start task to get weather & time updates
	xTaskCreatePinnedToCore(
			accuWeatherTask,                  /* Function to implement the task */
			"accuWeatherTask ",			          /* Name of the task */
			8000,              			          /* Stack size in words */
			NULL,                          		/* Task input parameter */
			5,                              	/* Priority of the task */
			&weatherTaskHandle,               /* Task handle. */
			0);                            		/* Core where the task should run */

  if (weatherTaskHandle == NULL) {
    return false;
  }
	weatherTicker.attach(1800, triggerGetAccuWeather);
  return true;
}

/**
 * triggerGetWeather
 *
 * Triggered by timer to get weather and time update every 30 minutes
 */
void triggerGetAccuWeather() {
	xTaskResumeFromISR(weatherTaskHandle);
}

/**
 * Task to read temperature from DHT11 sensor
 */
void accuWeatherTask(void *pvParameters) {
	Serial.println("weatherTask loop started");
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
        addMeeoMsg("", "[ERROR] " + digitalTimeDisplay() + " Failed to get update from NTP", true);
      }
      // Get weather info
      if (getAccuWeather()) {
        // Serial.println("Got weather conditions");
        addMeeoMsg("weather", accuWeatherText, false);
      } else {
        // Serial.println("Failed to get weather conditions");
        addMeeoMsg("", "[ERROR] " + digitalTimeDisplaySec() + " Failed to get weather conditions", true);
        // delay(60000); // try again in 60 seconds
      }
		}
    vTaskSuspend(NULL);
	}
}

/**
 * getAccuWeather
 * Get weather conditions from Accu Weather API
 *
 * @return <code>bool</bool>
 *				true if data was received and parsed
 *				false if error occurs
 **/
bool getAccuWeather() {
	String url = accuURL;
  url += "currentconditions/v1/";
  url += accuCity;
  url += "?apikey=";
  url += accuApiKey;
  url += "&details=true";

	Serial.println("Accu Weather " + url);

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
				payload = "[HTTP] GET error code: " + http.errorToString(httpCode);
				http.end();
				Serial.println(payload);
				// sendDebug(payload,"ESP32");
				addMeeoMsg("", "[ERROR] " + digitalTimeDisplaySec() + " getAccuWeather() http.getString() -> " + payload, true);
				return false;
			}
	} else {
		payload = "[HTTP] GET error code: " + http.errorToString(httpCode);
		http.end();
		Serial.println(payload);
		// sendDebug(payload,"ESP32");
		addMeeoMsg("", "[ERROR] " + digitalTimeDisplaySec() + " getAccuWeather() http.GET() -> " + payload, true);
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
		// sendDebug(String(json),"ESP32");
    addMeeoMsg("", "[ERROR] " + digitalTimeDisplaySec() + " Failed to parse AccuWeather", true);
		return false;
	} else {
    long observation_epoch = jsonIn["EpochTime"];
		int obsHourVal = ((observation_epoch  % 86400L) / 3600) + 8; // GMT+8
		if (obsHourVal >= 24) {
			obsHourVal -= 24;
		}
		String obsHour = String(obsHourVal);
		String obsMin = "";
		if ( ((observation_epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      obsMin = "0";
    }
		obsMin += String((observation_epoch  % 3600) / 60);
    String weather = jsonIn["WeatherText"];
    JsonObject& currentTemperature = jsonIn.get<JsonObject>("Temperature");
    JsonObject& currentTempMetric = currentTemperature.get<JsonObject>("Metric");
    String temp = String(currentTempMetric["Value"].as<double>());
    JsonObject& currentFeel = jsonIn.get<JsonObject>("RealFeelTemperature");
    JsonObject& currentFeelMetric = currentFeel.get<JsonObject>("Metric");
    String heat = String(currentFeelMetric["Value"].as<double>());
    String humid = String(jsonIn["RelativeHumidity"].as<double>());
    JsonObject& currentWind = jsonIn.get<JsonObject>("Wind");
    JsonObject& currentWindDir = currentWind.get<JsonObject>("Direction");
    String wind_dir = currentWindDir["English"];
    JsonObject& currentWindSpeed = currentWind.get<JsonObject>("Speed");
    JsonObject& currentWindSpeedMetric = currentWindSpeed.get<JsonObject>("Metric");
    String wind_kph = currentWindSpeedMetric["Value"];
    JsonObject& currentWindGust = jsonIn.get<JsonObject>("WindGust");
    JsonObject& currentWindGustSpeed = currentWindGust.get<JsonObject>("Speed");
    JsonObject& currentWindGustMetric = currentWindGustSpeed.get<JsonObject>("Metric");
    String wind_gust_kph = currentWindGustMetric["Value"];

		tft.setTextSize(1);
		tft.setCursor(0, 120);
		tft.fillRect(0, 120, 128, 40, TFT_DARKGREY);
		tft.println("Weather at " + obsHour + ":" + obsMin);
		accuWeatherText = "T " + temp + "'C H " + humid +"%";
		tft.println(accuWeatherText);
		tft.println(weather);
		accuWeatherText = wind_dir + " wind at " + wind_kph + "kph";
		tft.println(accuWeatherText);
		accuWeatherText = "gusting to " + wind_gust_kph + "kph";
		tft.println(accuWeatherText);

		const unsigned short *icon = unknown;
		if (jsonIn.containsKey("WeatherIcon")) {
			int iconNum =  jsonIn["WeatherIcon"].as<int>();
      icon = accuIconArray[iconNum];
		} else {
			Serial.println("Could not find the icon");
			addMeeoMsg("", "[ERROR] " + digitalTimeDisplay() + " Could not find weather icon", true);
		}
		// drawIcon(icon,  (tft.width() -  accuIconWidth), 75,  accuIconWidth,  accuIconHeight);
    // drawIcon(icon,  (tft.width() -  accuIconWidth)/2, 88,  accuIconWidth,  accuIconHeight);
		drawIcon(icon,  5, 88,  accuIconWidth,  accuIconHeight);
		tft.setCursor(45,103);
		tft.setTextSize(1);
		tft.print("Light:");

		accuWeatherText = "Weather at " + String(obsHour) + ":" + String(obsMin) + "\n";
		accuWeatherText += "T " + temp + "'C H " + humid +"%\n";
		accuWeatherText += weather + "\n";
		accuWeatherText += wind_dir + " wind at " + wind_kph + "kph\n";
		accuWeatherText += "gusting to " + wind_gust_kph + "kph\n";

		return true;
	}
}
