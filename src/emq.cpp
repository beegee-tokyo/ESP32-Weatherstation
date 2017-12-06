#include "setup.h"

/** MQTT broker URL */
static const char * mqttBroker = "93.104.213.79";
/** MQTT connection id */
static const char * mqttID = "ESP32";
/** MQTT user name */
static const char * mqttUser = "esp32";
/** MQTT password */
static const char * mqttPwd = "teresa1963";

/** WiFi client class to receive messages from mqtt broker */
WiFiClient mqttReceiver;
/** Label for consumption/production chart */
int solarLabel = 1;

void mqttTask(void *pvParameters);
void messageReceived(String &topic, String &payload);
void sendDeviceInfo();

/** Structure for MQTT message Buffer */
struct mqttMsgStruct {
	String topic = "";
	String payload = "";
	bool waiting = false;
	bool retained = false;
};

/** Queued messages for MQTT */
mqttMsgStruct mqttMsg[10];
/** Task handle for the MQTT publisher task */
TaskHandle_t mqttTaskHandle = NULL;

/**
 * initMqtt
 * Initialize Meeo connection
 */
void initMqtt() {
	// Connect to MQTT broker
	if (!connectMQTT()) {
		Serial.println("Can't connect to MQTT broker");
		return;
	}

  // Start task for MEEO publishing
	xTaskCreatePinnedToCore(
	    mqttTask,                       /* Function to implement the task */
	    "MQTTPublisher ",               /* Name of the task */
	    4000,                           /* Stack size in words */
	    NULL,                           /* Task input parameter */
	    5,                             	/* Priority of the task */
	    &mqttTaskHandle,                /* Task handle. */
	    1);                             /* Core where the task should run */

	// // Send deviceInfo for Crouton
	// sendDeviceInfo();
	mqttClient.publish((char *)"/DEV/MONITOR",(char *)"moni",4,true,0);
}

/**
 * addMqttMsg
 * Adds a message to the buffer to be processed by meeoTask()
 *
 * @param topic
 *     String with the topic
 * @param payload
 *    String with the payload
 * @return bool
 *    true if message is added to the buffer
 *    false if buffer was full
 */
bool addMqttMsg (String topic, String payload, bool retained) {
	for (byte index = 0; index < 10; index ++) {
		if (!mqttMsg[index].waiting) { // found an empty slot?
			mqttMsg[index].topic = topic;
			mqttMsg[index].payload = payload;
			mqttMsg[index].waiting = true;
			mqttMsg[index].retained = retained;
			vTaskResume(mqttTaskHandle);
			return true;
		}
	}
	vTaskResume(mqttTaskHandle);
	return false;
}

/**
 * Task to send data from meeoMsg buffer to Meeo.IO
 * @param pvParameters
 *    pointer to task parameters
 */
void mqttTask(void *pvParameters) {
	Serial.println("mqttTask loop started");
	while (1) // meeoTask loop
  {
    if (otaRunning) {
			vTaskDelete(NULL);
		}
		for (byte index = 0; index < 10; index ++) {
			if (mqttMsg[index].waiting) {
				if (mqttClient.publish("/"+mqttMsg[index].topic,mqttMsg[index].payload,mqttMsg[index].retained,0)) {
					mqttMsg[index].waiting = false;
				} else { // Publishing error. Maybe we lost connection ???
					Serial.println("Sending to MQTT broker failed");
					if (connectMQTT()) { // Try to reconnect and resend the message
						sendDeviceInfo();
						if (mqttClient.publish("/"+mqttMsg[index].topic,mqttMsg[index].payload)) {
							mqttMsg[index].waiting = false;
						}
					}
				}
			}
		}
    bool queueIsEmpty = true;
    for (byte index = 0; index < 10; index ++) {
      if (mqttMsg[index].waiting) {
        queueIsEmpty = false;
      }
    }
    if (queueIsEmpty) {
      vTaskSuspend(NULL);
    }
	}
}

/**
 * Connect to MQTT broker
 *
 * @return <code>bool</code>
 *			true if connection was successful
 *			false if connection failed
 **/
