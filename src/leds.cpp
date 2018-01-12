#include "setup.h"

void ledFlash();

/** Pin number for onboard LED */
uint8_t	ledPin = 16;
/** Ticker for LED flashing */
Ticker ledTicker;

/**
 * initLeds
 * Setup port for LED's
 * Starts flashing with 1000ms Interval
 */
void initLed() {
	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, HIGH);
}

/**
 * ledFlash
 * Called by timer interrupt
 * Inverts status of the LED
 */
void ledFlash() {
	digitalWrite(ledPin, !digitalRead(ledPin));
}

/**
 * startFlashing
 * Start LED flashing
 * @param flashTime
 *		flashing interval in ms
 */
void startFlashing(uint16_t flashTime) {
	ledTicker.detach();
	ledTicker.attach_ms(flashTime, ledFlash);
}

/**
 * stopFlashing
 * Stop flashing the LED and switch it off
 */
void stopFlashing() {
	ledTicker.detach();
	digitalWrite(ledPin, HIGH);
}
