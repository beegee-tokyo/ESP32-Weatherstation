#include "setup.h"

BluetoothSerial SerialBT;

/** Max number of messages in buffer */
#define sppMsgBufferSize 10
/** Bluetooth Serial device name */
String btSppName = "ESP32 Weather";

void btSppTask(void *pvParameters);

/** Structure for Bluetooth Serial message Buffer */
struct btSppMsgStruct {
	String payload = "";
};

/** Queued messages for Bluetooth Serial */
btSppMsgStruct btSppMsg[sppMsgBufferSize];
/** Task handle for the Bluetooth Serial sender task */
TaskHandle_t btSppTaskHandle = NULL;

bool initBtSerial() {
	if (!SerialBT.begin(apName)) {
		sendDebug(debugLabel, errorLabel + digitalTimeDisplaySec() + " Failed to start BTSerial", false);
		return false;
	}
	// Clear message structure
	for (int index = 0; index < sppMsgBufferSize; index++) {
		btSppMsg[index].payload = "";
	}
	// Start task for MEEO publishing
	xTaskCreatePinnedToCore(
			btSppTask,             /* Function to implement the task */
			"BtSpp ",              /* Name of the task */
			2000,                  /* Stack size in words */
			NULL,                  /* Task input parameter */
			5,                     /* Priority of the task */
			&btSppTaskHandle,      /* Task handle. */
			1);                    /* Core where the task should run */

	return true;
}

void stopBtSerial() {
	// Stop Bluetooth Serial sending task
	vTaskDelete(btSppTaskHandle);
	// Stop Bluetooth Serial
	SerialBT.end();
}

bool sendBtSerial(String message) {
	bool queueResults = false;
	// Check if we have a client
	if (SerialBT.hasClient()) {
		for (byte index = 0; index < sppMsgBufferSize; index ++) {
			if (btSppMsg[index].payload.length() == 0) { // found an empty slot?
				btSppMsg[index].payload = message;
				queueResults = true;
				break;
			}
		}
		if (tasksEnabled) {
			vTaskResume(btSppTaskHandle);
		}
	}
	return queueResults;
}

/**
 * Task to send data from btSppMsg buffer over Bluetooth Serial
 * @param pvParameters
 *		pointer to task parameters
 */
void btSppTask(void *pvParameters) {
	// Serial.println("btSppTask loop started");
	while (1) // btSppTask loop
	{
		if (otaRunning) {
			vTaskDelete(NULL);
		}
		for (byte index = 0; index < sppMsgBufferSize; index ++) {
			if (btSppMsg[index].payload.length() != 0) {
				// Check if we have a client
				if (SerialBT.hasClient()) {
					// Try to send the message
					// int result = SerialBT.println(btSppMsg[index].payload);
					int result = SerialBT.write(btSppMsg[index].payload);
					if (result == -1) { 
						// Sending failed, try to reinitialize the Bluetooth Serial
						SerialBT.end();
						delay(1000);
						SerialBT.begin(btSppName);
						// Retry to send the message
						// SerialBT.println(btSppMsg[index].payload);
						int result = SerialBT.write(btSppMsg[index].payload);
					}
					SerialBT.write("\n");
					// Flush the send buffer
					SerialBT.flush();
					delay(250);
					// Discard the message
					btSppMsg[index].payload = "";
				}
			}
		}
		bool queueIsEmpty = true;
		for (byte index = 0; index < sppMsgBufferSize; index ++) {
			if (btSppMsg[index].payload.length() != 0) {
				queueIsEmpty = false;
			}
		}
		if (queueIsEmpty) {
			vTaskSuspend(NULL);
		}
		delay(500);
	}
}