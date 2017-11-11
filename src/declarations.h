#ifndef declarations_h
#define declarations_h

/** OTA update status */
bool otaRunning = false;

/**********************************************************/
// Initialize TFT if required
/**********************************************************/
#ifdef HAS_TFT
/** TFT_eSPI class for display */
TFT_eSPI tft = TFT_eSPI();
#endif

/**********************************************************/
/**********************************************************/
// Put app specific declarations below
/**********************************************************/
/**********************************************************/
/** WiFiUDP class for listening to UDP broadcasts */
WiFiUDP udpListener;
/** Length of received UDP broadcast message */
int udpMsgLength = 0;
/** FTP server class */
FtpServer ftpSrv;
/** HTTPClient class to get data from Weather website*/
HTTPClient http;
/** Task handle for the weather and time update task */
TaskHandle_t weatherTaskHandle = NULL;
/** Ticker for weather and time update */
Ticker weatherTicker;
/** Flag for tasks if main app is ready */
bool tasksEnabled = false;

#endif
