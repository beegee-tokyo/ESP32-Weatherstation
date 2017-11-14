#include "setup.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_TSL2561_U.h>

void configureSensor ();
long readLux();
void triggerGetLight();
void lightTask(void *pvParameters);

/** TSL2561 class for light measurement */
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified ( TSL2561_ADDR_FLOAT, 12345 );
/** Wire class for ESP32 */
TwoWire esp32Wire = TwoWire(0);
/** Task handle for light measurement task */
TaskHandle_t lightTaskHandle = NULL;
/** Pin number for LDR analog input pin */
int ldrPin = 36;
/** Flag if TSL2561 light sensor was found */
bool hasTSLSensor = false;
/** Currently used integration time for light sensor, 0 = 13.7ms, 1 = 101ms, 2 = 402ms */
int lightInteg = 2;
/** LDR light value = 0 if not updated */
long newLDRValue = 0;
/** TLS light value = 0 if not updated */
int newTSLValue = 0;
/** Ticker for LED flashing */
Ticker lightTicker;

/**
 * initLight
 * Setup port for LDR and TSL2561 sensor
 * Starts task and timer for repeated measurement
 * @return byte
 *    0 if light sensor was found and update task and timer are inialized
 *    1 if no no TSL2561 sensor was found but update task and timer are inialized
 *    2 if update timer could not be started
 */
byte initLight() {
  byte resultValue = 0;
  // Initialize analog port for LDR
  pinMode(ldrPin,INPUT);
	adcAttachPin(ldrPin);
	analogReadResolution(11);
	analogSetAttenuation(ADC_6db);

  // Initialize light sensor with SDA,SCL,frequency
	esp32Wire.begin(21,22,100000);
	if (tsl.begin(&esp32Wire)) {
    hasTSLSensor = true;
		configureSensor();
  } else {
    hasTSLSensor = false;
    resultValue = 1;
  }
  // Start task for light value readings
	xTaskCreatePinnedToCore(
	    lightTask,                       /* Function to implement the task */
	    "LightMeasure ",                 /* Name of the task */
	    4000,                            /* Stack size in words */
	    NULL,                            /* Task input parameter */
	    5,                               /* Priority of the task */
	    &lightTaskHandle,                /* Task handle. */
	    1);                              /* Core where the task should run */

  if (lightTaskHandle == NULL) {
    resultValue = 2;
  } else {
    // Start update of light values data every 10 seconds
    lightTicker.attach(10, triggerGetLight);
  }

  return resultValue;
}
/**
 * Start task to reads TSL2561 light sensor and LDR analog value
 */
void triggerGetLight() {
	if (lightTaskHandle != NULL) {
    xTaskResumeFromISR(lightTaskHandle);
  }
}

/**
 * Task to read data from TSL2561 light sensor and LDR sensor
 * @param pvParameters
 *    pointer to task parameters
 */
void lightTask(void *pvParameters) {
	Serial.println("lightTask loop started");
	while (1) // lightTask loop
  {
		if (otaRunning)
		{
			vTaskDelete(NULL);
		}
		if (tasksEnabled) {
      if (hasTSLSensor) {
        // Read TSL2561 light sensor
  			long collLight = readLux();
  			if (collLight != 65536) {
  				newTSLValue = collLight;
  			} else {
          newTSLValue = 0;
          Serial.println("[ERROR] " + digitalTimeDisplaySec() + " Failed to read from TSL2561");
      		addMeeoMsg("", "[ERROR] " + digitalTimeDisplaySec() + " Failed to read from TSL2561", true);
          hasTSLSensor = false;
        }
  			esp32Wire.reset();
      } else {
        // Try to initialize sensor again
        // Initialize light sensor with SDA,SCL,frequency
        esp32Wire.reset();
      	esp32Wire.begin(21,22,100000);
      	if (tsl.begin(&esp32Wire)) {
          hasTSLSensor = true;
      		configureSensor();
        } else {
          hasTSLSensor = false;
        }
      }

			// Read analog value of LDR
			newLDRValue = analogRead(ldrPin);
		}
		vTaskSuspend(NULL);
	}
}

