#include "setup.h"

/** Timer for LED flashing */
hw_timer_t *ledFlasher = NULL;
/** Pin number for onboard LED */
uint8_t  ledPin = 16;

/**
	initLeds
	Setup port for LED's
	Starts flashing with 1000ms Interval
*/
void initLed() {
	pinMode(ledPin, OUTPUT);
	digitalWrite(ledPin, HIGH);
	// Start slow flashing (1000ms Interval)
	startFlashing(1000);
}

/**
	ledFlash
	Called by timer interrupt
	Inverts status of the LED
*/
void ledFlash() {
	digitalWrite(ledPin, !digitalRead(ledPin));
}

/**
	startFlashing
	Start a <flashTime> ms flashing interval
*/
void startFlashing(uint16_t flashTime) {
	if (ledFlasher != NULL) {
		stopTimer(ledFlasher);
	}
	ledFlasher = startTimerMsec(flashTime, ledFlash, true);
}

/**
	stopFlashing
	Stop flashing the LED and switch it off
*/
void stopFlashing() {
	if (ledFlasher != NULL) {
		stopTimer(ledFlasher);
		ledFlasher = NULL;
	}
	digitalWrite(ledPin, HIGH);
}
