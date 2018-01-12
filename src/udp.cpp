#include "setup.h"

/** Network address mask for TCP debug */
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

			tft.fillRect(0, 69, 128, 14, TFT_BLACK);
			tft.setTextSize(2);
			tft.setTextColor(TFT_WHITE);
			tft.setCursor(0,69);
			String displayText = "E " + String(outsideTemp,0) + "'C " + String(outsideHumid,0) + "%";
			tft.print(displayText);
		}
	} else {
		addMqttMsg("debug", "[ERROR] " + digitalTimeDisplaySec() + "Received invalid JSON", false);
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
	WiFiUDP udpClientServer;

	// Start UDP client for sending packets
	int connOK = udpClientServer.begin(udpPort);

	if (connOK == 0) {
		Serial.println("UDP could not get socket");
		addMqttMsg("debug", "[ERROR] " + digitalTimeDisplaySec() + "UDP could not get socket", false);
		return false;
	}
	int beginOK = udpClientServer.beginPacket(ipAddr, udpPort);

	if (beginOK == 0) { // Problem occured!
		udpClientServer.stop();
		Serial.println("UDP connection failed");
		addMqttMsg("debug", "[ERROR] " + digitalTimeDisplaySec() + "UDP connection failed", false);
		return false;
	}
	int bytesSent = udpClientServer.print(udpMsg);
	if (bytesSent == udpMsg.length()) {
		udpClientServer.endPacket();
		udpClientServer.stop();
		return true;
	} else {
		Serial.println("Failed to send " + udpMsg + ", sent " + String(bytesSent) + " of " + String(udpMsg.length()) + " bytes");
		addMqttMsg("debug", "[ERROR] " + digitalTimeDisplaySec() + "Failed to send " + udpMsg + ", sent " + String(bytesSent) + " of " + String(udpMsg.length()) + " bytes", false);
		udpClientServer.endPacket();
		udpClientServer.stop();
		return false;
	}
}

/**
 * sendDebug
 * Send a debug message over TCP
 *
 * @param debugMsg
 *				debug message
 * @param senderID
 *				device ID
 **/
void sendDebug(String debugMsg, String senderID) {
	/** WiFiClient class to create TCP communication */
	WiFiClient tcpDebugClient;

	if (!tcpDebugClient.connect(ipDebug, tcpDebugPort)) {
		Serial.println("connection to Debug Android " + String(ipDebug[0]) + "." + String(ipDebug[1]) + "." + String(ipDebug[2]) + "." + String(ipDebug[3]) + " failed");
		tcpDebugClient.stop();
		return;
	}

	debugMsg = senderID + " " + debugMsg;
	tcpDebugClient.print(debugMsg);

	tcpDebugClient.stop();
}
