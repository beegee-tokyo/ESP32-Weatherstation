#include "setup.h"

/** Network address for TCP debug */
static IPAddress ipDebug (192, 168, 0, 10);
/** TCP debug port */
static const int tcpDebugPort = 9999;

/**
 * getUDPbroadcast
 * Get UDP broadcast message
 * @param udpMsgLength
 *		size of buffered UDP message
 */
void getUDPbroadcast(int udpMsgLength) {
	/** Outside temperature value */
	double outsideTemp = 0;
	/** Outside humidity value */
	double outsideHumid = 0;
	/** Outside heat index value */
	double outsideHeat = 0;
	/** Buffer for udpPacket */
	byte udpPacket[udpMsgLength+1];

	udpListener.read(udpPacket, udpMsgLength);
	udpPacket[udpMsgLength] = 0;

	udpListener.flush(); // empty UDP buffer for next packet

	/** Buffer for incoming JSON string */
	DynamicJsonBuffer jsonInBuffer;
	/** Json object for incoming data */
	JsonObject& jsonIn = jsonInBuffer.parseObject((char *)udpPacket);
	if (jsonIn.success() && jsonIn.containsKey("de")) {
		String thisPayLoad;
		if (jsonIn["de"] == "sb1") {
			if (jsonIn.containsKey("te")) {
				thisPayLoad = String(jsonIn["te"].as<double>())+"C";
				outsideTemp = jsonIn["te"].as<double>();
			}
			if (jsonIn.containsKey("hu")) {
				thisPayLoad = String(jsonIn["hu"].as<double>());
				outsideHumid = jsonIn["hu"].as<double>();
			}
			if (jsonIn.containsKey("he")) {
				thisPayLoad = String(jsonIn["he"].as<double>());
				outsideHeat = jsonIn["he"].as<double>();
			}

			portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
			portENTER_CRITICAL(&mux);
			tft.fillRect(0, 69, 128, 14, TFT_BLACK);
			tft.setTextSize(2);
			tft.setTextColor(TFT_WHITE);
			tft.setCursor(0,69);
			String displayText = "E " + String(outsideTemp,0) + "'C " + String(outsideHumid,0) + "%";
			tft.print(displayText);
			portEXIT_CRITICAL(&mux);
		}
	} else {
		sendDebug(debugLabel, errorLabel + digitalTimeDisplaySec() + "Received invalid JSON", false);
	}
}

/**
 * udpSendMessage
 * Send a message over UDP
 *
 * @param ipAddr
 *				receiver IP address
 * @param udpMsg
 *				payload to be send
 * @param udpPort
 *				port to be used to send
 * @return <code>bool</bool>
 *				true if payload could be sent
 *				false if error occurs
 */
bool udpSendMessage(IPAddress ipAddr, String udpMsg, int udpPort) {
	/** WiFiUDP class for creating UDP communication */
	WiFiUDP udpServer;

	// Start UDP client for sending packets
	int connOK = udpServer.begin(udpPort);

	if (connOK == 0) {
		// sendDebug(debugLabel, errorLabel + digitalTimeDisplaySec() + "UDP could not get socket", false);
		return false;
	}
	int beginOK = udpServer.beginPacket(ipAddr, udpPort);

	if (beginOK == 0) { // Problem occured!
		udpServer.stop();
		// sendDebug(debugLabel, errorLabel + digitalTimeDisplaySec() + "UDP connection failed", false);
		return false;
	}
	int bytesSent = udpServer.print(udpMsg);
	if (bytesSent == udpMsg.length()) {
		udpServer.endPacket();
		udpServer.stop();
		return true;
	} else {
		// sendDebug(debugLabel, errorLabel + digitalTimeDisplaySec() + "Failed to send " + udpMsg + ", sent " + String(bytesSent) + " of " + String(udpMsg.length()) + " bytes", false);
		udpServer.endPacket();
		udpServer.stop();
		return false;
	}
}