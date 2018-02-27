#include "setup.h"
#include "BLEDevice.h"

// The remote service we wish to connect to.
static BLEUUID serviceUUID("00001809-0000-1000-8000-00805f9b34fb");
// The characteristic of the remote service we are interested in.
// static BLEUUID		charUUID("0d563a58-196a-48ce-ace2-dfec78acc814");
static BLEUUID		charUUID("00002a1c-0000-1000-8000-00805f9b34fb");

BLEAddress *pServerAddress;
bool doConnect = false;
bool connected = false;
bool isScanning = false;

BLERemoteCharacteristic* pRemoteCharacteristic;
BLEClient* pClient;

bool bleIsScanning = false;

static void notifyCallback(
	BLERemoteCharacteristic* pBLERemoteCharacteristic,
	uint8_t* pData,
	size_t length,
	bool isNotify) {
		char data[length];
		memcpy(data,pData,length);

		// sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec()
		//			 + " BLE Notification: " + String(data)
		//			 + " from: " + pBLERemoteCharacteristic->getUUID().toString().c_str(), false);
		Serial.println(infoLabel + digitalTimeDisplaySec()
					+ " BLE Notification: " + String(data)
					+ " from: " + pBLERemoteCharacteristic->getUUID().toString().c_str());
		// TODO send notification to main loop to read data
}

bool connectToServer(BLEAddress pAddress) {
	// Create a client
	pClient	= BLEDevice::createClient();
	// Connect to the remove BLE Server.
	pClient->connect(pAddress);

	// Obtain a reference to the service we are after in the remote BLE server.
	BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
	if (pRemoteService == nullptr) {
		pClient->disconnect();
		// sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec()
		//			 + " BLE Failed to find service UUID: " + serviceUUID.toString().c_str(), false);
		// Serial.print("Failed to find our service UUID: ");
		// Serial.println(serviceUUID.toString().c_str());
		return false;
	}
	Serial.println(" - Found our service");

	// Obtain a reference to the characteristic in the service of the remote BLE server.
	pRemoteCharacteristic = pRemoteService->getCharacteristic(charUUID);
	if (pRemoteCharacteristic == nullptr) {
		pClient->disconnect();
		// sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec()
		//			 + " BLE Failed to find characteristic UUID: " + charUUID.toString().c_str(), false);
		Serial.print("Failed to find our characteristic UUID: ");
		Serial.println(charUUID.toString().c_str());
		return false;
	}

	// Read the value of the characteristic.
	// std::string value = pRemoteCharacteristic->readValue();
	// sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec()
	//			 + " BLE characteristic value was: " + value.c_str(), false);
	// Serial.print("The characteristic value was: ");
	// Serial.println(value.c_str());

	uint8_t value = pRemoteCharacteristic->readUInt8();
	// sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec()
	//			 + " BLE characteristic value was: " + String(value), false);
	Serial.print("The characteristic value was: ");
	Serial.println(value);

	pRemoteCharacteristic->registerForNotify(notifyCallback);
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
 /**
	 * Called for each advertising BLE server.
	 */
	void onResult(BLEAdvertisedDevice advertisedDevice) {
		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec()
					+ " BLE Advertised Device found: " + advertisedDevice.toString().c_str(), false);
		Serial.println("BLE scan: " + digitalTimeDisplaySec()
					+ " BLE Advertised Device found: " + advertisedDevice.toString().c_str());

		// // We have found a device, let us now see if it contains the service we are looking for.
		// if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(serviceUUID)) {
		//	 // sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec()
		//	 //						 + " Found correct service UUID", false);
		//	 advertisedDevice.getScan()->stop();
		//
		//	 pServerAddress = new BLEAddress(advertisedDevice.getAddress());
		//	 doConnect = true;
		//
		// } // Found our server
	} // onResult

	void onFinish() {
		Serial.println(infoLabel + digitalTimeDisplaySec() + " BLE Scan finished");
		sendDebug(debugLabel, infoLabel + digitalTimeDisplaySec() + " BLE Scan finished", false);
		isScanning = false;
	} // onFinish
}; // MyAdvertisedDeviceCallbacks

void scanBLEdevices() {
	BLEDevice::init("");
	// Retrieve a Scanner and set the callback we want to use to be informed when we
	// have detected a new device.	Specify that we want active scanning and start the
	// scan to run for 30 seconds.
	BLEScan* pBLEScan = BLEDevice::getScan();
	pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
	pBLEScan->setActiveScan(true);
	pBLEScan->start(10);
	isScanning = true;
}
