/*
 * BLE2A6F.cpp
 *
 *  Created on: Dec 20, 2017
 *      Author: beegee-tokyo
 */

/*
 * See also:
 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.humidity.xml
 */
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "BLE2A6F.h"

BLE2A6F::BLE2A6F() : BLEDescriptor(BLEUUID((uint16_t) 0x2A6F)) {
	uint8_t data[2] = {0,0};
	setValue(data, 2);
} // BLE2A6F


/**
 * @brief Get the notifications value.
 * @return The notifications value.  True if notifications are enabled and false if not.
 */
bool BLE2A6F::getNotifications() {
	return (getValue()[0] & (1 << 0)) != 0;
} // getNotifications


/**
 * @brief Get the indications value.
 * @return The indications value.  True if indications are enabled and false if not.
 */
bool BLE2A6F::getIndications() {
	return (getValue()[0] & (1 << 1)) != 0;
} // getIndications


/**
 * @brief Set the indications flag.
 * @param [in] flag The indications flag.
 */
void BLE2A6F::setIndications(bool flag) {
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
void BLE2A6F::setNotifications(bool flag) {
	uint8_t *pValue = getValue();
	if (flag) {
		pValue[0] |= 1<<0;
	} else {
		pValue[0] &= ~(1<<0);
	}
} // setNotifications


#endif
