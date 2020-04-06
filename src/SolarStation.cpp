/*
 * SOLAR STATION
 * 
 * Smart solar watering system 
 * DPsoftware (Davide Perini)
 * Components:
 *   - Arduino C++ sketch running on an ESP8266EX D1 Mini from Lolin running at 80MHz
 *   - Raspberry + Home Assistant for Web GUI, automations and MQTT server
 *   - 88x142 5V solar panel
 *   - Sony VCT6 18650 Lithium Battery
 *   - TP4056 protected lithium charger
 *   - MT3608 DC DC step up module to step up battery voltage to 5.5V, ESP chip is happy with it
 *   - MT3608 DC DC step up module to step up battery voltage to 8.66V, water pump is powerful with it
 *   - Relay Shield to safely power the water pump and "detach it from the circuit"
 *   - 100kΩ + 22kΩ + 4.4kΩ resistance for Battery voltage level monitoring circuit
 *   - 3.5/9V water pump (3W @ 9V)
 *   - TTP223 capacitive touch button with A contact soldered (HIGH signal when button is not pressed, 
 *     LOW signal when button is pressed), used to reset the microcontroller
 *   - Google Home Mini for Voice Recognition
 * MIT license
 */
#include <FS.h> //this needs to be first, or it all crashes and burns...
#include <SolarStation.h>

/********************************** START SETUP*****************************************/
void setup() {
  
  Serial.begin(serialRate);

  pinMode(WATER_PUMP_PIN, OUTPUT); // setup water pump pin
  digitalWrite(WATER_PUMP_PIN, LOW); // turn off the pump just in case
  pinMode(LED_BUILTIN, OUTPUT);  // setup built in ESP led
  digitalWrite(LED_BUILTIN, HIGH); // turn off the ESP led
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  //OTA SETUP
  ArduinoOTA.setPort(OTAport);
  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(SENSORNAME);

  // No authentication by default
  ArduinoOTA.setPassword((const char *)OTApassword);

  ArduinoOTA.onStart([]() {
    Serial.println(F("Starting"));
  });
  ArduinoOTA.onEnd([]() {
    Serial.println(F("\nEnd"));
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println(F("Auth Failed"));
    else if (error == OTA_BEGIN_ERROR) Serial.println(F("Begin Failed"));
    else if (error == OTA_CONNECT_ERROR) Serial.println(F("Connect Failed"));
    else if (error == OTA_RECEIVE_ERROR) Serial.println(F("Receive Failed"));
    else if (error == OTA_END_ERROR) Serial.println(F("End Failed"));
  });
  ArduinoOTA.begin();

  Serial.println(F("Ready"));
  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());

}

/********************************** START SETUP WIFI *****************************************/
void setup_wifi() {

  unsigned int reconnectAttemp = 0;

  // DPsoftware domotics 
  Serial.println();
  Serial.println(F("DPsoftware domotics"));
  delay(delay_3000);
  
  Serial.println(F("Connecting to: "));
  Serial.print(ssid); Serial.println(F("..."));
  Serial.println();
  Serial.print(F("Connecting to "));
  Serial.print(ssid);  

  delay(delay_2000);

  WiFi.persistent(false);   // Solve possible wifi init errors (re-add at 6.2.1.16 #4044, #4083)
  WiFi.disconnect(true);    // Delete SDK wifi config
  delay(200);
  WiFi.mode(WIFI_STA);      // Disable AP mode
  //WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.setAutoConnect(true);  
  #ifdef TARGET_SOLAR_WS
    WiFi.config(arduinoip, mydns, mygateway);
  #endif  

  WiFi.hostname(SENSORNAME);

  // Set wifi power in dbm range 0/0.25, set to 0 to reduce PIR false positive due to wifi power, 0 low, 20.5 max.
  #ifdef TARGET_SOLAR_WS
    WiFi.setOutputPower(20.5);
  #endif

  WiFi.begin(ssid, password);

  // loop here until connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
    reconnectAttemp++;
    if (reconnectAttemp > 10) {
      digitalWrite(WATER_PUMP_PIN, LOW);      
      Serial.print(F("Reconnect attemp= "));
      Serial.print(reconnectAttemp);
      if (reconnectAttemp >= MAX_RECONNECT) {
        Serial.println(F("Max retry reached, powering off peripherals."));
        espDeepSleep(false, false);
      }
    } else if (reconnectAttemp > 10000) {
      reconnectAttemp = 0;
    }
  }

  Serial.println(F("WIFI CONNECTED"));
  Serial.println(WiFi.localIP());

  delay(delay_1500);

}

