/*
  SolarStation.cpp - Smart solar watering system 
  
 Copyright © 2020 - 2024  Davide Perini
  
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

#include <SolarStation.h>


/********************************** START SETUP*****************************************/
void setup() {

  // if fastDisconnectionManagement we need to execute the callback immediately,
  // example: power off a watering system can't wait MAX_RECONNECT attemps
  fastDisconnectionManagement = true;
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
  Serial.setTxTimeoutMs(0);
#endif
  Serial.begin(SERIAL_RATE);

  pinMode(WATER_PUMP_PIN, OUTPUT); // setup water pump pin
  digitalWrite(WATER_PUMP_PIN, LOW); // turn off the pump just in case

#if defined(ESP8266)
  pinMode(OLED_RESET, OUTPUT);  // setup built in ESP led
  digitalWrite(OLED_RESET, HIGH); // turn off the ESP led
#else
  pinMode(ANALOG_IN_PIN, INPUT); //It is necessary to declare the input pin
#endif

  // Bootsrap setup() with Wifi and MQTT functions
  bootstrapManager.bootstrapSetup(manageDisconnections, manageHardwareButton, callback);

#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
  // Turn off integrated LED
  turnOffBuiltInLed();
  // Use 12 bit width (analog reading up to 4095), with DB_11 attenuation (up to 3.1V) on ADC1_CHANNEL_1 (GPIO 2)
  // https://docs.espressif.com/projects/esp-idf/en/v4.4/esp32s3/api-reference/peripherals/adc.html
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_12);
#endif
  readAnalogBatteryLevel();

}

void turnOffBuiltInLed() {
#if CONFIG_IDF_TARGET_ESP32C3 || CONFIG_IDF_TARGET_ESP32S2 || CONFIG_IDF_TARGET_ESP32S3
    if (ledsEsp32 == NULL) {
        ledsEsp32 = new NeoPixelBus<NeoRgbFeature, NeoWs2812xMethod>(1, OLED_RESET);
        if (ledsEsp32 == nullptr) {
            Serial.println(F("OUT OF MEMORY"));
        }
        ledsEsp32->Begin();
    }
    ledsEsp32->SetPixelColor(0, {0, 0, 0});
    ledsEsp32->Show();
#endif
}

/********************************** MANAGE WIFI AND MQTT DISCONNECTION *****************************************/
void manageDisconnections() {

  // Shut down water pump on small disconnections
  if (wifiReconnectAttemp > 10 || mqttReconnectAttemp > 10) {
      digitalWrite(WATER_PUMP_PIN, LOW);      
  }
  // Shut down ESP on MAX_RECONNECT attemp
  if (wifiReconnectAttemp > MAX_RECONNECT || mqttReconnectAttemp > MAX_RECONNECT) {
      espDeepSleep(false, true);
  }

}

/********************************** MQTT SUBSCRIPTIONS *****************************************/
void manageQueueSubscription() {

  bootstrapManager.subscribe(SOLAR_STATION_UPLOADMODE_TOPIC);      
  bootstrapManager.subscribe(SOLAR_STATION_WATERPUMP_ACTIVE_TOPIC);      
  bootstrapManager.subscribe(SOLAR_STATION_MQTT_CONFIG, 1);      
  bootstrapManager.subscribe(SOLAR_STATION_MQTT_ACK);
      
}

/********************************** MANAGE HARDWARE BUTTON *****************************************/
void manageHardwareButton() {

  // no need to manage hardware button, TP223 is used to reboot the ESP and does not need code to do it

}

/********************************** START CALLBACK *****************************************/
void callback(char* topic, byte* payload, unsigned int length) {

  JsonDocument json = bootstrapManager.parseQueueMsg(topic, payload, length);

  if(strcmp(topic, SOLAR_STATION_MQTT_CONFIG) == 0) {
    processMQTTConfig(json);
  } else if(strcmp(topic, SOLAR_STATION_MQTT_ACK) == 0) {
    processAckTopic(json);
  } else if(strcmp(topic, SOLAR_STATION_UPLOADMODE_TOPIC) == 0) {
    processUploadModeJson(json);
  } 

}

