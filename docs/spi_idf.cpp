/* SPI Slave example, sender (uses SPI master driver)

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "setup.h"

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "lwip/igmp.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "soc/rtc_cntl_reg.h"
#include "rom/cache.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include "esp_spi_flash.h"

#include "soc/gpio_reg.h"
#include "driver/gpio.h"
#include "esp_intr_alloc.h"


/*
SPI sender (master) example.

This example is supposed to work together with the SPI receiver. It uses the standard SPI pins (MISO, MOSI, SCLK, CS) to 
transmit data over in a full-duplex fashion, that is, while the master puts data on the MOSI pin, the slave puts its own
data on the MISO pin.

This example uses one extra pin: GPIO_HANDSHAKE is used as a handshake pin. The slave makes this pin high as soon as it is
ready to receive/send data. This code connects this line to a GPIO interrupt which gives the rdySem semaphore. The main 
task waits for this semaphore to be given before queueing a transmission.
*/


/*
Pins in use. The SPI Master can use the GPIO mux, so feel free to change these if needed.
*/
#define GPIO_MOSI 13
#define GPIO_MISO 4
#define GPIO_SCLK 14
#define GPIO_CS 15

#define WRITE_STATUS 0x01
#define WRITE_DATA   0x02
#define READ_DATA    0x03
#define READ_STATUS  0x04
#define CMD_FILLER   0x00

spi_device_handle_t handle = NULL;
int _ss_pin_idf = 15;

void pulseSSidf() {
		// digitalWrite(_ss_pin_idf, HIGH);
		// delayMicroseconds(5);
		// digitalWrite(_ss_pin_idf, LOW);
}

//Main application
esp_err_t initSPIidf() {
	esp_err_t ret;

	// pinMode(_ss_pin_idf, OUTPUT);
	// pulseSSidf();
	//Configuration for the SPI bus
	spi_bus_config_t buscfg={
			.mosi_io_num=GPIO_MOSI,
			.miso_io_num=GPIO_MISO,
			.sclk_io_num=GPIO_SCLK,
			.quadwp_io_num=-1,
			.quadhd_io_num=-1
	};

	//Configuration for the SPI device on the other side of the bus
	spi_device_interface_config_t devcfg;
	memset(&devcfg, 0, sizeof(devcfg));       //Zero out the configuration
	// devcfg.command_bits=7;           //7 bit command phase
	// devcfg.command_bits=0;           //7 bit command phase
	// devcfg.address_bits=0;
	// devcfg.dummy_bits=0;
	// devcfg.clock_speed_hz=500000;     //500kHz
	// devcfg.duty_cycle_pos=128;        //50% duty cycle
	// devcfg.mode=1;
	// devcfg.spics_io_num=GPIO_CS;
	// devcfg.cs_ena_posttrans=3;        //Keep the CS low 3 cycles after transaction, to stop slave from missing the last bit when CS has less propagation delay than CLK
	// devcfg.cs_ena_pretrans=3;        //Keep the CS low 3 cycles before transaction, to give slave soem time
	// devcfg.queue_size=3;
	// devcfg.flags = SPI_DEVICE_HALFDUPLEX;

	devcfg.clock_speed_hz=500000;     //500kHz
	devcfg.mode=1;
	devcfg.spics_io_num=GPIO_CS;               //CS pin
	devcfg.queue_size=3;
	devcfg.flags = SPI_TRANS_VARIABLE_CMD | SPI_DEVICE_HALFDUPLEX;

	int n=0;
	spi_transaction_t t;
	memset(&t, 0, sizeof(t));

	//Initialize the SPI bus and add the device we want to send stuff to.
	ret=spi_bus_initialize(HSPI_HOST, &buscfg, 1);
	if (ret != ESP_OK) {
		return ret;
	}
	ret=spi_bus_add_device(HSPI_HOST, &devcfg, &handle);
	return ret;
}

esp_err_t stopSPIidf() {
	if (handle != NULL) {
		return spi_bus_remove_device(handle);
	}
	return ESP_FAIL;
}