bool connectMQTT() {
  // Connect to MQTT broker
  mqttClient.begin(mqttBroker, mqttReceiver);
  mqttClient.onMessage(messageReceived);

  Serial.println("Connecting to MQTT broker");

  int connectTimeout = 0;

  while (!mqttClient.connect(mqttID, mqttUser, mqttPwd)) {
    delay(100);
    connectTimeout++;
    if (connectTimeout > 10) { // Wait for 1 seconds to connect
      Serial.println("Can't connect to MQTT broker");
      return false;
    }
  }
  Serial.println("Connected to MQTT");

  // Set MQTT last will
  mqttClient.setWill("/DEV/MONI", "Dead");

	// //Subscribe to outbox/weather/deviceInfo
	// mqttClient.subscribe("/inbox/weather/deviceInfo");

  return true;
}

/**
 * messageReceived
 * Called when subscribed message was received from MQTT broker
 *
 * @param topic
 *      topic of received message
 * @param payload
 *      payload of message
*/
void messageReceived(String &topic, String &payload) {
	// if (topic == "/inbox/weather/deviceInfo") {
	// 	sendDeviceInfo();
	// }
  Serial.println("MQTT - received from topic: " + topic +  " payload: " + payload);
}

/**
 * sendDeviceInfo
 * creates deviceInfo as Json
 */
