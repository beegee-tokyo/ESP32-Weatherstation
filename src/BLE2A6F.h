/*
 * BLE2A6F.h
 *
 *  Created on: Dec 20, 2017
 *      Author: beegee-tokyo
 */

#ifndef COMPONENTS_CPP_UTILS_BLE2A6F_H_
#define COMPONENTS_CPP_UTILS_BLE2BLE2A6F_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "BLEDescriptor.h"

/**
 * @brief Descriptor for Client Characteristic Configuration.
 *
 * This is a descriptor for the Client Characteristic Configuration Humidity which has a UUID of 0x2A6F.
 *
 * See also:
 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.humidity.xml
 */
class BLE2A6F: public BLEDescriptor {
public:
	BLE2A6F();
	bool getNotifications();
	bool getIndications();
	void setNotifications(bool flag);
	void setIndications(bool flag);

}; // BLE2A6F

#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLE2A6F_H_ */
