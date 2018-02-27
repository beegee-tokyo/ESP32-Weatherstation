/** mDNS and Access point name */
extern char apName[];

/** OTA update status */
extern bool otaRunning;

/** TFT_eSPI class for display */
extern TFT_eSPI tft;

/**********************************************************/
// Function declarations
/**********************************************************/
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
extern String debugLabel;
extern String infoLabel;
extern String errorLabel;
extern bool debugOn;

// WiFi utilities
void createName(char *apName, int apIndex);
bool connDirect(const char *ssid, const char *password, uint32_t timeout);
bool initNTP();
bool tryGetTime();
String digitalTimeDisplay();
String digitalTimeDisplaySec();

// Weather interface
extern HTTPClient http;
extern TaskHandle_t weatherTaskHandle;
extern Ticker weatherTicker;

// UDP interface
void getUDPbroadcast(int udpMsgLength);
bool udpSendMessage(IPAddress ipAddr, String udpMsg, int udpPort);
extern WiFiUDP udpListener;
extern int udpMsgLength;

// TCP server interface
void getTCPPacket(WiFiClient tcpClient);
extern WiFiServer tcpServer;

// BLE Server interface
void initBlueTooth(byte which);
void stopBLE();
void restartBLE();
extern BLEServer *pServer;
extern uint8_t digitalOut;
extern bool digOutChanged;

// BT serial interface
extern BluetoothSerial SerialBT;
bool initBtSerial();
bool sendBtSerial(String message);
void stopBtSerial();

// Debug and system stuff
void sendDebug(String topic, String payload, bool retained);
void printPartitions();
void printLastResetReason();

// BLE Client interface
// void scanBLEdevices();
// bool connectToServer(BLEAddress pAddress);
// extern bool doConnect;
// extern bool connected;
// extern bool isScanning;
// extern BLEAddress *pServerAddress;
// extern BLEClient* pClient;
// extern BLERemoteCharacteristic* pRemoteCharacteristic;

// Touch interface
void initTouch();
void disableTouch();
extern Ticker touchTickerPad1;
extern Ticker touchTickerPad2;
extern Ticker touchTickerPad3;
extern bool shortTouchPad1;
extern bool longTouchPad1;
extern bool shortTouchPad2;
extern bool longTouchPad2;
extern bool shortTouchPad3;
extern bool longTouchPad3;

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
void stopLight();
extern long newLDRValue;
extern int newTSLValue;
extern TaskHandle_t lightTaskHandle;
extern Ticker lightTicker;

// Temperature functions / variables
bool initTemp();
void stopTemp();
extern TaskHandle_t tempTaskHandle;
extern Ticker tempTicker;

// Underground Weather & NTP time update functions / variables
bool initUGWeather();
void stopUGWeather();
void ugWeatherTask(void *pvParameters);
