#include "setup.h"

void sendDebug(String topic, String payload, bool retained) {
	// Send debug message to Serial and MQTT only if debug is enabled
	if (debugOn) {
		// Send debug message over MQTT
		addMqttMsg(topic, payload, retained);
		// Send debug message to serial
		Serial.println(payload);
	}

	// Try to send debug message over Bluetooth Serial
	sendBtSerial(payload);
}