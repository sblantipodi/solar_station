/*
  SolarStation.cpp - Smart solar watering system 
  
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

#include <SolarStation.h>


/********************************** START SETUP*****************************************/
void setup() {
  
  Serial.begin(SERIAL_RATE);

  pinMode(WATER_PUMP_PIN, OUTPUT); // setup water pump pin
  digitalWrite(WATER_PUMP_PIN, LOW); // turn off the pump just in case
  pinMode(LED_BUILTIN, OUTPUT);  // setup built in ESP led
  digitalWrite(LED_BUILTIN, HIGH); // turn off the ESP led
  
  // Bootsrap setup() with Wifi and MQTT functions
  bootstrapManager.bootstrapSetup(manageDisconnections, manageHardwareButton, callback);

}

/********************************** MANAGE WIFI AND MQTT DISCONNECTION *****************************************/
void manageDisconnections() {

  // if fastDisconnectionManagement we need to execute the callback immediately, 
  // example: power off a watering system can't wait MAX_RECONNECT attemps
  fastDisconnectionManagement = true;

  // Shut down water pump on small disconnections
  if (wifiReconnectAttemp > 10 || mqttReconnectAttemp > 10) {
      digitalWrite(WATER_PUMP_PIN, LOW);      
  }
  // Shut down ESP on MAX_RECONNECT attemp
  if (wifiReconnectAttemp > MAX_RECONNECT || mqttReconnectAttemp > MAX_RECONNECT) {
    espDeepSleep(false, false);
  }

}

/********************************** MQTT SUBSCRIPTIONS *****************************************/
void manageQueueSubscription() {

  mqttClient.subscribe(SOLAR_STATION_UPLOADMODE_TOPIC);      
  mqttClient.subscribe(SOLAR_STATION_WATERPUMP_ACTIVE_TOPIC);      
  mqttClient.subscribe(SOLAR_STATION_MQTT_CONFIG);      
  mqttClient.subscribe(SOLAR_STATION_MQTT_ACK);
      
}

/********************************** MANAGE HARDWARE BUTTON *****************************************/
void manageHardwareButton() {
  // no need to manage hardware button, TP223 is used to reboot the ESP and does not need code to do it
}

/********************************** START CALLBACK *****************************************/
void callback(char* topic, byte* payload, unsigned int length) {

  char message[length + 1];
  for (unsigned int i = 0; i < length; i++) {
    message[i] = (char)payload[i];
  }
  message[length] = '\0';

  if(strcmp(topic, SOLAR_STATION_MQTT_CONFIG) == 0) {
    if (!processMQTTConfig(message)) {
      return;
    }
  } else if(strcmp(topic, SOLAR_STATION_MQTT_ACK) == 0) {
    if (!processAckTopic(message)) {
      return;
    }
  } else if(strcmp(topic, SOLAR_STATION_UPLOADMODE_TOPIC) == 0) {
    if (!processUploadModeJson(message)) {
      return;
    }
  } else if(strcmp(topic, SOLAR_STATION_WATERPUMP_ACTIVE_TOPIC) == 0) {
    if (!processWaterPumpActiveJson(message)) {
      return;
    }
  }

}