/********************************** START PROCESS JSON *****************************************/
bool processMQTTConfig(JsonDocument json) {

  timedate = helper.getValue(json["time"]);

  String uploadModeStr = json["upload_mode"];
  uploadMode = uploadModeStr == on_CMD;
  if (uploadMode) {
    nowMillisSendStatus = millis(); // reset the counter, first ten seconds the blu led will be on
  }

  String waterPumpActiveStr = json["pump_active"];
  waterPumpActive = waterPumpActiveStr == on_CMD;

  // reconnection can cause mqttConfig to be reprocessed. don't reinitialize number of attemps
  if (!dataMQTTReceived) {
    if (waterPumpActive) {
      number_of_attemps = number_of_attemps - 6;
    } else {
      number_of_attemps = number_of_attemps - 4;
    }
  }
  
  String pumpSecondsStr = json["pump_seconds"];
  waterPumpSecondsOn = pumpSecondsStr.toDouble() * 1000;

  String espSleepTimeMinutesStr = json["esp_sleep_time_minutes"];
  
  // if sleepTime is 1 sleep 1 second, if it's 61 sleep forever, sleep N minutes otherwise
  if ((espSleepTimeMinutesStr.toDouble() >= 1)) {
    espSleepTime = (espSleepTimeMinutesStr.toDouble() * 60 * 1000000);
  } else {
    espSleepTime = (1e6);
  }
  if ((espSleepTimeMinutesStr.toDouble() >= 61)) {
    espSleepTime = 0; // 0 means sleep forever
  }

  // dataMQTTReceived enables sketch processing, we do our things only after MQTT config has been received
  dataMQTTReceived = true;

  // Reset the millis used for water pump activity
  nowMillisWaterPumpStatus = millis();
  // Reset the millis used for send status activity
  nowMillisSendStatus = millis();
  // Reset the millis used for force deep sleep after 15 minutes
  nowMillisForceDeepSleepStatus = millis();
  // Reset the millis used for MQTT QoS1
  onStateNowMillis = millis();
  offStateNowMillis = millis();
  waterPumpPowerStateOnNowMillis = millis();
  waterPumpPowerStateOffNowMillis = millis();
  sensorStateNowMillis = millis();
  waterPumpActiveStateOffNowMillis = millis();

  // if battery analog level is below 3700 (3.7V) the microcontroller can continue to wake up and sleep but it can't turn on the water pump
  int batteryVoltage = readAnalogBatteryLevel();
  waterPumpCutOff = (batteryVoltage < WATER_PUMP_CUTOFF);
  // if battery analog level is below 3500 (3.5V) hard cut off
  espCutOff = (batteryVoltage < ESP_CUTOFF);
  return true;

}

// ACK Automation for custom implementation of QoS1 in pubsubclient that doesn't support QoS1 for publish if not for subscribe
bool processAckTopic(JsonDocument json) {

    String ackMsg = json[VALUE];

    if (ackMsg == "sendOnState") {
      onStateAckReceived = true;
    } else if (ackMsg == "sendOffState") {
      offStateAckReceived = true;
    } else if (ackMsg == "sendWaterPumpPowerStateOff") {
      waterPumpPowerStateOffAckReceived = true;
    } else if (ackMsg == "sendWaterPumpPowerStateOn") {
      waterPumpPowerStateOnAckReceived = true;
      waterPumpPower = true;
      // This runs until the pump is off if the batt voltage is greater than 3.6V
      digitalWrite(WATER_PUMP_PIN, HIGH); // PHISICALLY TURN ON THE PUMP!!!
      // Turning on water pump, resetting the millis counter used to turn it off
      nowMillisWaterPumpStatus = millis();
    } else if (ackMsg == "sendSensorState") {
      sensorStateAckReceived = true;
    } else if (ackMsg == "sendWaterPumpActiveStateOff") {
      waterPumpActiveStateOffAckReceived = true;
    }
    
    return true;

}

bool processUploadModeJson(JsonDocument json) {

    uploadMode = (helper.isOnOff(json) == ON_CMD);
    Serial.println();
    Serial.print("UPLOAD_MODE= "); Serial.println(uploadMode);    
    Serial.println();
    return true;

}