esp_err_t transmitSPIidf (uint16_t cmd, uint8_t * txBuffer, uint8_t * rxBuffer, size_t txLength) {
	if (handle != NULL) {
		spi_transaction_ext_t t_cmd;
		spi_transaction_ext_t t_data;
		memset(&t_cmd, 0, sizeof(t_cmd));       //Zero out the transaction
		memset(&t_data, 0, sizeof(t_data));       //Zero out the transaction

		// Prepare  7bit command
		t_cmd.base.cmd = cmd;
		t_cmd.base.tx_buffer = NULL;
		// t_cmd.base.length = NULL;
		// t_cmd.base.rxlength = NULL;
		t_cmd.base.rx_buffer = NULL;
		t_cmd.command_bits = 7;

		// t_cmd.tx_buffer=&cmd;
		// t_cmd.length=7; // tx buffer in bits
		// t_cmd.rxlength=4*8; // rx buffer in bits

		// Prepare data
		t_data.base.length=8*4;
		t_data.base.rxlength=8*4;
		// t_data.flags = SPI_TRANS_USE_RXDATA;
		t_data.base.tx_buffer = NULL;
		t_data.base.rx_buffer = rxBuffer;
		t_data.command_bits = 0;

		// t.tx_buffer=&cmd;
		// t.tx_buffer=txBuffer;
		// t.rx_buffer=rxBuffer;
		// t.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA; 
		// t.flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA; 
		// t.flags = SPI_TRANS_USE_TXDATA; 
	// pulseSSidf();
		// First send 7bit command
		spi_device_transmit(handle, (spi_transaction_t*)&t_cmd);

		// Then get the data
		spi_device_transmit(handle, (spi_transaction_t*)&t_data);
	// pulseSSidf();
		Serial.print("Buffer content: 0x");
		Serial.print(String(t_data.base.rx_data[0],HEX));
		Serial.print(String(t_data.base.rx_data[1],HEX));
		Serial.print(String(t_data.base.rx_data[2],HEX));
		Serial.println(String(t_data.base.rx_data[3],HEX));
		Serial.print("Buffer content: 0x");
		Serial.print(String(rxBuffer[0],HEX));
		Serial.print(String(rxBuffer[1],HEX));
		Serial.print(String(rxBuffer[2],HEX));
		Serial.println(String(rxBuffer[3],HEX));
		// memcpy(rxBuffer,t.rx_data,4);
	}
	return ESP_FAIL;
}

esp_err_t checkSPISlaveIDF() {
	String spiAnswer;
	uint8_t status[4];
	uint8_t sendBuf[32];
	uint8_t recvBuf[32];
	esp_err_t transmitResult;

	memset(status,0,4);
	memset(recvBuf,0,32);
	transmitResult = transmitSPIidf(READ_STATUS, sendBuf, status, 1); // 0x04 => read status
	sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() 
		+ " Transmit result: " + String(transmitResult)
		+ " ESP8266 SPI status: " + String((uint32_t)status,HEX), false);

return transmitResult;

	// 	spiWriteData((uint8_t*) "Hello Slave!",(size_t)12);
	// 	delay(20);
	// 	spiAnswer = spiReadData();
	// 	if (spiAnswer[0] != 'H') {
	// 		// Serial.println(infoLabel 
	// 		// 	+ digitalTimeDisplaySec() 
	// 		// 	+ " ESP8266 SPI Hello Slave! got no response: " + spiAnswer);
	// 		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Hello Slave! got no response", false);
	// 	} else {
	// 		// Serial.println(infoLabel 
	// 		// 	+ digitalTimeDisplaySec() 
	// 		// 	+ " ESP8266 SPI Hello Slave! response: " + spiAnswer);
	// 		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Hello Slave! response: " + spiAnswer, false);
	// 	}
	// 	spiWriteData("Are you alive?");
	// 	delay(20);
	// 	spiAnswer = spiReadData();
	// 	if (spiAnswer[0] != 'A') {
	// 		// Serial.println(infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Are you alive? got no response: " + spiAnswer);
	// 		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Are you alive? got no response", false);
	// 	} else {
	// 		// Serial.println(infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Are you alive? response: " + spiAnswer);
	// 		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Are you alive? response: " + spiAnswer, false);
	// 	}
	// 	spiWriteData("Invalid question");
	// 	delay(20);
	// 	spiAnswer = spiReadData();
	// 	if (spiAnswer[0] != 'S') {
	// 		// Serial.println(infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Invalid question got no response: " + spiAnswer);
	// 		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Invalid question got no response", false);
	// 	} else {
	// 		// Serial.println(infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Invalid question response: " + spiAnswer);
	// 		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " ESP8266 SPI Invalid question response: " + spiAnswer, false);
	// 	}
	// }
}