/********************************** START PROCESS JSON *****************************************/
bool processMQTTConfig(char* message) {
  StaticJsonDocument<BUFFER_SIZE> doc;
  DeserializationError error = deserializeJson(doc, message);

  if (error) {
    Serial.println(F("parseObject() failed 2"));
    return false;
  }

  const char* timeConst = doc["time"];
  timedate = timeConst;  

  const char* uploadModeConst = doc["upload_mode"];
  String uploadModeStr = uploadModeConst;
  uploadMode = uploadModeStr == "on";
  if (uploadMode) {
    nowMillisSendStatus = millis(); // reset the counter, first ten seconds the blu led will be on
  }
  
  const char* waterPumpActiveConst = doc["pump_active"];
  String waterPumpActiveStr = waterPumpActiveConst;
  waterPumpActive = waterPumpActiveStr == "on";

  // reconnection can cause mqttConfig to be reprocessed. don't reinitialize number of attemps
  if (!dataMQTTReceived) {
    if (waterPumpActive) {
      number_of_attemps = number_of_attemps - 6;
    } else {
      number_of_attemps = number_of_attemps - 4;
    }
  }
  
  const char* pumpSecondsConst = doc["pump_seconds"];
  String pumpSecondsStr = pumpSecondsConst;
  waterPumpSecondsOn = pumpSecondsStr.toDouble() * 1000;

  const char* espSleepTimeMinutesConst = doc["esp_sleep_time_minutes"];
  String espSleepTimeMinutesStr = espSleepTimeMinutesConst;
  
  // if sleepTime is 1 sleep 1 second, if it's 61 sleep forever, sleep N minutes otherwise
  if ((espSleepTimeMinutesStr.toDouble() >= 1)) {
    espSleepTime = (espSleepTimeMinutesStr.toDouble() * 60 * 1000000);
  } else if ((espSleepTimeMinutesStr.toDouble() >= 61)) {
    espSleepTime = 0; // 0 means sleep forever
  } else {
    espSleepTime = (1e6);
  }

  Serial.println("MQTT CONFIG RECEIVED");
  serializeJsonPretty(doc, Serial);

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

  // Remaining seconds
  waterPumpRemainingSeconds = (waterPumpSecondsOn / 1000);
  // if battery analog level is below 890 (3.6V) the microcontroller can continue to wake up and sleep but it can't turn on the water pump
  waterPumpCutOff = (readAnalogBatteryLevel() < 890); 
  
  return true;
}

