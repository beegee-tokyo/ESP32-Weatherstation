#include "setup.h"

/** Touch status of T3 pad */
bool isTouchedT3 = false;
/** Long touch status of T3 pad */
bool longTouchT3 = false;
/** Time that pad T3 was touched */
long touchTimeT3 = 0;
/** Ticker for touch pad check */
Ticker touchTicker;

// void touchFunction();
void checkT3Status();
void touchT3ISR();

/**
 * initTouch
 * Initialize timer to read touch value every 500ms
 */
void initTouch() {
  touchTicker.attach_ms(500, checkT3Status);
  touchAttachInterrupt(T3, touchT3ISR, 20);
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
 * checkT3
 * Checks if T3 pad is still touched
*/
void checkT3Status() {
  if (isTouchedT3) {
    if (touchRead(T3) > 50) {
      isTouchedT3 = false;
      longTouchT3 = false;
      addMeeoMsg("touch", "0", false);
      addMeeoMsg("longtouch", "0", false);
    } else {
      if ((millis()-touchTimeT3) >= 3000) {
        if (!longTouchT3) {
          addMeeoMsg("longtouch", "1", false);
          xTaskResumeFromISR(weatherTaskHandle);
        }
        longTouchT3 = true;
      }
    }
  }
}
