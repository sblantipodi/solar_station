/*
  SolarStation.h - Smart solar watering system 
  
  Copyright (C) 2020  Davide Perini
  
  Permission is hereby granted, free of charge, to any person obtaining a copy of 
  this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
  copies of the Software, and to permit persons to whom the Software is 
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in 
  all copies or substantial portions of the Software.
  
  You should have received a copy of the MIT License along with this program.  
  If not, see <https://opensource.org/licenses/MIT/>.

  * Components:
   - Arduino C++ sketch running on an ESP8266EX D1 Mini from Lolin running at 80MHz
   - Raspberry + Home Assistant for Web GUI, automations and MQTT server
   - 88x142 5V solar panel
   - Sony VCT6 18650 Lithium Battery
   - TP4056 protected lithium charger
   - MT3608 DC DC step up module to step up battery voltage to 5.5V, ESP chip is happy with it
   - MT3608 DC DC step up module to step up battery voltage to 8.66V, water pump is powerful with it
   - Relay Shield to safely power the water pump and "detach it from the circuit"
   - 100kΩ + 22kΩ + 4.4kΩ resistance for Battery voltage level monitoring circuit
   - 3.5/9V water pump (3W @ 9V)
   - TTP223 capacitive touch button with A contact soldered (HIGH signal when button is not pressed, 
     LOW signal when button is pressed), used to reset the microcontroller
   - Google Home Mini for Voice Recognition
   NOTE: GND of the battery MUST not be directly connected to the GND of the circuit.
*/

#include "Version.h"
#include "BootstrapManager.h"

/****************** BOOTSTRAP MANAGER ******************/
BootstrapManager bootstrapManager;
Helpers helper;

/**************************** PIN DEFINITIONS **************************************************/
#define OLED_RESET LED_BUILTIN // Pin used for integrated D1 Mini blue LED
#define ANALOG_IN_PIN A0  // ESP8266 Analog Pin ADC0 = A0
#define WATER_PUMP_PIN D1 // D1 Pin, water pump
#define WATER_LEAK_PIN D6 // Water leak pin
// NOTE: TP223 capacitive touch button is not registered because I don't manage it from sketch, it is only used to reset the microcontroller (or to wake it up from the deep sleed)

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
const int MQTT_PUBLISH_MAX_RETRY = 240; // max retry for MQTT publish
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
double espSleepTime = 600e6; // 15e6 = 15 seconds, 600e6 1 hour
int sensorValue = 0;  // analogRead to measure battery level via a voltage divider

bool hardCutOff = false;
bool waterPumpCutOff = true;

const int FORCE_DEEP_SLEEP_TIME = 900000; // force deepSleep after 15 minutes

// variable used for faster delay instead of arduino delay(), this custom delay prevent a lot of problem and memory leak
const int TENSECONDSPERIOD = 10000;
unsigned long timeNowStatus = 0;
unsigned long nowMillisWaterPumpStatus = 0; // used to turn off the pump after seconds
unsigned long nowMillisSendStatus = 0; // used to send status every second when the pump is on or when in upload mode
unsigned long nowMillisForceDeepSleepStatus = 0; // used to force deep sleep after 15 minutes

/********************************** FUNCTION DECLARATION (NEEDED BY PLATFORMIO WHILE COMPILING CPP FILES) *****************************************/
// Bootstrap functions
void callback(char* topic, byte* payload, unsigned int length);
void manageDisconnections();
void manageQueueSubscription();
void manageHardwareButton();
// Project specific functions
bool processMQTTConfig(StaticJsonDocument<BUFFER_SIZE> json);
bool processUploadModeJson(StaticJsonDocument<BUFFER_SIZE> json);
bool processWaterPumpActiveJson(StaticJsonDocument<BUFFER_SIZE> json);
bool processAckTopic(StaticJsonDocument<BUFFER_SIZE> json);
void sendWaterPumpActiveStateOff();
void sendWaterPumpPowerStateOff();
void sendWaterPumpPowerStateOn();
void readSensorData();
void sendOnState();
void sendOffState();
void sendOnOffState(String cmd, unsigned long stateMillis);
void espDeepSleep(bool sendState, bool hardCutOff);
void espDeepSleep(bool hardCutOff);
void sendSensorState();
void sendSensorStateNotTimed();
void turnOffWaterPumpAfterSeconds();
void sendSensorStateAfterSeconds(int delay);
int readAnalogBatteryLevel();
void forceDeepSleep();