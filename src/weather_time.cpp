#include "setup.h"
#include "icons.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

bool getWGWeather();
void triggerGetWeather();
void drawIcon(const unsigned short* icon, int16_t x, int16_t y, int8_t width, int8_t height);

/** Underground Weather API key */
const String wgApiKey = "e062ec62675eb278";
/** Country for Underground Weather */
const String wgCountry = "PH";
/** City for Underground Weather */
const String wgCity = "Paranaque_City";
/** URL for Underground Weather */
const String wgURL = "http://api.wunderground.com/api/";
/** HTTPClient class to get data from Underground Weather */
HTTPClient http;
/** Task handle for the weather and time update task */
TaskHandle_t weatherTaskHandle = NULL;
/** String with current weather situation */
String weatherText = "";
/** Ticker for weather and time update */
Ticker weatherTicker;

/**
 * initWeather
 * Setup task for repeated weather and time update
 * @return bool
 *    true if task is started
 *    false if task couldn't be started
 */
bool initWeather() {
  // Start NTP listener
	initNTP();
	tryGetTime();

  // Start task to get weather & time updates
	xTaskCreatePinnedToCore(
			weatherTask,                      /* Function to implement the task */
			"weatherTask ",			              /* Name of the task */
			8000,              			          /* Stack size in words */
			NULL,                          		/* Task input parameter */
			5,                              	/* Priority of the task */
			&weatherTaskHandle,               /* Task handle. */
			0);                            		/* Core where the task should run */

  if (weatherTaskHandle == NULL) {
    return false;
  }
	weatherTicker.attach(1800, triggerGetWeather);
  return true;
}

/**
 * triggerGetWeather
 *
 * Triggered by timer to get weather and time update every 30 minutes
 */
void triggerGetWeather() {
	xTaskResumeFromISR(weatherTaskHandle);
}

/**
 * Task to read temperature from DHT11 sensor
 */
void weatherTask(void *pvParameters) {
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
      if (getWGWeather()) {
        // Serial.println("Got weather conditions");
        addMeeoMsg("weather", weatherText, false);
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
 * getWGWeather
 * Get weather conditions from WunderGround API
 *
 * @return <code>bool</bool>
 *				true if data was received and parsed
 *				false if error occurs
 **/
bool getWGWeather() {
	// /** HTTPClient class to get data */
	// HTTPClient http;
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
				payload = "[HTTP] GET error code: " + http.errorToString(httpCode);
				http.end();
				Serial.println(payload);
				// sendDebug(payload,"ESP32");
				addMeeoMsg("", "[ERROR] " + digitalTimeDisplaySec() + " getWGWeather() http.getString() -> " + payload, true);
				return false;
			}
	} else {
		payload = "[HTTP] GET error code: " + http.errorToString(httpCode);
		http.end();
		Serial.println(payload);
		// sendDebug(payload,"ESP32");
		addMeeoMsg("", "[ERROR] " + digitalTimeDisplaySec() + " getWGWeather() http.GET() -> " + payload, true);
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

		tft.setTextSize(1);
		tft.setCursor(0, 120);
		tft.fillRect(0, 120, 128, 40, TFT_DARKGREY);
		tft.println("Weather at " + obsHour + ":" + obsMin);
		String ugWeather = "T " + temp + "'C H " + humid +"%";
		tft.println(ugWeather);
		tft.println(weather);
		ugWeather = wind_dir + " wind at " + wind_kph + "kph";
		tft.println(ugWeather);
		ugWeather = "gusting to " + wind_gust_kph + "kph";
		tft.println(ugWeather);
		// Serial.println("Temperature: " + temp);
		// Serial.println("Humidity:    " + humid);
		// Serial.println("Heat index:  " + heat);
		// Serial.println(weather);
		// Serial.println("Wind from the " + wind_dir + " at " + wind_kph + "km/h up to " + wind_gust_kph + "km/h");

		const unsigned short *icon = unknown;
		if (current_observation.containsKey("icon")) {
			String iconName =  current_observation["icon"].as<char*>();
			// Serial.printf("Showing icon for %s\n",current_observation["icon"].asString());
			for (int index = 0; index < iconNums; index++) {
				if (strcmp(iconName.c_str(), iconNames[index].c_str()) == 0) {
					icon = iconArray[index];
					break;
				}
			}
		} else {
			Serial.println("Could not find the icon");
			addMeeoMsg("", "[ERROR] " + digitalTimeDisplay() + "Could not find weather icon", true);
		}
		drawIcon(icon,  (tft.width() -  infoWidth)/2, 88,  infoWidth,  infoHeight);

		weatherText = "Weather at " + String(obsHour) + ":" + String(obsMin) + "\n";
		weatherText += "T " + temp + "'C H " + humid +"%\n";
		weatherText += weather + "\n";
		weatherText += wind_dir + " wind at " + wind_kph + "kph\n";
		weatherText += "gusting to " + wind_gust_kph + "kph\n";

		return true;
	}
}

// To speed up rendering we use a 64 pixel buffer
#define BUFF_SIZE 64

/**
 * drawIcon
 * Draw array "icon" of defined width and height at coordinate x,y
 * Maximum icon size is 255x255 pixels to avoid integer overflow
 * Icon is stored as an array in program memory (FLASH)
 * @param icon
 *		pointer to icon
 * @param x
 *		x coordinate where icon should be drawn
 * @param y
 *		y coordinate where icon should be drawn
 * @param width
 *		width of icon
 * @param height
 *		height of icon
 */
void drawIcon(const unsigned short* icon, int16_t x, int16_t y, int8_t width, int8_t height) {

  uint16_t  pix_buffer[BUFF_SIZE];   // Pixel buffer (16 bits per pixel)

  // Set up a window the right size to stream pixels into
  tft.setAddrWindow(x, y, x + width - 1, y + height - 1);

  // Work out the number whole buffers to send
  uint16_t nb = ((uint16_t)height * width) / BUFF_SIZE;

  // Fill and send "nb" buffers to TFT
  for (int i = 0; i < nb; i++) {
    for (int j = 0; j < BUFF_SIZE; j++) {
      pix_buffer[j] = pgm_read_word(&icon[i * BUFF_SIZE + j]);
    }
    tft.pushColors(pix_buffer, BUFF_SIZE);
  }

  // Work out number of pixels not yet sent
  uint16_t np = ((uint16_t)height * width) % BUFF_SIZE;

  // Send any partial buffer left over
  if (np) {
    for (int i = 0; i < np; i++) pix_buffer[i] = pgm_read_word(&icon[nb * BUFF_SIZE + i]);
    tft.pushColors(pix_buffer, np);
  }
}
