#include "setup.h"

#define PAD1 T8 // GPIO33
#define PAD2 T9 // GPIO32
#define PAD3 T7 // GPIO27

/** Touch status of Touch pad 1 */
bool shortTouchPad1 = false;
/** Long touch status of pad 1 */
bool longTouchPad1 = false;
/** Time that pad 1 was touched */
long touchTimePad1 = 0;
/** Ticker for touch pad 1 check */
Ticker touchTickerPad1;
/** Flag if pad 1 was touched */
bool pad1Touched = false;

/** Touch status of pad 2 */
bool shortTouchPad2 = false;
/** Long touch status of pad 2 */
bool longTouchPad2 = false;
/** Time that pad 2 was touched */
long touchTimePad2 = 0;
/** Ticker for touch pad 2 check */
Ticker touchTickerPad2;
/** Flag if pad 2 was touched */
bool pad2Touched = false;

/** Touch status of pad 3 */
bool shortTouchPad3 = false;
/** Long touch status of pad 3 */
bool longTouchPad3 = false;
/** Time that pad 3 was touched */
long touchTimePad3 = 0;
/** Ticker for touch pad 3 check */
Ticker touchTickerPad3;
/** Flag if pad 3 was touched */
bool pad3Touched = false;

void checkPad1TouchStatus();
void checkPad2TouchStatus();
void checkPad3TouchStatus();
void touchPad1ISR();
void touchPad2ISR();
void touchPad3ISR();

/**
 * initTouch
 * Attach interrupts to the touch pad pins
 */
void initTouch() {
	touchAttachInterrupt(PAD1, touchPad1ISR, 20);
	touchAttachInterrupt(PAD2, touchPad2ISR, 20);
	touchAttachInterrupt(PAD3, touchPad3ISR, 20);
}

/**
 * touchPad1ISR
 * Called when touch pin value goes below treshold
*/
void touchPad1ISR() {
	if (!pad1Touched) {
		touchTickerPad1.attach_ms(50, checkPad1TouchStatus);
		touchTimePad1= millis();
		pad1Touched = true;
		// Serial.println("Pad 1 touch start at " + String(touchTimePad1));
	}
}

/**
 * checkPad1TouchStatus
 * Checks if pad 1 is still touched
*/
void checkPad1TouchStatus() {
	if (touchRead(PAD1) > 50) { // No longer touched
		if ((millis()-touchTimePad1) >= 500) { //	500ms => long touch!
			// Serial.println("Pad 1 long touch detected after " + String((millis()-touchTimePad1)));
			longTouchPad1 = true;
		} else { // short touch!
			// Serial.println("Pad 1 short touch detected after " + String((millis()-touchTimePad1)));
			shortTouchPad1 = true;
		}
		pad1Touched = false;
		touchTickerPad1.detach();
	}
}

/**
 * touchPad2ISR
 * Called when touch pin value goes below treshold
*/
void touchPad2ISR() {
	if (!pad2Touched) {
		touchTickerPad2.attach_ms(50, checkPad2TouchStatus);
		touchTimePad2= millis();
		pad2Touched = true;
		// Serial.println("Pad 2 touch start at " + String(touchTimePad2));
	}
}

/**
 * checkPad2TouchStatus
 * Checks if pad 2 is still touched
*/
void checkPad2TouchStatus() {
	if (touchRead(PAD2) > 50) { // No longer touched
		if ((millis()-touchTimePad2) >= 500) { //	500ms => long touch!
			Serial.println("Pad 2 long touch detected after " + String((millis()-touchTimePad2)));
			addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " RESET request", false);
			delay(2000);
			esp_restart();
		} else { // short touch!
			Serial.println("Pad 2 short touch detected after " + String((millis()-touchTimePad2)));
			if (lightTaskHandle != NULL) {
				xTaskResumeFromISR(lightTaskHandle);
			}
			if (tempTaskHandle != NULL) {
				xTaskResumeFromISR(tempTaskHandle);
			}
			if (weatherTaskHandle != NULL) {
				xTaskResumeFromISR(weatherTaskHandle);
			}
		}
		pad2Touched = false;
		touchTickerPad2.detach();
	}
}

/**
 * touchPad3ISR
 * Called when touch pin value goes below treshold
*/
void touchPad3ISR() {
	if (!pad3Touched) {
		touchTickerPad3.attach_ms(50, checkPad3TouchStatus);
		touchTimePad3= millis();
		pad3Touched = true;
		// Serial.println("Pad 3 touch start at " + String(touchTimePad1));
	}
}

/**
 * checkPad3TouchStatus
 * Checks if pad 3 is still touched
*/
void checkPad3TouchStatus() {
	if (touchRead(PAD3) > 50) { // No longer touched
		if ((millis()-touchTimePad3) >= 500) { //	500ms => long touch!
			// Serial.println("Pad 3 long touch detected after " + String((millis()-touchTimePad3)));
			longTouchPad3 = true;
		} else { // short touch!
			// Serial.println("Pad 3 short touch detected after " + String((millis()-touchTimePad3)));
			shortTouchPad3 = true;
		}
		pad3Touched = false;
		touchTickerPad3.detach();
	}
}
