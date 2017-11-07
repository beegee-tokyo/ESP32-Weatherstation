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

	// /** UDP sender address */
	// IPAddress udpIP;
	// String debugMsg = "UDP broadcast from ";
	// udpIP = udpListener.remoteIP();
	// debugMsg += "Sender IP: " + String(udpIP[0]) + "." + String(udpIP[1]) + "." + String(udpIP[2]) + "." + String(udpIP[3]);
	// Serial.println(debugMsg);

	udpListener.flush(); // empty UDP buffer for next packet

	/** Buffer for incoming JSON string */
	DynamicJsonBuffer jsonInBuffer;
	/** Json object for incoming data */
	JsonObject& jsonIn = jsonInBuffer.parseObject((char *)udpPacket);
	if (jsonIn.success() && jsonIn.containsKey("de")) {
		String thisPayLoad;
		if (jsonIn["de"] == "spm") {
			if (jsonIn.containsKey("c") && meeoConnected) {
				thisPayLoad = String(jsonIn["c"].as<double>());
				// Serial.print("Consumption: "); Serial.println(thisPayLoad);
				addMeeoMsg("cons", thisPayLoad, false);
				addMeeoMsg("cons_chart", thisPayLoad, false);
			}
			if (jsonIn.containsKey("s") && meeoConnected) {
				thisPayLoad = String(jsonIn["s"].as<double>());
				// Serial.print("Solar production: "); Serial.println(thisPayLoad);
				addMeeoMsg("solar", thisPayLoad, false);
				addMeeoMsg("solar_chart", thisPayLoad, false);
			}
		}
		if (jsonIn["de"] == "ly1") {
			if (jsonIn.containsKey("lo") && meeoConnected) {
				byte lightIsOn = jsonIn["lo"];
				// Serial.print("Light is: "); Serial.println(lightIsOn);
				if (lightIsOn == 1) {
					addMeeoMsg("backyard-light", (char *) "1", false);
				} else {
					addMeeoMsg("backyard-light", (char *) "0", false);
				}
			}
		}
		if (jsonIn["de"] == "sb1") {
			if (jsonIn.containsKey("te") && meeoConnected) {
				thisPayLoad = String(jsonIn["te"].as<double>())+"C";
				outsideTemp = jsonIn["te"].as<double>();
				// Serial.print("Temperature outside: "); Serial.println(thisPayLoad);
				addMeeoMsg("temp2", thisPayLoad, false);
			}
			if (jsonIn.containsKey("hu") && meeoConnected) {
				thisPayLoad = String(jsonIn["hu"].as<double>());
				outsideHumid = jsonIn["hu"].as<double>();
				// Serial.print("Humidity outside: "); Serial.println(thisPayLoad);
				addMeeoMsg("humid2", thisPayLoad, false);
			}
			if (jsonIn.containsKey("he") && meeoConnected) {
				thisPayLoad = String(jsonIn["he"].as<double>());
				outsideHeat = jsonIn["he"].as<double>();
				// Serial.print("Heat index outside: "); Serial.println(thisPayLoad);
				addMeeoMsg("heat2", thisPayLoad, false);
			}

      tft.fillRect(0, 69, 128, 14, TFT_BLACK);
      tft.setTextSize(2);
			tft.setTextColor(TFT_WHITE);
      tft.setCursor(0,69);
      String displayText = "E " + String(outsideTemp,0) + "'C " + String(outsideHumid,0) + "%";
      tft.print(displayText);
		}
	} else {
		addMeeoMsg("", "[ERROR] " + digitalTimeDisplay() + "Received invalid JSON", true);
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
		addMeeoMsg("", "[ERROR] " + digitalTimeDisplay() + "UDP could not get socket", true);
		return false;
	}
	udpClientServer.begin(udpPort);
	int beginOK = udpClientServer.beginPacket(ipAddr, udpPort);

	if (beginOK == 0) { // Problem occured!
		udpClientServer.stop();
		Serial.println("UDP connection failed");
		addMeeoMsg("", "[ERROR] " + digitalTimeDisplay() + "UDP connection failed", true);
		return false;
	}
	int bytesSent = udpClientServer.print(udpMsg);
	if (bytesSent == udpMsg.length()) {
		// Serial.println("Sent " + String(bytesSent) + " bytes from " + udpMsg + " which had a length of " + String(udpMsg.length()) + " bytes");
		udpClientServer.endPacket();
		udpClientServer.stop();
		return true;
	} else {
		Serial.println("Failed to send " + udpMsg + ", sent " + String(bytesSent) + " of " + String(udpMsg.length()) + " bytes");
		addMeeoMsg("", "[ERROR] " + digitalTimeDisplay() + "Failed to send " + udpMsg + ", sent " + String(bytesSent) + " of " + String(udpMsg.length()) + " bytes", true);
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
