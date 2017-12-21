/*
 * BLE2A46.h
 *
 *  Created on: Dec 20, 2017
 *      Author: beegee-tokyo
 */

#ifndef COMPONENTS_CPP_UTILS_BLE2A46_H_
#define COMPONENTS_CPP_UTILS_BLE2BLE2A46_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "BLEDescriptor.h"

/**
 * @brief Descriptor for Client Characteristic Configuration.
 *
 * This is a descriptor for the Client Characteristic Configuration New Alert which has a UUID of 0x2A46.
 *
 * See also:
 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.new_alert.xml
 */
class BLE2A46: public BLEDescriptor {
public:
	BLE2A46();
	bool getNotifications();
	bool getIndications();
	void setNotifications(bool flag);
	void setIndications(bool flag);

}; // BLE2A46

#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLE2A46_H_ */
