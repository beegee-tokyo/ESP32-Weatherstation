#include "setup.h"
#include <SPI.h>

#define WRITE_STATUS 0x01
#define WRITE_DATA 0x02
#define READ_DATA 0x03
#define READ_STATUS 0x04
#define CMD_FILLER 0x00

SPIClass thisSPI(HSPI);

#define VSPI_FREQUENCY 140000
#define VSPI_BITORDER MSBFIRST
#define VSPI_MODE SPI_MODE0

int _ss_pin = 15;

void initSPI() {
  // CLK, MISO, MOSI, SS
  thisSPI.begin(14, 12, 13, 15);
  // thisSPI.setFrequency(VSPI_FREQUENCY);
}

void stopSPI() {
  thisSPI.end();
}

void checkSPISlave() {
  String spiAnswer;
  uint32_t esp8266Status = spiGetStatus();
  if (esp8266Status == 0) {
  	addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI status: " + String(esp8266Status) + " -> no device found?", false);
  } else {
  	addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI status: " + String(esp8266Status), false);
  	spiWriteData((uint8_t*) "Hello Slave!",(size_t)12);
  	delay(10);
  	spiAnswer = spiReadData();
  	if (spiAnswer[0] != 'H') {
  		addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI Hello Slave! got no response", false);
  	} else {
  		addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI Hello Slave! response: " + spiAnswer, false);
  	}
  	spiWriteData("Are you alive?");
  	delay(10);
  	spiAnswer = spiReadData();
  	if (spiAnswer[0] != 'A') {
  		addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI Are you alive? got no response", false);
  	} else {
  		addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI Are you alive? response: " + spiAnswer, false);
  	}
  	spiWriteData("Invalid question");
  	delay(10);
  	spiAnswer = spiReadData();
  	if (spiAnswer[0] != 'S') {
  		addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI Invalid question got no response", false);
  	} else {
  		addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " ESP8266 SPI Invalid question response: " + spiAnswer, false);
  	}
  }
}

uint32_t spiGetStatus() {
  thisSPI.beginTransaction(SPISettings());
  // thisSPI.beginTransaction(SPISettings(VSPI_FREQUENCY, VSPI_BITORDER, VSPI_MODE));
  digitalWrite(_ss_pin, LOW);
  thisSPI.transfer(READ_STATUS);
  uint32_t status = thisSPI.transfer32(0);
  digitalWrite(_ss_pin, HIGH);
  thisSPI.endTransaction();
  return status;
}

void spiWriteData(uint8_t * data, size_t len) {
  uint8_t i=0;
  thisSPI.beginTransaction(SPISettings());
  // thisSPI.beginTransaction(SPISettings(VSPI_FREQUENCY, VSPI_BITORDER, VSPI_MODE));
  digitalWrite(_ss_pin, LOW);
  thisSPI.transfer(WRITE_DATA);
  // thisSPI.transfer(CMD_FILLER);
  while(len-- && i < 32) {
    thisSPI.transfer(data[i++]);
  }
  while(i++ < 32) {
    thisSPI.transfer(0);
  }
  digitalWrite(_ss_pin, HIGH);
  thisSPI.endTransaction();
}

void spiWriteData(const char * data) {
  spiWriteData((uint8_t *)data, strlen(data));
}

void spiReadData(uint8_t * data) {
  thisSPI.beginTransaction(SPISettings());
  // thisSPI.beginTransaction(SPISettings(VSPI_FREQUENCY, VSPI_BITORDER, VSPI_MODE));
  digitalWrite(_ss_pin, LOW);
  thisSPI.transfer(READ_DATA);
  thisSPI.transfer(CMD_FILLER);
  for(uint8_t i=0; i<32; i++) {
    data[i] = thisSPI.transfer(0);
  }
  digitalWrite(_ss_pin, HIGH);
  thisSPI.endTransaction();
}

String spiReadData() {
 char data[33];
 data[32] = 0;
 spiReadData((uint8_t *)data);
 return String(data);
}
