#include "setup.h"


/** Structure for meeo message Buffer */
struct meeoMsgStruct {
	String topic = "";
	String payload = "";
	bool waiting = false;
	bool debug = false;
};

/** WiFi client to access MEEO */
WiFiClient meeoClient;
/** Meeo user name */
String meeoName = "bg-t16bi7m3";
/** Meeo key */
String meeoKey = "user_KINU39Bj916gvnU7";
/** Flag for connection to Meeo */
bool meeoConnected = false;
/** Queued messages for MEEO */
meeoMsgStruct meeoMsg[10];
/** Task handle for the MEEO publisher task */
TaskHandle_t meeoTaskHandle = NULL;

void meeoTask(void *pvParameters);
void meeoDataHandler(String topic, String payload);
void meeoEventHandler(MeeoEventType event);

/**
	initMeeo
	Initialize Meeo connection
*/
void initMeeo() {
  // Start task for MEEO publishing
	xTaskCreatePinnedToCore(
	    meeoTask,                       /* Function to implement the task */
	    "MeeoPublisher ",               /* Name of the task */
	    4000,                           /* Stack size in words */
	    NULL,                           /* Task input parameter */
	    5,                             	/* Priority of the task */
	    &meeoTaskHandle,                /* Task handle. */
	    1);                             /* Core where the task should run */

	// Connect to MEEO
	Meeo.setEventHandler(meeoEventHandler);
	Meeo.setDataReceivedHandler(meeoDataHandler);
	Meeo.begin(meeoName, meeoKey, meeoClient);
	Meeo.setLoggerChannel("debug");
	Meeo.run();
}

/**
	addMeeoMsg
	Adds a message to the buffer to be processed by meeoTask()

  @param topic
      String with the topic
  @param payload
      String with the payload
  @param debug
      Flag if message should be sent to default logger channel
  @return bool
      true if message is added to the buffer
      false if buffer was full
*/
bool addMeeoMsg (String topic, String payload, bool debug) {
	for (byte index = 0; index < 10; index ++) {
		if (!meeoMsg[index].waiting) { // found an empty slot?
			meeoMsg[index].topic = topic;
			meeoMsg[index].payload = payload;
			meeoMsg[index].waiting = true;
			meeoMsg[index].debug = debug;
			vTaskResume(meeoTaskHandle);
			return true;
		}
	}
	vTaskResume(meeoTaskHandle);
	return false;
}

void meeoTask(void *pvParameters) {
	Serial.println("meeoTask loop started");
	while (1) // meeoTask loop
  {
    if (otaRunning)
		{
			vTaskDelete(NULL);
		}
		for (byte index = 0; index < 10; index ++) {
			if (meeoMsg[index].waiting) {
				bool sendSuccess = false;
				if (meeoMsg[index].debug) {
					sendSuccess = Meeo.println(meeoMsg[index].payload);
				} else {
					sendSuccess = sendSuccess = Meeo.publish(meeoMsg[index].topic, meeoMsg[index].payload);
				}
				if (sendSuccess) {
					meeoMsg[index].waiting = false;
				}
			}
		}
		vTaskSuspend(NULL);
	}
}

/**
	meeoDataHandler
	Called when data from MEEO was received

  @param topic
      String with the received topic
  @param payload
      String with the received payload
*/
void meeoDataHandler(String topic, String payload) {
	String debugMsg = "MEEO incoming: " + topic + " message " + payload;
	Serial.println(debugMsg);
	if (Meeo.isChannelMatched(topic, "getweather")) {
		if (payload == "1") {
			addMeeoMsg("getweather", "0", false);
      // TODO resume task to get local weather situation
			// vTaskResume(weatherTaskHandle);
		}
	}
}

/**
	meeoEventHandler
	Called on WiFi or MQTT events
*/
void meeoEventHandler(MeeoEventType event) {
  switch (event) {
  //   case WIFI_DISCONNECTED:
  //     Serial.println("Not Connected to WiFi");
  //     break;
  //   case WIFI_CONNECTING:
  //     Serial.println("Connecting to WiFi");
  //     break;
  //   case WIFI_CONNECTED:
  //     Serial.println("Connected to WiFi");
  //     break;
    case MQ_DISCONNECTED:
      Serial.println("Lost connection to MEEO Server");
			meeoConnected = false;
      break;
    case MQ_CONNECTED:
      Serial.println("Connected to MEEO Server");
			Meeo.subscribe("getweather", 0, false);
			meeoConnected = true;
			break;
    case MQ_BAD_CREDENTIALS:
      Serial.println("MEEO Bad Credentials");
      break;
  //   case AP_MODE:
  //     Serial.println("AP Mode");
  //     break;
  //   default:
  //     break;
  }
}
