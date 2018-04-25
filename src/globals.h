/**********************************************************/
/**********************************************************/
// Global function and variable declarations
/**********************************************************/
/**********************************************************/

// WiFi utilities
// Preferences
#define PREF_NAME "WiFiCred"
#define SSID_PRIM_KEY "ssidPrim"
#define SSID_SEC_KEY "ssidSec"
#define PW_PRIM_KEY "pwPrim"
#define PW_SEC_KEY "pwSec"
#define VALID_KEY "valid"
#define DEV_ID_KEY "devID"
#define DEV_LOC_KEY "devLoc"
#define DEV_TYPE_KEY "devType"

void connectInit();
void connectWiFi();
bool scanWiFi();
void activateOTA(const char *MODULTYPE, const char *MODULID);
void createName();
bool initNTP();
bool tryGetTime();
String digitalTimeDisplay();
String digitalTimeDisplaySec();
extern WiFiMulti wifiMulti;
extern IPAddress multiIP;
extern String ssidPrim;
extern String ssidSec;
extern String pwPrim;
extern String pwSec;
extern String devID;
extern String devType;
extern String devLoc;
extern volatile byte connStatus;
extern bool usePrimAP;
extern unsigned long wifiConnectStart;
extern char apName[];
extern int apIndex;
extern bool otaRunning;
extern bool otaInitDone;

/** Connection status */
#define CON_INIT   0 // connection initialized
#define CON_START  1 // connecting
#define CON_GOTIP	 2 // connected with IP
#define CON_LOST   3 // disconnected

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
void initBLE();
void stopBLE();
// void initBlueTooth(byte which);
// void stopBLE();
// void restartBLE();
// void reStartBtSerial();
extern BLEServer *pServer;
// extern uint8_t digitalOut;
// extern bool digOutChanged;

// BT serial interface
extern BluetoothSerial SerialBT;
bool initBtSerial();
bool sendBtSerial(String message);
void stopBtSerial();

// Debug and system stuff
void sendDebug(String topic, String payload, bool retained);
void printPartitions();
void printLastResetReason();
String reset_reason(RESET_REASON reason);
extern bool tasksEnabled;
extern String debugLabel;
extern String infoLabel;
extern String errorLabel;
extern bool debugOn;

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

// TFT display functions / variables
void drawIcon(const unsigned short* icon, int16_t x, int16_t y, int8_t width, int8_t height);
extern TFT_eSPI tft;

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
extern uint8_t	ledPin;

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
