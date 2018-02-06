#include "setup.h"

TwoWire slave8266 = TwoWire(2);

void initI2C() {
	//              sda, scl, frequency
	slave8266.begin(15,   4,  15000);
	slave8266.reset();
}

void i2cWrite(uint8_t value) {
  // portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
  // portENTER_CRITICAL(&mux);

	slave8266.beginTransmission(0x42);
	slave8266.write(value);
	slave8266.endTransmission();
	
	// portEXIT_CRITICAL(&mux);
}