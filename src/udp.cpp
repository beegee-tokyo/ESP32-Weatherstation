#include "setup.h"

/** Network address for TCP debug */
static IPAddress ipDebug (192, 168, 254, 105);
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

			portMUX_TYPE udpMux = portMUX_INITIALIZER_UNLOCKED;
			portENTER_CRITICAL(&udpMux);
			tft.fillRect(0, 59, 128, 18, TFT_BLACK);
			tft.setTextSize(2);
			tft.setTextColor(TFT_WHITE);
			tft.setCursor(0,60);
			String displayText = "E " + String(outsideTemp,0) + "'C " + String(outsideHumid,0) + "%";
			tft.print(displayText);
			portEXIT_CRITICAL(&udpMux);
		}
		if (jsonIn["de"] == "spm") {
			int solarProduction = 0;
			int houseConsumption = 0;
			if (jsonIn.containsKey("s")) {
				solarProduction = jsonIn["s"].as<int>();
			}
			if (jsonIn.containsKey("c")) {
				houseConsumption = jsonIn["c"].as<int>();
			}
			portMUX_TYPE udpMux = portMUX_INITIALIZER_UNLOCKED;
			portENTER_CRITICAL(&udpMux);
			tft.fillRect(0, 78, 80, 10, TFT_WHITE);
			if (houseConsumption <= 0) {
				tft.fillRect(1, 79, 38, 8, TFT_GREEN);
				tft.setTextColor(TFT_BLACK);
			} else {
				tft.fillRect(1, 79, 38, 8, TFT_RED);
				tft.setTextColor(TFT_WHITE);
			}
			tft.setTextSize(1);
			tft.setCursor(6, 79);
			tft.print(String(abs(houseConsumption)));
			if (solarProduction > 0) {
				tft.fillRect(41, 79, 38, 8, TFT_YELLOW);
			} else {
				tft.fillRect(41, 79, 38, 8, TFT_LIGHTGREY);
			}
			tft.setTextColor(TFT_BLACK);
			tft.setCursor(46, 79);
			tft.print(String(abs(solarProduction)));
			portEXIT_CRITICAL(&udpMux);
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
	//- WiFiUDP udpServer;

	// Start UDP client for sending packets
	//- int connOK = udpServer.begin(udpPort);

	//- if (connOK == 0) {
		// sendDebug(debugLabel, errorLabel + digitalTimeDisplaySec() + "UDP could not get socket", false);
	//- 	return false;
	//- }
	//- int beginOK = udpServer.beginPacket(ipAddr, udpPort);
	int beginOK = udpListener.beginPacket(ipAddr, udpPort);

	if (beginOK == 0) { // Problem occured!
		//- udpServer.stop();
		// sendDebug(debugLabel, errorLabel + digitalTimeDisplaySec() + "UDP connection failed", false);
		return false;
	}
	//- int bytesSent = udpServer.print(udpMsg);
	int bytesSent = udpListener.print(udpMsg);
	if (bytesSent == udpMsg.length()) {
		//- udpServer.endPacket();
		udpListener.endPacket();
		//- udpServer.stop();
		return true;
	} else {
		// sendDebug(debugLabel, errorLabel + digitalTimeDisplaySec() + "Failed to send " + udpMsg + ", sent " + String(bytesSent) + " of " + String(udpMsg.length()) + " bytes", false);
		//- udpServer.endPacket();
		udpListener.endPacket();
		//- udpServer.stop();
		return false;
	}
}