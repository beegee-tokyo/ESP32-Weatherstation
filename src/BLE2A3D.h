/*
 * BLE2A3D.h
 *
 *  Created on: Dec 20, 2017
 *      Author: beegee-tokyo
 */

#ifndef COMPONENTS_CPP_UTILS_BLE2A3D_H_
#define COMPONENTS_CPP_UTILS_BLE2BLE2A3D_H_
#include "sdkconfig.h"
#if defined(CONFIG_BT_ENABLED)

#include "BLEDescriptor.h"

/**
 * @brief Descriptor for Client Characteristic Configuration.
 *
 * This is a descriptor for the Client Characteristic Configuration String which has a UUID of 0x2A3D.
 *
 * See also:
 * https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.characteristic.string.xml
 */
class BLE2A3D: public BLEDescriptor {
public:
	BLE2A3D();
	bool getNotifications();
	bool getIndications();
	void setNotifications(bool flag);
	void setIndications(bool flag);

}; // BLE2A3D

#endif /* CONFIG_BT_ENABLED */
#endif /* COMPONENTS_CPP_UTILS_BLE2A3D_H_ */
