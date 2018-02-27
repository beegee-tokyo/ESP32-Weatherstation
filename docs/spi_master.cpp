#include "setup.h"
#include <SPI.h>

#define WRITE_STATUS 0x01
#define WRITE_DATA   0x02
#define READ_DATA    0x03
#define READ_STATUS  0x04
#define CMD_FILLER   0x00

SPIClass thisSPI(HSPI);

#define VSPI_FREQUENCY 500000 // 500kHz
#define VSPI_BITORDER MSBFIRST
#define VSPI_MODE SPI_MODE1

int _ss_pin = 15;

void pulseSS() {
		digitalWrite(_ss_pin, HIGH);
		delayMicroseconds(5);
		digitalWrite(_ss_pin, LOW);
}

void initSPI() {
	pinMode(_ss_pin, OUTPUT);
	// digitalWrite(_ss_pin, HIGH);
	pulseSS();
	thisSPI.setFrequency(VSPI_FREQUENCY);
	thisSPI.setDataMode(VSPI_MODE);
	thisSPI.setBitOrder(VSPI_BITORDER);
	//            CLK, MISO, MOSI, SS
	thisSPI.begin(14,  4,    13,   15);
}

void stopSPI() {
	thisSPI.end();
}

void checkSPISlave() {
	String spiAnswer;
	uint32_t esp8266Status = spiGetStatus();
	if (esp8266Status == 0) {
		// Serial.println(infoLabel 
		// 	+ digitalTimeDisplaySec() 
		// 	+ " ESP8266 SPI status: " 
		// 	+ String(esp8266Status,HEX) + " -> no device found?");
		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI status: " + String(esp8266Status,HEX) + " -> no device found?", false);
	} else {
		// Serial.println(infoLabel 
		// 	+ digitalTimeDisplaySec() 
		// 	+ " ESP8266 SPI status: " 
		// 	+ String(esp8266Status,HEX));
		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI status: " + String(esp8266Status,HEX), false);
return;
		spiWriteData((uint8_t*) "Hello Slave!",(size_t)12);
		delay(20);
		spiAnswer = spiReadData();
		if (spiAnswer[0] != 'H') {
			// Serial.println(infoLabel 
			// 	+ digitalTimeDisplaySec() 
			// 	+ " ESP8266 SPI Hello Slave! got no response: " + spiAnswer);
			sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Hello Slave! got no response", false);
		} else {
			// Serial.println(infoLabel 
			// 	+ digitalTimeDisplaySec() 
			// 	+ " ESP8266 SPI Hello Slave! response: " + spiAnswer);
			sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Hello Slave! response: " + spiAnswer, false);
		}
		spiWriteData("Are you alive?");
		delay(20);
		spiAnswer = spiReadData();
		if (spiAnswer[0] != 'A') {
			// Serial.println(infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Are you alive? got no response: " + spiAnswer);
			sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Are you alive? got no response", false);
		} else {
			// Serial.println(infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Are you alive? response: " + spiAnswer);
			sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Are you alive? response: " + spiAnswer, false);
		}
		spiWriteData("Invalid question");
		delay(20);
		spiAnswer = spiReadData();
		if (spiAnswer[0] != 'S') {
			// Serial.println(infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Invalid question got no response: " + spiAnswer);
			sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Invalid question got no response", false);
		} else {
			// Serial.println(infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Invalid question response: " + spiAnswer);
			sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Invalid question response: " + spiAnswer, false);
		}
	}
}

uint32_t spiGetStatus() {
	thisSPI.beginTransaction(SPISettings(VSPI_FREQUENCY, VSPI_BITORDER, VSPI_MODE));
	// digitalWrite(_ss_pin, LOW);
	pulseSS();
	thisSPI.transfer(READ_STATUS);
	uint32_t status = 0;
	status = thisSPI.transfer32(0);
	// status = status >> 1; // bit shift to right 

	// digitalWrite(_ss_pin, HIGH);
	pulseSS();
	thisSPI.endTransaction();
	return status;
}

void spiWriteData(uint8_t * data, size_t len) {
	uint8_t i=0;
	thisSPI.beginTransaction(SPISettings(VSPI_FREQUENCY, VSPI_BITORDER, VSPI_MODE));
	// digitalWrite(_ss_pin, LOW);
	pulseSS();
	thisSPI.transfer(WRITE_DATA);
	thisSPI.transfer(CMD_FILLER);
	while(len-- && i < 32) {
		thisSPI.transfer(data[i++]);
	}
	while(i++ < 32) {
		thisSPI.transfer(0);
	}
	// digitalWrite(_ss_pin, HIGH);
	pulseSS();
	thisSPI.endTransaction();
}

void spiWriteData(const char * data) {
	spiWriteData((uint8_t *)data, strlen(data));
}

void spiReadData(uint8_t * data) {
	thisSPI.beginTransaction(SPISettings(VSPI_FREQUENCY, VSPI_BITORDER, VSPI_MODE));
	// digitalWrite(_ss_pin, LOW);
	pulseSS();
	thisSPI.transfer(READ_DATA);
	thisSPI.transfer(CMD_FILLER);
	for(uint8_t i=0; i<32; i++) {
		data[i] = thisSPI.transfer(0);
		// data[i] = data[i] >> 1; // bit shift to right 
	}
	// digitalWrite(_ss_pin, HIGH);
	pulseSS();
	thisSPI.endTransaction();
}

String spiReadData() {
 char data[33];
 data[32] = 0;
 spiReadData((uint8_t *)data);
 return String(data);
}
