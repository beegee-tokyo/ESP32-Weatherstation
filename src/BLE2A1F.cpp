/*
 * BLE2A1F.cpp
 *
 *  Created on: Dec 20, 2017
 *      Author: beegee-tokyo
 */

/*
 * See also:
 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.temperature_celsius.xml
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "BLE2A1F.h"

BLE2A1F::BLE2A1F() : BLEDescriptor(BLEUUID((uint16_t) 0x2A1F)) {
	uint8_t data[2] = {0x1F,0x2A};
	setValue(data, 2);
} // BLE2A1F


/**
 * @brief Get the notifications value.
 * @return The notifications value.  True if notifications are enabled and false if not.
 */
bool BLE2A1F::getNotifications() {
	return (getValue()[0] & (1 << 0)) != 0;
} // getNotifications


/**
 * @brief Get the indications value.
 * @return The indications value.  True if indications are enabled and false if not.
 */
bool BLE2A1F::getIndications() {
	return (getValue()[0] & (1 << 1)) != 0;
} // getIndications


/**
 * @brief Set the indications flag.
 * @param [in] flag The indications flag.
 */
void BLE2A1F::setIndications(bool flag) {
	uint8_t *pValue = getValue();
	if (flag) {
		pValue[0] |= 1<<1;
	} else {
		pValue[0] &= ~(1<<1);
	}
} // setIndications


/**
 * @brief Set the notifications flag.
 * @param [in] flag The notifications flag.
 */
void BLE2A1F::setNotifications(bool flag) {
	uint8_t *pValue = getValue();
	if (flag) {
		pValue[0] |= 1<<0;
	} else {
		pValue[0] &= ~(1<<0);
	}
} // setNotifications


#endif
