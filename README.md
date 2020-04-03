# SOLAR STATION
Arduino (ESP8266) solar powered project

Smart solar watering system 
DPsoftware (Davide Perini)

Components:
  - Arduino C++ sketch running on an ESP8266EX D1 Mini from Lolin running at 80MHz
  - Raspberry + Home Assistant for Web GUI, automations and MQTT server
  - 88x142 5V solar panel
  - Sony VCT6 18650 Lithium Battery
  - TP4056 protected lithium charger
  - MT3608 DC DC step up module to step up battery voltage to 5V, ESP chip is happy with it
  - MT3608 DC DC step up module to step up battery voltage to 9V, water pump is powerful with it
  - Relay Shield to safely power the water pump and "detach it from the circuit"
  - 100kΩ + 22kΩ + 4.4kΩ resistance for battery voltage level monitoring circuit (voltage divider)
  - 3.5/9V water pump
  - Google Home Mini for Voice Recognition
MIT license


