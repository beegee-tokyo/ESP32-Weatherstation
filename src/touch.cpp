#include "setup.h"

/** Touch status of T0 pad */
bool isTouchedT0 = false;
/** Long touch status of T0 pad */
bool longTouchT0 = false;
/** Time that pad T0 was touched */
long touchTimeT0 = 0;
/** Touch status of T2 pad */
bool isTouchedT2 = false;
/** Long touch status of T2 pad */
bool longTouchT2 = false;
/** Time that pad T2 was touched */
long touchTimeT2 = 0;
/** Ticker for touch pad check */
Ticker touchTicker;

void checkTouchStatus();
void touchT0ISR();
void touchT2ISR();

/**
 * initTouch
 * Initialize timer to read touch value every 500ms
 */
void initTouch() {
  touchTicker.attach_ms(250, checkTouchStatus);
  touchAttachInterrupt(T0, touchT0ISR, 20);
  touchAttachInterrupt(T2, touchT2ISR, 20);
}

/**
 * touchT0ISR
 * Called when touch pin value goes below treshold
*/
void touchT0ISR() {
  if (!isTouchedT0) {
    touchTimeT0= millis();
    isTouchedT0 = true;
    addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " touch T0", false);
    // xTaskResumeFromISR(lightTaskHandle);
    // xTaskResumeFromISR(tempTaskHandle);
  }
}

/**
 * touchT2ISR
 * Called when touch pin value goes below treshold
*/
void touchT2ISR() {
  if (!isTouchedT2) {
    touchTimeT2 = millis();
    isTouchedT2 = true;
    xTaskResumeFromISR(lightTaskHandle);
    xTaskResumeFromISR(tempTaskHandle);
    xTaskResumeFromISR(weatherTaskHandle);
  }
}

/**
 * checkTouchStatus
 * Checks if T2 or T0 pad is still touched
*/
void checkTouchStatus() {
  if (isTouchedT0) {
    if (touchRead(T0) > 50) {
      isTouchedT0 = false;
      longTouchT0 = false;
    } else {
      if ((millis()-touchTimeT0) >= 1000) {
        if (!longTouchT0) {
          addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " longtouch T0", false);
          // xTaskResumeFromISR(weatherTaskHandle);
        }
        longTouchT0 = true;
      }
    }
  }
  if (isTouchedT2) {
    if (touchRead(T2) > 50) {
      isTouchedT2 = false;
      longTouchT2 = false;
    } else {
      if ((millis()-touchTimeT2) >= 1000) {
        if (!longTouchT2) {
          addMqttMsg("debug", "[INFO] " + digitalTimeDisplaySec() + " RESET request", false);
    			delay(2000);
    			esp_restart();
        }
        longTouchT2 = true;
      }
    }
  }
}
