#include "setup.h"

/** Touch status of T3 pad */
bool isTouchedT3 = false;
/** Long touch status of T3 pad */
bool longTouchT3 = false;
/** Time that pad T3 was touched */
long touchTimeT3 = 0;
/** Touch status of T2 pad */
bool isTouchedT2 = false;
/** Long touch status of T2 pad */
bool longTouchT2 = false;
/** Time that pad T2 was touched */
long touchTimeT2 = 0;
/** Ticker for touch pad check */
Ticker touchTicker;

void checkTouchStatus();
void touchT3ISR();
void touchT2ISR();

/**
 * initTouch
 * Initialize timer to read touch value every 500ms
 */
void initTouch() {
  touchTicker.attach_ms(250, checkTouchStatus);
  touchAttachInterrupt(T3, touchT3ISR, 20);
  touchAttachInterrupt(T2, touchT2ISR, 20);
}

/**
 * touchT3ISR
 * Called when touch pin value goes below treshold
*/
void touchT3ISR() {
  if (!isTouchedT3) {
    touchTimeT3 = millis();
    isTouchedT3 = true;
    addMeeoMsg("touch", "1", false);
    xTaskResumeFromISR(lightTaskHandle);
    xTaskResumeFromISR(tempTaskHandle);
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
  }
}

/**
 * checkTouchStatus
 * Checks if T2 or T3 pad is still touched
*/
void checkTouchStatus() {
  if (isTouchedT3) {
    if (touchRead(T3) > 50) {
      isTouchedT3 = false;
      longTouchT3 = false;
      addMeeoMsg("touch", "0", false);
      addMeeoMsg("longtouch", "0", false);
    } else {
      if ((millis()-touchTimeT3) >= 1000) {
        if (!longTouchT3) {
          addMeeoMsg("longtouch", "1", false);
          xTaskResumeFromISR(weatherTaskHandle);
        }
        longTouchT3 = true;
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
          addMeeoMsg("", "[INFO] " + digitalTimeDisplaySec() + " RESET request", true);
    			delay(2000);
    			esp_restart();
        }
        longTouchT3 = true;
      }
    }
  }
}
