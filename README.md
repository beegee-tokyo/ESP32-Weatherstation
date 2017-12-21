# ESP32 Weatherstation
ESP32 based Weatherstation    
Uses attached sensors, remote sensors and Internet weather information to display current weather on a 1.44" TFT display.    

## Development platform    
PlatformIO

## Used hardware    
- [Elecrow ESP32 WIFI BLE BOARD WROOM](https://circuit.rocks/esp32-wifi-ble-board-wroom.html?search=ESP32)    
- [Elecrow 1.44" TFT display](https://circuit.rocks/displays/tft-display-128x128)    
- [Adafruit TSL2561 LUMINOSITY/LUX/LIGHT DIGITAL SENSOR BREAKOUT](https://circuit.rocks/luminosity-lux-light-adafruit-tsl2561-digital-sensor-breakout?search=TSL2561)    
- [Elecrow DHT11 TEMPERATURE AND HUMIDITY SENSOR](https://circuit.rocks/temperature-and-humidity-dht11-sensor.html?search=DHT11)    
- [LDR](http://ph.rs-online.com/web/p/ldr-light-dependent-resistors/9146710/)    
- 10k and 2.2k resistor   
- ESP8266 security module with PIR, light relay, alarm buzzer and DHT11 temperature and humidity sensor (optional, another DIY project)    

## SW practices used   
- Use of ESP32 HW timer    
- Use of SPI    
- Use of bit-banging 1 wire interface to DHT11    
- Use of ESP32 touch interface    
- Use of ESP32 multi tasking    
- Use of ArduinoOTA for SW updates     
- Use of MQTT for web based information display
- Use of UDP broadcast messaging
- Use of UDP broadcast receiving    
- Use of BLE for sending sensor data    

## Library dependencies    
PlatformIO library ID - Library name / Github link
- ID34 [Adafruit TSL2561 by Adafruit Industries](https://github.com/adafruit/Adafruit_TSL2561)    
- [zhouhan0126 WIFIMANAGER-ESP32](https://github.com/zhouhan0126/WIFIMANAGER-ESP32)    
- [zhouhan0126 WebServer-esp32](https://github.com/zhouhan0126/WebServer-esp32)    
- [zhouhan0126 DNSServer---esp32](https://github.com/zhouhan0126/DNSServer---esp32)    
- [MQTT by Joel Gaehwiler](https://github.com/256dpi/arduino-mqtt)    
- [esp8266FTPServer by David Paiva](https://github.com/nailbuster/esp8266FTPServer)    
- ID64 [ArduinoJson by Benoit Blanchon](https://github.com/bblanchon/ArduinoJson)    
- ID2029 [DHTesp by Bernd Giesecke](https://github.com/beegee-tokyo/DHTesp)    
- ID1559 [TFT_eSPI by Bodmer](https://github.com/Bodmer/TFT_eSPI)    
- [ESP32-MyLib by Bernd Giesecke](https://github.com/beegee-tokyo/ESP32-MyLib)    
- ID1841 [ESP32 BLE Arduino by Neil Kolban](https://github.com/nkolban/ESP32_BLE_Arduino)    