/*
   Return the quality (Received Signal Strength Indicator) of the WiFi network.
   Returns a number between 0 and 100 if WiFi is connected.
   Returns -1 if WiFi is disconnected.
*/
int getQuality() {
  if (WiFi.status() != WL_CONNECTED)
    return -1;
  int dBm = WiFi.RSSI();
  if (dBm <= -100)
    return 0;
  if (dBm >= -50)
    return 100;
  return 2 * (dBm + 100);
}


/********************************** START CALLBACK*****************************************/
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

/********************************** START PROCESS JSON*****************************************/
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
  
  const char* waterPumpActiveConst = doc["pump_active"];
  String waterPumpActiveStr = waterPumpActiveConst;
  waterPumpActive = waterPumpActiveStr == "on";

  const char* pumpSecondsConst = doc["pump_seconds"];
  String pumpSecondsStr = pumpSecondsConst;
  waterPumpSecondsOn = pumpSecondsStr.toDouble() * 1000;

  const char* espSleepTimeMinutesConst = doc["esp_sleep_time_minutes"];
  String espSleepTimeMinutesStr = espSleepTimeMinutesConst;
  espSleepTime = (espSleepTimeMinutesStr.toDouble() >= 1) ? (espSleepTimeMinutesStr.toDouble() * 60 * 1000000) : (1e6);

  Serial.println("MQTT CONFIG RECEIVED");
  serializeJsonPretty(doc, Serial);

  // dataMQTTReceived enables sketch processing, we do our things only after MQTT config has been received
  dataMQTTReceived = true;

  // Reset the millis used for water pump activity
  nowMillisWaterPumpStatus = millis();
  // Reset the millis used for send status activity
  nowMillisStatusWithPumpOn = millis();
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
  if(millis() > sensorStateNowMillis + delay_1000){
    sensorStateNowMillis = millis();
    number_of_attemps++;
    sendSensorStateNotTimed();
  }
}
void sendSensorStateNotTimed() {    
    StaticJsonDocument<BUFFER_SIZE> doc;
    JsonObject root = doc.to<JsonObject>();

    root["Whoami"] = SENSORNAME;
    root["IP"] = WiFi.localIP().toString();
    root["MAC"] = WiFi.macAddress();
    root["ver"] = VERSION_SHORT;

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
    root["wifi"] = getQuality();
    root["frequency"] = ESP.getCpuFreqMHz();  
    root["remaining_seconds"] = waterPumpRemainingSeconds;  
    
    
    char buffer[measureJson(root) + 1];
    serializeJson(root, buffer, sizeof(buffer));

    Serial.println();
    Serial.println("SENDING SENSOR STATE");
    serializeJsonPretty(root, Serial);

    client.publish(SOLAR_STATION_STATE_TOPIC, buffer, false);

    delay(delay_500);

    // hardCutOff but only if uploadMode is false
    if (hardCutOff && !uploadMode) {
      espDeepSleep(true, true);
    }
}

void sendSensorStateAfterSeconds(int delay) {
  if(millis() > nowMillisStatusWithPumpOn + delay){
    nowMillisStatusWithPumpOn = millis();
    number_of_attemps++;
    waterPumpRemainingSeconds = (waterPumpRemainingSeconds-(delay/1000));
    sendSensorStateNotTimed();
  }
}

void sendOnState() {    
  if(millis() > onStateNowMillis + delay_1000){
    onStateNowMillis = millis();
    number_of_attemps++;
    Serial.println("SENDING ON STATE"); 
    client.publish(SOLAR_STATION_POWER_TOPIC, ON_CMD, false);
    delay(delay_500);  
  }
}

void sendOffState() {
  if(millis() > offStateNowMillis + delay_1000){
    offStateNowMillis = millis();
    number_of_attemps++;
    Serial.println("SENDING OFF STATE");
    client.publish(SOLAR_STATION_POWER_TOPIC, OFF_CMD, false);
    delay(delay_500);
  }
}

void sendWaterPumpPowerStateOff() {
  if(millis() > waterPumpPowerStateOffNowMillis + delay_1000){
    waterPumpPowerStateOffNowMillis = millis();
    number_of_attemps++;
    Serial.println(); 
    Serial.print("SENDING WATER PUMP POWER STATE OFF");
    client.publish(SOLAR_STATION_WATERPUMP_POWER_TOPIC, OFF_CMD, false);
    delay(delay_500);
  }
}