/********************************** SEND STATE *****************************************/
void sendSensorState() {

  if(millis() > sensorStateNowMillis + DELAY_1000){
    sensorStateNowMillis = millis();
    number_of_attemps++;
    sendSensorStateNotTimed();
  }

}
void sendSensorStateNotTimed() {  

  JsonObject root = bootstrapManager.getJsonObject();

  // read analog battery level 
  int batteryLevelAnalog = readAnalogBatteryLevel();

  // if the analog value is below 3500 (3.5V) execute an hard cutoff
  if (espCutOff) {
    hardCutOff = true;
    root["HARD_CUT_OFF"] = true;
  } else {
    root["HARD_CUT_OFF"] = false;
  }
  if (waterPumpCutOff) {
    root["WATER_PUMP_CUT_OFF"] = true;
  } else {
    root["WATER_PUMP_CUT_OFF"] = false;
  }
  if (batteryLevelOnboot == -1) {
    root["battery_level_on_boot"] = batteryLevelAnalog;
    batteryLevelOnboot = batteryLevelAnalog;
  }
  root["battery"] = batteryLevelAnalog;
  root["frequency"] = ESP.getCpuFreqMHz();  
  
  bootstrapManager.sendState(SOLAR_STATION_STATE_TOPIC, root, VERSION); 

  delay(DELAY_10);

  // hardCutOff but only if uploadMode is false
  if (hardCutOff && !uploadMode) {
    espDeepSleep(true, true);
  }

}

void sendSensorStateAfterSeconds(int delay) {

  if(millis() > nowMillisSendStatus + delay){
    nowMillisSendStatus = millis();
    sendSensorStateNotTimed();
  }

}

void sendOnState() {    

  if(millis() > onStateNowMillis + DELAY_1000){
    sendOnOffState(ON_CMD, onStateNowMillis);     
  }

}

void sendOffState() {

  if(millis() > offStateNowMillis + DELAY_1000){
    sendOnOffState(OFF_CMD, offStateNowMillis);
  }
  
}

void sendOnOffState(String cmd, unsigned long stateMillis) {
  
  number_of_attemps++;
  Serial.println("SENDING STATE"); Serial.println(cmd);

  JsonObject root = bootstrapManager.getJsonObject();

  root["state"] = cmd;
  if (cmd == OFF_CMD) {
    offStateNowMillis = millis();
    root["number_of_attemps"] = number_of_attemps;
  } else {
    onStateNowMillis = millis();
    root["number_of_attemps"] = 0;
  }

  bootstrapManager.publish(SOLAR_STATION_POWER_TOPIC, root, false);

  delay(DELAY_10);

}

void sendWaterPumpPowerStateOff() {

  if(millis() > waterPumpPowerStateOffNowMillis + DELAY_1000){
    waterPumpPowerStateOffNowMillis = millis();
    number_of_attemps++;
    Serial.println(); 
    Serial.print("SENDING WATER PUMP POWER STATE OFF");
    bootstrapManager.publish(SOLAR_STATION_WATERPUMP_POWER_TOPIC, helper.string2char(OFF_CMD), false);
    delay(DELAY_10);
  }

}

void sendWaterPumpPowerStateOn() {

  if(millis() > waterPumpPowerStateOnNowMillis + DELAY_1000){
    waterPumpPowerStateOnNowMillis = millis();
    number_of_attemps++;
    Serial.println(); 
    Serial.print("SENDING WATER PUMP POWER STATE ON"); 
    bootstrapManager.publish(SOLAR_STATION_WATERPUMP_POWER_TOPIC, helper.string2char(ON_CMD), false);
    delay(DELAY_10);
  }

}

void sendWaterPumpActiveStateOff() {

  if(millis() > waterPumpActiveStateOffNowMillis + DELAY_1000){
    waterPumpActiveStateOffNowMillis = millis();
    number_of_attemps++;
    Serial.println(); 
    Serial.println("SENDING WATER PUMP ACTIVE STATE OFF"); 
    bootstrapManager.publish(SOLAR_STATION_WATERPUMP_ACTIVE_STAT_TOPIC, helper.string2char(OFF_CMD), false);
    delay(DELAY_10);
  }

}

/********************************** WATER PUMP MANAGEMENT (non blocking delay) *****************************************/
void turnOffWaterPumpAfterSeconds() {

  if(millis() > nowMillisWaterPumpStatus + waterPumpSecondsOn){
    nowMillisWaterPumpStatus = millis();
    waterPumpPower = false;
    digitalWrite(WATER_PUMP_PIN, LOW); // PHISICALLY TURN OFF THE WATER PUMP
  }

}

