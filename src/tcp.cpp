#include "setup.h"

/**
 * getTCPPacket
 * Get TCP packet
 * @param tcpClient
 *		WiFiClient of sending client
 */
void getTCPPacket(WiFiClient tcpClient) {
	uint32_t timeoutStart = millis();
	char recvdBuffer[128];
	byte index = 0;
	char inByte;
	while (tcpClient.connected()) {
		if (tcpClient.available()) {
			inByte = tcpClient.read();
			recvdBuffer[index] = inByte;
			index++;
		}
		if (millis() > timeoutStart + 3000) { // Wait a maximum of 3 seconds
			break; // End the while loop because of timeout
		}
	}
	recvdBuffer[index] = 0;
	tcpClient.flush();
	tcpClient.stop();
 
	if (index != 0) { // Any data received?
		// Copy received buffer into a string for easier handling
		String payload(recvdBuffer);
		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " Received TCP packet:" + payload, false);
		// parse the incoming data
		// toggle debugging
		if (payload.substring(0,1) == "d") {
			// toggle debug flag
			debugOn = !debugOn;
			if (debugOn) {
				Serial.println("Debug on");
			} else {
				Serial.println("Debug off");
			}
			return;
		// Reset device
		} else if (payload.substring(0, 1) == "x") {
			Serial.println("Reset");
			tcpClient.flush();
			tcpClient.stop();
			// Reset the ESP
			delay(3000);
			esp_restart();
		// Print system info
		} else if (payload.substring(0, 1) == "i") {
			printLastResetReason();
			printPartitions();
			sendDebug(debugLabel, "\nFree Heap: " + String(esp_get_free_heap_size()) + "bytes", false);
		}
	}
}
