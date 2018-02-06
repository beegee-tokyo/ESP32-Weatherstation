#include "setup.h"
#include "driver/spi_slave.h"

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

SPIClass slaveSPI(HSPI);

void post_setup_cb(spi_slave_transaction_t *trans) {
	Serial.println("SPI registers loaded");
	Serial.println("Length: " + String(trans->length));
	Serial.println("Transaction Length: " + String(trans->trans_len));
	if (trans->tx_buffer != NULL) {
		char * tx_data = (char *)trans->tx_buffer;
		Serial.println("TX buffer 0: " + String(tx_data[0]));
	}
	if (trans->rx_buffer != NULL) {
		char * rx_data = (char *)trans->rx_buffer;
		Serial.println("RX buffer 0: " + String(rx_data[0]));
	}
}

void post_trans_cb(spi_slave_transaction_t *trans) {
	Serial.println("SPI transaction finished");
	Serial.println("Length: " + String(trans->length));
	Serial.println("Transaction Length: " + String(trans->trans_len));
	if (trans->tx_buffer != NULL) {
		char * tx_data = (char *)trans->tx_buffer;
		Serial.println("TX buffer 0: " + String(tx_data[0]));
	}
	if (trans->rx_buffer != NULL) {
		char * rx_data = (char *)trans->rx_buffer;
		Serial.println("RX buffer 0: " + String(rx_data[0]));
	}
}

esp_err_t initSPIslave() {
	//Configuration for the SPI bus
	spi_bus_config_t buscfg={
			.mosi_io_num=GPIO_MOSI,
			.miso_io_num=GPIO_MISO,
			.sclk_io_num=GPIO_SCLK,
			.quadwp_io_num=-1,
			.quadhd_io_num=-1
	};

	spi_slave_interface_config_t slaveConfig;
	slaveConfig.spics_io_num = GPIO_CS;
	slaveConfig.flags = 0;
	slaveConfig.queue_size = 3;
	slaveConfig.mode = 1;
	slaveConfig.post_setup_cb = post_setup_cb;
	slaveConfig.post_trans_cb = post_trans_cb;

	slaveSPI.begin();
	return spi_slave_initialize(HSPI_HOST, &buscfg, &slaveConfig, 1);
}