/********************************** ESP DEEP SLEEP *****************************************/
// Note: to achieve timed deepSleep you need to connect D0 with RST pin
void espDeepSleep(bool sendState, bool hardCutOff) {

  // if sendState is required, shutdown the microcontroller after the offStateAckReceived only
  if (sendState && !offStateAckReceived) {
    sendOffState();
  }
  if (sendState && offStateAckReceived) {
    espDeepSleep(hardCutOff); 
  }
  // if sendState is not required, MQTT or Wifi not available for example, shutdown microcontroller
  if (!sendState) {
    espDeepSleep(hardCutOff);
  }

}
void espDeepSleep(bool hardCutOff) {
  dataMQTTReceived = false;
  delay(DELAY_1000);
#if CONFIG_IDF_TARGET_ESP32S3
  // Turn OFF integrated LED before setting the ESP to sleep
  turnOffBuiltInLed();
#endif
  // if hardCutOff sleep forever or until capacitive button is pressed
    if (hardCutOff || espSleepTime == 0) {
#if defined(ESP8266)
    ESP.deepSleep(0);
#else
      esp_deep_sleep_start();
#endif
  } else {
    ESP.deepSleep(espSleepTime);
  }

}
// force deepSleep after 15 minutes
void forceDeepSleep() {

  if(millis() > nowMillisForceDeepSleepStatus + FORCE_DEEP_SLEEP_TIME){
    nowMillisForceDeepSleepStatus = millis();
    digitalWrite(WATER_PUMP_PIN, LOW);
    espDeepSleep(false, true);
  }

}

/********************************** BATTERY LEVEL *****************************************/
// read analog battery level from A0 pin (voltage divider required on that PIN because ESP can read up to 3.3V and the battery goes up to 4.2V)
int readAnalogBatteryLevel() {

  return analogRead(ANALOG_IN_PIN);

}

/********************************** START MAIN LOOP *****************************************/
void loop() {  
  
  // Bootsrap loop() with Wifi, MQTT and OTA functions
  bootstrapManager.bootstrapLoop(manageDisconnections, manageQueueSubscription, manageHardwareButton);

  // Send status on startup and wait for MQTT config, this is subscribed with QoS1 so MQTT server will retry until received
  if (!onStateAckReceived || !dataMQTTReceived) {
    sendOnState();
  }
 
  // MQTT config received, job start (MQTT Config sent via HA in QoS1)
  if (dataMQTTReceived && onStateAckReceived) {
    if (uploadMode) {
      digitalWrite(WATER_PUMP_PIN, LOW);
      sendSensorStateAfterSeconds(TENSECONDSPERIOD); // this sendState does not wait for an ack    
    } else {
      // if battery analog level is below 816 (3.3V) the microcontroller can continue to wake up and sleep but it can't turn on the water pump
      if (waterPumpActive && !waterPumpCutOff) {      
        // Water pump active, if the pump was off, send sensor state and pump state for the first time              
        if (!sensorStateAckReceived) {
          sendSensorState();
        } else {
          if (!waterPumpPowerStateOnAckReceived) {            
            sendWaterPumpPowerStateOn();                                             
          } else {
            if (waterPumpPower) {
              turnOffWaterPumpAfterSeconds(); // Pump is turned off as soon as the delay is reached, no matter for ACK from the MQTT server
              // continue to send state every seconds when the pump is turned on
              sendSensorStateAfterSeconds(DELAY_1000); // this sendState does not wait for an ack    
            } else {
              if (!waterPumpPowerStateOffAckReceived) {
                sendWaterPumpPowerStateOff(); // Update the MQTT server with off state of the pump when it is possible
              } else {
                if (!waterPumpActiveStateOffAckReceived) {
                  sendWaterPumpActiveStateOff();
                } else {
                  espDeepSleep(true, false);
                  // Pump is turned off, microcontroller is finally sleeping
                }
              }                            
            }          
          }                          
        }         
      } else {
        // Water pump was not active, send state and sleep until next
        digitalWrite(WATER_PUMP_PIN, LOW);
        if (!waterPumpPowerStateOffAckReceived) {
          sendWaterPumpPowerStateOff();
        } else {
          if (!sensorStateAckReceived) {
            sendSensorState();
          } else {
            espDeepSleep(true, false);
          }          
        }        
      }
    }
  }

  // Force deepSleep after 15 minutes of activity, no matter what's happening, some errors occured probably.
  forceDeepSleep();

  delay(DELAY_10);
  
}