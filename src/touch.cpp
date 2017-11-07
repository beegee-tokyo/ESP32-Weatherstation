#include "setup.h"

// /** Lock for touch read */
// portMUX_TYPE touchMux = portMUX_INITIALIZER_UNLOCKED;
/** Touch status of T3 pad */
bool isTouchedT3 = false;
/** Long touch status of T3 pad */
bool longTouchT3 = false;
/** Time that pad T3 was touched */
long touchTimeT3 = 0;

// void touchFunction();
void checkT3Status();
void touchT3ISR();

/**
	initTouch
	Initialize timer to read touch value every 500ms
*/
void initTouch() {
  // startTimerMsec(500, touchFunction, true);
  startTimerMsec(500, checkT3Status, true);
  touchAttachInterrupt(T3, touchT3ISR, 20);
}

/**
  touchT3ISR
  Called when touch pin value goes below treshold
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
  checkT3
  Checks if T3 pad is still touched
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
  
// /**
// 	touchFunction
//   Check touch value and restart all tasks if touched
// */
// void touchFunction() {
//   // portENTER_CRITICAL(&touchMux);
//   int touchValue = touchRead(T3);
//   // portEXIT_CRITICAL(&touchMux);
//
//   // if (touchValue < 50) {
//   //   Serial.println(String(touchCounter) + "\t\tTouch value = " + String(touchValue));
//   // } else {
//   //   Serial.println(String(touchCounter) + " Touch value = " + String(touchValue));
//   // }
//   // touchCounter++;
//
// 	if (touchValue < 20) {
//     touchValue = touchRead(T3);
//     if (touchValue < 20) {
//       if (!isTouched) {
//   			isTouched = true;
//   			addMeeoMsg("touch", "1", false);
//   			xTaskResumeFromISR(lightTaskHandle);
//         xTaskResumeFromISR(tempTaskHandle);
//         xTaskResumeFromISR(weatherTaskHandle);
//   		}
//     }
// 	}
// 	if (touchValue > 50) {
// 		if (isTouched) {
// 			isTouched = false;
// 			addMeeoMsg("touch", "0", false);
// 		}
// 	}
// }