void sendDeviceInfo() {
	// const size_t bufferSize = 3*JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + 3*JSON_OBJECT_SIZE(4);
	// DynamicJsonBuffer jsonBuffer(bufferSize);
  //
	// JsonObject& root = jsonBuffer.createObject();
  //
	// JsonObject& deviceInfo = root.createNestedObject("deviceInfo");
	// deviceInfo["name"] = "weather";
  //
	// deviceInfo["description"] = "MHC weather display";
	// deviceInfo["status"] = "good";
  //
	// JsonObject& deviceInfo_endPoints = deviceInfo.createNestedObject("endPoints");
  //
	// JsonObject& deviceInfo_endPoints_temp_in = deviceInfo_endPoints.createNestedObject("temp_in");
	// deviceInfo_endPoints_temp_in["title"] = "Temperature inside (C)";
	// deviceInfo_endPoints_temp_in["card-type"] = "crouton-simple-text";
	// deviceInfo_endPoints_temp_in["units"] = "";
	// JsonObject& deviceInfo_endPoints_temp_in_values = deviceInfo_endPoints_temp_in.createNestedObject("values");
	// deviceInfo_endPoints_temp_in_values["value"] = 0;
  //
	// JsonObject& deviceInfo_endPoints_humid_in = deviceInfo_endPoints.createNestedObject("humid_in");
	// deviceInfo_endPoints_humid_in["title"] = "Humidity inside";
	// deviceInfo_endPoints_humid_in["card-type"] = "crouton-simple-text";
	// deviceInfo_endPoints_humid_in["units"] = "";
	// JsonObject& deviceInfo_endPoints_humid_in_values = deviceInfo_endPoints_humid_in.createNestedObject("values");
	// deviceInfo_endPoints_humid_in_values["value"] = 0;
  //
	// JsonObject& deviceInfo_endPoints_heat_in = deviceInfo_endPoints.createNestedObject("heat_in");
	// deviceInfo_endPoints_heat_in["title"] = "Heat index inside (C)";
	// deviceInfo_endPoints_heat_in["card-type"] = "crouton-simple-text";
	// deviceInfo_endPoints_heat_in["units"] = "";
	// JsonObject& deviceInfo_endPoints_heat_in_values = deviceInfo_endPoints_heat_in.createNestedObject("values");
	// deviceInfo_endPoints_heat_in_values["value"] = 0;
  //
	// JsonObject& deviceInfo_endPoints_temp_out = deviceInfo_endPoints.createNestedObject("temp_out");
	// deviceInfo_endPoints_temp_out["title"] = "Temperature outside (C)";
	// deviceInfo_endPoints_temp_out["card-type"] = "crouton-simple-text";
	// deviceInfo_endPoints_temp_out["units"] = "";
	// JsonObject& deviceInfo_endPoints_temp_out_values = deviceInfo_endPoints_temp_out.createNestedObject("values");
	// deviceInfo_endPoints_temp_out_values["value"] = 0;
  //
	// JsonObject& deviceInfo_endPoints_humid_out = deviceInfo_endPoints.createNestedObject("humid_out");
	// deviceInfo_endPoints_humid_out["title"] = "Humidity outside";
	// deviceInfo_endPoints_humid_out["card-type"] = "crouton-simple-text";
	// deviceInfo_endPoints_humid_out["units"] = "";
	// JsonObject& deviceInfo_endPoints_humid_out_values = deviceInfo_endPoints_humid_out.createNestedObject("values");
	// deviceInfo_endPoints_humid_out_values["value"] = 0;
  //
	// JsonObject& deviceInfo_endPoints_heat_out = deviceInfo_endPoints.createNestedObject("heat_out");
	// deviceInfo_endPoints_heat_out["title"] = "Heat index outside (C)";
	// deviceInfo_endPoints_heat_out["card-type"] = "crouton-simple-text";
	// deviceInfo_endPoints_heat_out["units"] = "";
	// JsonObject& deviceInfo_endPoints_heat_out_values = deviceInfo_endPoints_heat_out.createNestedObject("values");
	// deviceInfo_endPoints_heat_out_values["value"] = 0;
  //
	// JsonObject& deviceInfo_endPoints_power = deviceInfo_endPoints.createNestedObject("power");
	// JsonObject& deviceInfo_endPoints_power_values = deviceInfo_endPoints_power.createNestedObject("values");
	// JsonArray& deviceInfo_endPoints_power_values_labels = deviceInfo_endPoints_power_values.createNestedArray("labels");
	// deviceInfo_endPoints_power_values_labels.add(1);
	// JsonArray& deviceInfo_endPoints_power_values_series = deviceInfo_endPoints_power_values.createNestedArray("series");
	// JsonArray& deviceInfo_endPoints_power_values_series_0 = deviceInfo_endPoints_power_values_series.createNestedArray();
	// deviceInfo_endPoints_power_values_series_0.add(100);
	// JsonArray& deviceInfo_endPoints_power_values_series_1 = deviceInfo_endPoints_power_values_series.createNestedArray();
	// deviceInfo_endPoints_power_values_series_1.add(50);
	// deviceInfo_endPoints_power_values["update"] = 0;
	// deviceInfo_endPoints_power["max"] = 30;
	// deviceInfo_endPoints_power["low"] = -400;
	// deviceInfo_endPoints_power["high"] = 1200;
	// deviceInfo_endPoints_power["card-type"] = "crouton-chart-line";
	// deviceInfo_endPoints_power["title"] = "Consumption/Production (kW)";
  //
	// JsonObject& deviceInfo_endPoints_weather = deviceInfo_endPoints.createNestedObject("gweather");
	// deviceInfo_endPoints_weather["title"] = "Weather now";
	// deviceInfo_endPoints_weather["card-type"] = "crouton-simple-text";
	// deviceInfo_endPoints_weather["units"] = "";
	// JsonObject& deviceInfo_endPoints_weather_values = deviceInfo_endPoints_weather.createNestedObject("values");
	// deviceInfo_endPoints_weather_values["value"] = 0;
  //
	// String deviceTopic = "outbox/weather/deviceInfo";
	// String deviceInfoStr;
	// root.printTo(deviceInfoStr);
  //
	// // Push device identification
	// // addMqttMsg(deviceTopic, deviceInfoStr);
	// mqttClient.publish("/"+deviceTopic,deviceInfoStr,true,0);
  //
	// root.prettyPrintTo(Serial); Serial.println("");
}

/**
 * updatePower
 * send consumption and production values
 *
 * @param cons
 *		current consumption as double float value
 * @param solar
 *		current solar production as double float value
 */
void updatePower(double cons, double solar) {
	// const size_t bufferSize = 3*JSON_ARRAY_SIZE(1) + JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2);
	// DynamicJsonBuffer jsonBuffer(bufferSize);
  //
	// JsonObject& root = jsonBuffer.createObject();
  //
	// JsonObject& update = root.createNestedObject("update");
	// JsonArray& update_labels = update.createNestedArray("labels");
	// solarLabel++;
	// update_labels.add(solarLabel);
  //
	// JsonArray& update_series = update.createNestedArray("series");
	// JsonArray& update_series_0 = update_series.createNestedArray();
	// update_series_0.add((int)cons);
	// JsonArray& update_series_1 = update_series.createNestedArray();
	// update_series_1.add((int)solar);
  //
	// String deviceTopic = "outbox/weather/power";
	// String deviceInfoStr;
	// root.printTo(deviceInfoStr);
  //
	// // Push device identification
	// addMqttMsg(deviceTopic, deviceInfoStr);
}
