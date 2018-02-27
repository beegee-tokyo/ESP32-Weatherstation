#include "setup.h"
#include <esp_partition.h>

void printPartitions() {
	// Get Partitionsizes
	size_t ul;
	esp_partition_iterator_t _mypartiterator;
	const esp_partition_t *_mypart;
	char mqttMsg[1024];

	ul = spi_flash_get_chip_size();
	sprintf(mqttMsg," Flash chip size: %d", ul);
	sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + String(mqttMsg), false);
	// delay(1000);

	sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " Partition table:", false);
	// delay(1000);

	_mypartiterator = esp_partition_find(ESP_PARTITION_TYPE_APP, ESP_PARTITION_SUBTYPE_ANY, NULL);
	if (_mypartiterator) {
		
		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " App partition table:", false);
		// delay(1000);
		do {
			_mypart = esp_partition_get(_mypartiterator);
			sprintf(mqttMsg," Type: %02x SubType %x Address 0x%06X Size 0x%06X Encryption %i Label %s", _mypart->type, _mypart->subtype, _mypart->address, _mypart->size, _mypart->encrypted, _mypart->label);
			sendDebug(debugLabel, infoLabel + String(mqttMsg), false);
			// delay(1000);
		} while (_mypartiterator = esp_partition_next(_mypartiterator));
	}
	esp_partition_iterator_release(_mypartiterator);
	_mypartiterator = esp_partition_find(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_ANY, NULL);
	if (_mypartiterator) {
		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " Data partition table:", false);
		// delay(1000);
		do {
			_mypart = esp_partition_get(_mypartiterator);
			sprintf(mqttMsg," Type: %02x SubType %02x Address 0x%06X Size 0x%06X Encryption %i Label %s", _mypart->type, _mypart->subtype, _mypart->address, _mypart->size, _mypart->encrypted, _mypart->label);
			sendDebug(debugLabel, infoLabel + String(mqttMsg), false);
			// delay(1000);
		} while (_mypartiterator = esp_partition_next(_mypartiterator));
	}
	esp_partition_iterator_release(_mypartiterator);
}

void printLastResetReason() {
	// Get last reset reason and publish it
	String resetReason = reset_reason(rtc_get_reset_reason(0));
	sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " Reset reason CPU0: " + resetReason, false);
	resetReason = reset_reason(rtc_get_reset_reason(1));
	sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " Reset reason CPU1: " + resetReason, false);

	sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " SDK Version: " + ESP.getSdkVersion(), false);
}