void sendWaterPumpPowerStateOn() {
  if(millis() > waterPumpPowerStateOnNowMillis + delay_1000){
    waterPumpPowerStateOnNowMillis = millis();
    number_of_attemps++;
    Serial.println(); 
    Serial.print("SENDING WATER PUMP POWER STATE ON"); 
    client.publish(SOLAR_STATION_WATERPUMP_POWER_TOPIC, ON_CMD, false);
    delay(delay_500);
  }
}

void sendWaterPumpActiveStateOff() {
  if(millis() > waterPumpActiveStateOffNowMillis + delay_1000){
    waterPumpActiveStateOffNowMillis = millis();
    number_of_attemps++;
    Serial.println(); 
    Serial.println("SENDING WATER PUMP ACTIVE STATE OFF"); 
    client.publish(SOLAR_STATION_WATERPUMP_ACTIVE_STAT_TOPIC, OFF_CMD, false);
    delay(delay_500);
  }
}

/********************************** START MQTT RECONNECT *****************************************/
void mqttReconnect() {
  // how many attemps to MQTT connection
  int brokermqttcounter = 0;
  // Loop until we're reconnected
  while (!client.connected()) {   
    // Attempt to connect to MQTT server with QoS = 1 (pubsubclient supports QoS 1 for subscribe only, published msg have QoS 0 this is why I implemented a custom solution)
    if (client.connect(SENSORNAME, mqtt_username, mqtt_password, 0, 1, 0, 0, 1)) {
      Serial.println(F("connected"));
                
      client.subscribe(SOLAR_STATION_UPLOADMODE_TOPIC);      
      client.subscribe(SOLAR_STATION_WATERPUMP_ACTIVE_TOPIC);      
      client.subscribe(SOLAR_STATION_MQTT_CONFIG);      
      client.subscribe(SOLAR_STATION_MQTT_ACK);

      delay(delay_2000);
      brokermqttcounter = 0;
    } else {
      Serial.println(F("Number of attempts="));
      Serial.println(brokermqttcounter);
      // after 10 attemps all peripherals are shut down
      digitalWrite(WATER_PUMP_PIN, LOW);
      if (brokermqttcounter >= MAX_RECONNECT) {
        Serial.println(F("Max retry reached, powering off peripherals."));
        espDeepSleep(false, false);
      } else if (brokermqttcounter > 10000) {
        brokermqttcounter = 0;
      }
      brokermqttcounter++;
      // Wait 5 seconds before retrying
      delay(500);
    }
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
  delay(delay_1000);
  // if hardCutOff sleep forever or until capacitive button is pressed
  if (hardCutOff) {
    ESP.deepSleep(0);
  } else {
    ESP.deepSleep(espSleepTime);
  }
}
// force deepSleep after 15 minutes
void forceDeepSleep() {
  if((millis() > nowMillisForceDeepSleepStatus + FORCE_DEEP_SLEEP_TIME) || (MQTT_PUBLISH_MAX_RETRY == number_of_attemps)){
    nowMillisForceDeepSleepStatus = millis();
    digitalWrite(WATER_PUMP_PIN, LOW);
    espDeepSleep(true, false);
  }
}

/********************************** BATTERY LEVEL *****************************************/
// read analog battery level from A0 pin (voltage divider required on that PIN because ESP can read up to 3.3V and the battery goes up to 4.2V)
int readAnalogBatteryLevel() {
  return analogRead(A0);
}

/********************************** START MAIN LOOP *****************************************/
void loop() {  
  
  // Wifi management
  if (WiFi.status() != WL_CONNECTED) {
    delay(1);
    Serial.print(F("WIFI Disconnected. Attempting reconnection."));
    setup_wifi();
    return;
  }

  ArduinoOTA.handle();

  if (!client.connected()) {
    mqttReconnect();
  }
  client.loop();

  // Send status on startup and wait for MQTT config
  if (!onStateAckReceived) {
    sendOnState();
  }
 
  // MQTT config received, job start (MQTT Config sent via HA in QoS1)
  if (dataMQTTReceived) {
    // Upload mode ON loop with blu LED ON
    if (uploadMode) {
      digitalWrite(LED_BUILTIN, LOW);    
      digitalWrite(WATER_PUMP_PIN, LOW);
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
              sendSensorStateAfterSeconds(delay_1000); // this sendState does not wait for an ack    
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

  delay(delayTime);
  
}