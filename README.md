# Solar Station
Smart solar watering system  
_Written for Arduino IDE and PlatformIO._

[![GitHub Actions CI](https://github.com/sblantipodi/solar_station/workflows/GitHub%20Actions%20CI/badge.svg)](https://github.com/sblantipodi/solar_station/actions)
[![GitHub version](https://img.shields.io/github/v/release/sblantipodi/solar_station.svg)](https://github.com/sblantipodi/solar_station/releases)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Maintenance](https://img.shields.io/badge/Maintained%3F-yes-green.svg)](https://GitHub.com/sblantipodi/solar_station/graphs/commit-activity)
[![DPsoftware](https://img.shields.io/static/v1?label=DP&message=Software&color=orange)](https://www.dpsoftware.org)

If you like **Solar Station**, give it a star, or fork it and contribute!

[![GitHub stars](https://img.shields.io/github/stars/sblantipodi/solar_station.svg?style=social&label=Star)](https://github.com/sblantipodi/solar_station/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/sblantipodi/solar_station.svg?style=social&label=Fork)](https://github.com/sblantipodi/solar_station/network)

Project is bootstrapped with my [Arduino Bootstrapper](https://github.com/sblantipodi/arduino_bootstrapper) library.

## Credits
- Davide Perini

## Components:
  - Arduino C++ sketch running on an ESP8266EX D1 Mini from Lolin running at 80MHz
  - Raspberry + Home Assistant for Web GUI, automations and MQTT server
  - 88x142 5V solar panel
  - Sony VCT6 18650 Lithium Battery
  - TP4056 protected lithium charger
  - MT3608 DC DC step up module to step up battery voltage @ 5.5V, ESP chip is happy with it
  - MT3608 DC DC step up module to step up battery voltage @ 8.66V, water pump is powerful with it
  - Relay Shield to safely power the water pump and "detach it from the circuit"
  - (100kΩ + (22kΩ + 4.4kΩ)) voltage divider for Battery voltage level monitoring circuit
  - 3.5V/9V water pump (3W @ 9V)
  - TTP223 capacitive touch button with A contact soldered (HIGH signal when button is not pressed, 
    LOW signal when button is pressed), used to reset the microcontroller
  - Google Home Mini for Voice Recognition
  
## Schematic  
![CIRCUITS](https://github.com/sblantipodi/solar_station/blob/master/assets/img/fritzing_hardware_project.png)

## First working project:
![IMG](https://github.com/sblantipodi/solar_station/blob/master/assets/img/1.jpg)

## Final working project with two DC to DC step up modules:
![IMG](https://github.com/sblantipodi/solar_station/blob/master/assets/img/2.jpg)

## Voltage divider: 

- Voltage source = 4.2V (lithium battery at max)
- R1 = 100kΩ 
- R2 = 22kΩ + 4.4kΩ (in series)
- Voltage Out = 3.3V (maximum voltage that a D1 Mini can read from the analog pin)  

![IMG](https://github.com/sblantipodi/solar_station/blob/master/assets/img/3b.jpg)

## From the top:
![IMG](https://github.com/sblantipodi/solar_station/blob/master/assets/img/4.jpg)

## Water pump:
![IMG](https://github.com/sblantipodi/solar_station/blob/master/assets/img/water_pump.jpg)

## IP56 box:
![IMG](https://github.com/sblantipodi/solar_station/blob/master/assets/img/5.jpg)

## Second iteration of the same project for other plants in a non stacked format
![IMG](https://github.com/sblantipodi/solar_station/blob/master/assets/img/solar_station_part2.jpg)  

![IMG](https://github.com/sblantipodi/solar_station/blob/master/assets/img/solar_station_part2_front.jpg)


## Home Assistant Mobile Client Screenshots
![IMG](https://github.com/sblantipodi/solar_station/blob/master/assets/img/ha_screenshot_d.jpg)

## License
This program is licensed under MIT License

## Thanks To 
|  Thanks              |  For                           |
|----------------------|--------------------------------|
|<a href="https://www.jetbrains.com/"><img width="200" src="https://raw.githubusercontent.com/sblantipodi/arduino_bootstrapper/master/data/img/jetbrains.png"></a>| For the <a href="https://www.jetbrains.com/clion">CLion IDE</a> licenses.|

