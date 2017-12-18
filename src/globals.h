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
/**
 * Get reset reason as string array
 */
String reset_reason(RESET_REASON reason);

/**********************************************************/
/**********************************************************/
// Put app specific function / variable declarations below
/**********************************************************/
/**********************************************************/
// Global variables
extern bool tasksEnabled;

// Weather interface
extern HTTPClient http;
/** Task handle for the weather and time update task */
extern TaskHandle_t weatherTaskHandle;
/** Ticker for weather and time update */
extern Ticker weatherTicker;

// UDP interface
void getUDPbroadcast(int udpMsgLength);
bool udpSendMessage(IPAddress ipAddr, String udpMsg, int udpPort);
extern WiFiUDP udpListener;
extern int udpMsgLength;

// BLE interface
void initBLE();
extern BLECharacteristic *pCharacteristic;
extern BLECharacteristic *pCharacteristicNotify;
extern bool bleConnected;
extern String bleNewData;

// Touch interface
void initTouch();
extern Ticker touchTicker;

// Graphics functions
void drawIcon(const unsigned short* icon, int16_t x, int16_t y, int8_t width, int8_t height);

// MQTT functions / variables
void initMqtt();
bool connectMQTT();
bool addMqttMsg (String topic, String payload, bool retained);
extern MQTTClient mqttClient;

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

// Underground Weather & NTP time update functions / variables
bool initUGWeather();
void ugWeatherTask(void *pvParameters);

// Weather & NTP time update functions / variables
extern TaskHandle_t weatherTaskHandle;
extern Ticker weatherTicker;

// SPI interface
void initSPI();
void stopSPI();
uint32_t spiGetStatus();
void spiWriteData(uint8_t * data, size_t len);
void spiWriteData(const char * data);
void spiReadData(uint8_t * data);
String spiReadData();
