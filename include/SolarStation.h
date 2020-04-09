#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <SPI.h>
#include <Wire.h>
#include <Secrets.h>
#include <Version.h>


/****************** WIFI and MQTT INFO ******************/
// MQTT server port
const int mqtt_port = 1883;
// DNS address for the microcontroller:
IPAddress mydns(192, 168, 1, 1);
// GATEWAY address for the microcontroller:
IPAddress mygateway(192, 168, 1, 1);

/****************** OTA ******************/
#ifdef TARGET_SOLAR_WS
  // SENSORNAME will be used as device network name
  #define SENSORNAME "solar_station"
  // Port for the OTA firmware uplaod
  int OTAport = 8290;
  // Static IP address for the microcontroller:
  IPAddress arduinoip(192, 168, 1, 59); 
#endif 

/**************************** PIN DEFINITIONS **************************************************/
#define OLED_RESET LED_BUILTIN // Pin used for integrated D1 Mini blue LED
#define ANALOG_IN_PIN A0  // ESP8266 Analog Pin ADC0 = A0
#define WATER_PUMP_PIN D1 // D5 Pin, 5V power, for capactitive touch sensor. When Sig Output is high, touch sensor is being 
#define WATER_LEAK_PIN D6 // Water leak pin

// Serial rate for debug
#define serialRate 115200


/************* MQTT TOPICS **************************/
// subscribe
const char* SOLAR_STATION_UPLOADMODE_TOPIC = "cmnd/upload_mode/SLEEP";
const char* SOLAR_STATION_WATERPUMP_ACTIVE_TOPIC = "cmnd/water_pump/ACTIVE";
const char* SOLAR_STATION_MQTT_CONFIG = "stat/solarstation/CONFIG";
// publish
const char* SOLAR_STATION_STATE_TOPIC = "tele/solarstation/STATE";
const char* SOLAR_STATION_WATERPUMP_ACTIVE_STAT_TOPIC = "stat/water_pump/ACTIVE";
const char* SOLAR_STATION_WATERPUMP_POWER_TOPIC = "stat/water_pump/POWER";
const char* SOLAR_STATION_POWER_TOPIC = "stat/solarstation/POWER";
const char* SOLAR_STATION_MQTT_ACK = "stat/solarstation/ACK";

/****************** GLOBAL VARS ******************/
// MQTT publish retry until ack received
const int MQTT_PUBLISH_MAX_RETRY = 60; // max retry for MQTT publish
int number_of_attemps = 0;
bool onStateAckReceived = false;
unsigned long onStateNowMillis = 0; 
bool offStateAckReceived = false;
unsigned long offStateNowMillis = 0; 
bool waterPumpPowerStateOnAckReceived = false;
unsigned long waterPumpPowerStateOnNowMillis = 0;
bool waterPumpPowerStateOffAckReceived = false;
unsigned long waterPumpPowerStateOffNowMillis = 0;
bool sensorStateAckReceived = false;
unsigned long sensorStateNowMillis = 0;
bool waterPumpActiveStateOffAckReceived = false;
unsigned long waterPumpActiveStateOffNowMillis = 0;

int blinked = 10;
float temperature = 0;
float pressure = 0;
float humidity = 0;
bool contextInitialized = false;
bool uploadMode = true; // if true, microcontroller does nothing but expose its OTA programmable interface
bool waterPumpActive = false; // value received via MQTT config, if true, turn on the pump
bool waterPumpPower = false; // value send via MQTT message to the broker, if true, the pump if turned on
bool dataMQTTReceived = false;  // don't do anything until MQTT server sent its configuration
int waterPumpSecondsOn = 10000; // default 10 seconds, it changes after config received via MQTT message 
int waterPumpRemainingSeconds = 10;
double espSleepTime = 600e6; // 15e6 = 15 seconds
int sensorValue = 0;  // analogRead to measure battery level via a voltage divider
// LED_BUILTIN vars
unsigned long previousMillis = 0;    // will store last time LED was updated
const long interval = 200;           // interval at which to blink (milliseconds)
bool ledTriggered = false;
const int blinkTimes = 6;            // 6 equals to 3 blink on and 3 off
int blinkCounter = 0;
bool hardCutOff = false;
bool waterPumpCutOff = true;
// MQTT cmnd
const char* ON_CMD = "ON";
const char* OFF_CMD = "OFF";

String timedate = "OFF";
const int delay_10 = 10;
const int delay_50 = 50;
const int delay_500 = 500;
const int delay_1000 = 1000;
const int delay_1500 = 1500;
const int delay_3000 = 3000;
const int delay_2000 = 2000;
const int delay_4000 = 4000;
const int delay_5000 = 5000;
const int FORCE_DEEP_SLEEP_TIME = 900000; // force deepSleep after 15 minutes

// variable used for faster delay instead of arduino delay(), this custom delay prevent a lot of problem and memory leak
const int tenSecondsPeriod = 10000;
unsigned long timeNowStatus = 0;
unsigned long nowMillisWaterPumpStatus = 0; // used to turn off the pump after seconds
unsigned long nowMillisStatusWithPumpOn = 0; // used to send status every second when the pump is on
unsigned long nowMillisForceDeepSleepStatus = 0; // used to force deep sleep after 15 minutes
#define MAX_RECONNECT 500
unsigned int delayTime = 10;

/**************************************** FOR JSON ***************************************/
const int BUFFER_SIZE = JSON_OBJECT_SIZE(20);

WiFiClient espClient;
PubSubClient client(espClient);

/********************************** FUNCTION DECLARATION (NEEDED BY VSCODE WHILE COMPILING CPP FILES) *****************************************/
bool processMQTTConfig(char *message);
bool processUploadModeJson(char *message);
bool processWaterPumpActiveJson(char *message);
bool processAckTopic(char *message);
void sendWaterPumpActiveStateOff();
void sendWaterPumpPowerStateOff();
void sendWaterPumpPowerStateOn();
void readSensorData();
void sendOnState();
void sendOffState();
void sendOnOffState(const char *cmd, unsigned long stateMillis);
void espDeepSleep(bool sendState, bool hardCutOff);
void espDeepSleep(bool hardCutOff);
void sendSensorState();
void sendSensorStateNotTimed();
void turnOffWaterPumpAfterSeconds();
int getQuality();
void setup_wifi();
void callback(char* topic, byte* payload, unsigned int length);
void sendSensorStateAfterSeconds(int delay);
int readAnalogBatteryLevel();
void forceDeepSleep();