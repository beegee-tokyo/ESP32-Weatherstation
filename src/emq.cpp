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

	// Tell MQTT broker we are alive
	mqttClient.publish((char *)"/DEV/ESP32",(char *)"esp32",5,true,0);
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
	while (1) // mqttTask loop
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
						if (mqttClient.publish("/"+mqttMsg[index].topic,mqttMsg[index].payload)) {
							mqttMsg[index].waiting = false;
							mqttClient.publish((char *)"/DEV/ESP32",(char *)"esp32",5,true,0);
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
  mqttClient.setWill("/DEV/ESP32", "Dead");

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
  Serial.println("MQTT - received from topic: " + topic +  " payload: " + payload);
}
