/** Build time */
extern const char compileDate[];

/** mDNS and Access point name */
extern char apName[];
/** Index to add module ID to apName */
extern int apIndex;

/** OTA update status */
extern bool otaRunning;

#ifdef HAS_TFT
/** TFT_eSPI class for display */
extern TFT_eSPI tft;
#endif

/**********************************************************/
// Function declarations
/**********************************************************/
/**
 * Activate OTA
 */
void activateOTA(const char *MODULTYPE, const char *MODULID);
/**
 * Connect to WiFi with pre-defined method
 */
bool connectWiFi();

/**********************************************************/
/**********************************************************/
// Put app specific function / variable declarations below
/**********************************************************/
/**********************************************************/
// Global variables
extern bool tasksEnabled;
extern FtpServer ftpSrv;

// UDP interface
void getUDPbroadcast(int udpMsgLength);
bool udpSendMessage(IPAddress ipAddr, String udpMsg, int udpPort);
extern WiFiUDP udpListener;
extern int udpMsgLength;

// Touch interface
void initTouch();
extern Ticker touchTicker;

// Meeo functions / variables
void initMeeo();
bool addMeeoMsg (String topic, String payload, bool debug = false);
extern bool meeoConnected;

// LED functions / variables
void initLed();
void startFlashing(uint16_t flashTime);
void stopFlashing();
extern Ticker ledTicker;

// Light functions / variables
byte initLight();
extern long newLDRValue;
extern int newTSLValue;
extern TaskHandle_t lightTaskHandle;
extern Ticker lightTicker;

// Temperature functions / variables
bool initTemp();
extern TaskHandle_t tempTaskHandle;
extern Ticker tempTicker;

// Weather and time update functions / variables
bool initWeather();
void weatherTask(void *pvParameters);
extern TaskHandle_t weatherTaskHandle;
extern Ticker weatherTicker;