// ACK Automation for custom implementation of QoS1 in pubsubclient that doesn't support QoS1 for publish if not for subscribe
bool processAckTopic(char* message) {
    String ackMsg = message;

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

bool processUploadModeJson(char* message) {
    String uploadModeStr = message;
    uploadMode = uploadModeStr == ON_CMD;
    Serial.println();
    Serial.print("UPLOAD_MODE= "); Serial.println(uploadMode);    
    Serial.println();
    return true;
}

bool processWaterPumpActiveJson(char* message) {
    String waterPumpStr = message;
    waterPumpActive = waterPumpStr == ON_CMD;
    Serial.println();
    Serial.print("WATER_PUMP_ACTIVE= "); Serial.println(waterPumpActive);    
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
    StaticJsonDocument<BUFFER_SIZE> doc;
    JsonObject root = doc.to<JsonObject>();

    root["Whoami"] = WIFI_DEVICE_NAME;
    root["IP"] = WiFi.localIP().toString();
    root["MAC"] = WiFi.macAddress();
    root["ver"] = VERSION;

    root["time"] = timedate;

    // read analog battery level 
    int batteryLevelAnalog = readAnalogBatteryLevel();

    // if the analog value is below 816 (3.3V) execute an hard cutoff
    if (batteryLevelAnalog <= 816) {
      hardCutOff = true;
      root["HARD_CUT_OFF"] = batteryLevelAnalog;
    }
    if (waterPumpCutOff) {
      root["WATER_PUMP_CUT_OFF"] = batteryLevelAnalog;
    }
    root["battery"] = batteryLevelAnalog;
    root["wifi"] = bootstrapManager.getSignalQuality();
    root["frequency"] = ESP.getCpuFreqMHz();  
    root["remaining_seconds"] = waterPumpRemainingSeconds;  
    
    
    char buffer[measureJson(root) + 1];
    serializeJson(root, buffer, sizeof(buffer));

    Serial.println();
    Serial.println("SENDING SENSOR STATE");
    serializeJsonPretty(root, Serial);

    // This topic should be retained, we don't want unknown values on battery voltage or wifi signal
    mqttClient.publish(SOLAR_STATION_STATE_TOPIC, buffer, true);

    delay(DELAY_10);

    // hardCutOff but only if uploadMode is false
    if (hardCutOff && !uploadMode) {
      espDeepSleep(true, true);
    }
}

void sendSensorStateAfterSeconds(int delay) {
  if(millis() > nowMillisSendStatus + delay){
    nowMillisSendStatus = millis();
    if (!uploadMode) {
      waterPumpRemainingSeconds = (waterPumpRemainingSeconds-(delay/1000));
    }    
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

void sendOnOffState(const char *cmd, unsigned long stateMillis) {
  number_of_attemps++;
  Serial.println("SENDING STATE"); Serial.println(cmd);

  StaticJsonDocument<BUFFER_SIZE> doc;
  JsonObject root = doc.to<JsonObject>();
  root["state"] = cmd;
  if (cmd == OFF_CMD) {
    offStateNowMillis = millis();
    root["number_of_attemps"] = number_of_attemps;
  } else {
    onStateNowMillis = millis();
    root["number_of_attemps"] = 0;
  }
  char buffer[measureJson(root) + 1];
  serializeJson(root, buffer, sizeof(buffer));
  serializeJsonPretty(root, Serial);
  mqttClient.publish(SOLAR_STATION_POWER_TOPIC, buffer, false);

  delay(DELAY_10);
}

void sendWaterPumpPowerStateOff() {
  if(millis() > waterPumpPowerStateOffNowMillis + DELAY_1000){
    waterPumpPowerStateOffNowMillis = millis();
    number_of_attemps++;
    Serial.println(); 
    Serial.print("SENDING WATER PUMP POWER STATE OFF");
    mqttClient.publish(SOLAR_STATION_WATERPUMP_POWER_TOPIC, OFF_CMD, false);
    delay(DELAY_10);
  }
}

void sendWaterPumpPowerStateOn() {
  if(millis() > waterPumpPowerStateOnNowMillis + DELAY_1000){
    waterPumpPowerStateOnNowMillis = millis();
    number_of_attemps++;
    Serial.println(); 
    Serial.print("SENDING WATER PUMP POWER STATE ON"); 
    mqttClient.publish(SOLAR_STATION_WATERPUMP_POWER_TOPIC, ON_CMD, false);
    delay(DELAY_10);
  }
}

void sendWaterPumpActiveStateOff() {
  if(millis() > waterPumpActiveStateOffNowMillis + DELAY_1000){
    waterPumpActiveStateOffNowMillis = millis();
    number_of_attemps++;
    Serial.println(); 
    Serial.println("SENDING WATER PUMP ACTIVE STATE OFF"); 
    mqttClient.publish(SOLAR_STATION_WATERPUMP_ACTIVE_STAT_TOPIC, OFF_CMD, false);
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
  // if hardCutOff sleep forever or until capacitive button is pressed
  if (hardCutOff) {
    ESP.deepSleep(0);
  } else {
    ESP.deepSleep(espSleepTime);
  }
}
// force deepSleep after 15 minutes
void forceDeepSleep() {
  if((millis() > nowMillisForceDeepSleepStatus + FORCE_DEEP_SLEEP_TIME) || (number_of_attemps >= MQTT_PUBLISH_MAX_RETRY)){
    nowMillisForceDeepSleepStatus = millis();
    digitalWrite(WATER_PUMP_PIN, LOW);
    espDeepSleep(false, false);
  }
}

/********************************** BATTERY LEVEL *****************************************/
// read analog battery level from A0 pin (voltage divider required on that PIN because ESP can read up to 3.3V and the battery goes up to 4.2V)
int readAnalogBatteryLevel() {
  return analogRead(A0);
}

/********************************** START MAIN LOOP *****************************************/
void loop() {  
  
  // Bootsrap loop() with Wifi, MQTT and OTA functions
  bootstrapManager.bootstrapLoop(manageDisconnections, manageQueueSubscription, manageHardwareButton);

  // Send status on startup and wait for MQTT config
  if (!onStateAckReceived) {
    sendOnState();
  }
 
  // MQTT config received, job start (MQTT Config sent via HA in QoS1)
  if (dataMQTTReceived && onStateAckReceived) {
    // Upload mode ON loop with blu LED ON
    if (uploadMode) {
      digitalWrite(LED_BUILTIN, LOW);    
      digitalWrite(WATER_PUMP_PIN, LOW);
      sendSensorStateAfterSeconds(TENSECONDSPERIOD); // this sendState does not wait for an ack    
    } else {
      // Upload mode OFF, turn off blu LED
      digitalWrite(LED_BUILTIN, HIGH);    
      // if battery analog level is below 890 (3.6V) the microcontroller can continue to wake up and sleep but it can't turn on the water pump
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