/**
 * Configures the gain and integration time for the TSL2561
 */
void configureSensor() {
	/* You can also manually set the gain or enable auto-gain support */
	tsl.enableAutoRange ( true );				 /* Auto-gain ... switches automatically between 1x and 16x */

	/* Changing the integration time gives you better sensor resolution (402ms = 16-bit data) */
	tsl.setIntegrationTime ( TSL2561_INTEGRATIONTIME_402MS ); /* 16-bit data but slowest conversions */
}

/**
 * Get current light measurement.
 * Function makes 5 measurements and returns the average value.
 * Function adapts integration time in case of sensor overload
 *
 * @result long
 *    measured light in Lux, 0 if measurement failed
 */
long readLux() {
	/** Accumulated sensor values */
	long accLux = 0;
	/** Sensor event reads value from the sensor */
	sensors_event_t event;

	/** Counter for successfull readings, used to adjust the integration time */
	int lightOk = 0; /* In case of saturation we retry 5 times */

	for ( int i = 0; i < 5; i++ ) {
		tsl.getEvent ( &event );

		/* Display the results (light is measured in lux) */
		if ( event.light ) {
			/** Int value read from AD conv for sun measurement */
			accLux += event.light;
			lightOk++; /* Increase counter of successful measurements */

			if ( lightInteg == 1 ) { /* we are at medium integration time, try a higher one */
				tsl.setIntegrationTime ( TSL2561_INTEGRATIONTIME_402MS ); /* 16-bit data but slowest conversions */
				/* Test new integration time */
				tsl.getEvent ( &event );

				if ( event.light == 0 ) {
					/* Satured, switch back to medium integration time */
					tsl.setIntegrationTime ( TSL2561_INTEGRATIONTIME_101MS ); /* medium resolution and speed	 */
				} else {
					lightInteg = 2;
				}
			} else if ( lightInteg == 0 ) { /* we are at lowest integration time, try a higher one */
				tsl.setIntegrationTime ( TSL2561_INTEGRATIONTIME_101MS ); /* medium resolution and speed	 */
				/* Test new integration time */
				tsl.getEvent ( &event );

				if ( event.light == 0 ) {
					/* Satured, switch back to low integration time */
					tsl.setIntegrationTime ( TSL2561_INTEGRATIONTIME_13MS ); /* fast but low resolution */
				} else {
					lightInteg = 1;
				}
			}
		} else {
			/* If event.light = 0 lux the sensor is probably saturated and no reliable data could be generated! */
			if ( lightInteg == 2 ) { /* we are at highest integration time, try a lower one */
				tsl.setIntegrationTime ( TSL2561_INTEGRATIONTIME_101MS ); /* medium resolution and speed	 */
				tsl.getEvent ( &event );

				if ( event.light == 0 ) { /* Still saturated? */
					lightInteg = 0;
					tsl.setIntegrationTime ( TSL2561_INTEGRATIONTIME_13MS ); /* fast but low resolution */
					tsl.getEvent ( &event );

					if ( event.light != 0 ) { /* Got a result now? */
						accLux += event.light;
						lightOk++; /* Increase counter of successful measurements */
					}
				} else {
					lightInteg = 1;
					accLux += event.light;
					lightOk++; /* Increase counter of successful measurements */
				}
			} else if ( lightInteg == 1 ) { /* we are at medium integration time, try a lower one */
				lightInteg = 0;
				tsl.setIntegrationTime ( TSL2561_INTEGRATIONTIME_13MS ); /* fast but low resolution */
				tsl.getEvent ( &event );
				if ( event.light != 0 ) { /* Got a result now? */
					accLux += event.light;
					lightOk++; /* Increase counter of successful measurements */
				}
			}
		}
	}

	if ( lightOk != 0 ) {
		return (accLux / lightOk);
	} else {
		return 0;
	}
}
