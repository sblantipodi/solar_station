# Welcome to the **Luciferin Project** wiki!

Ambient Lighting, or Bias Light, enhances your viewing experience by projecting colored lights onto the wall behind your TV or monitor. The lights match the colors on the screen, creating a more immersive and engaging atmosphere that makes movies, shows, and games feel even more captivating.

**Luciferin** is the light-emitting compound found in bioluminescent organisms like fireflies and glow worms.

<img align="right" width="100" height="100" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/luciferin_logo.png?raw=true">

**Firefly Luciferin** is a `Java Fast Screen Capture PC` software designed to work with **Glow Worm Luciferin firmware**. Together, they form a complete Bias Lighting and Ambient Light system for PCs.

> **Unlike other solutions, Firefly and Glow Worm Luciferin are designed to work seamlessly together for the best possible experience.**

---

## Quick Start

Want to build your own **Luciferin**? Start here: [Quick Start Guide](https://github.com/sblantipodi/firefly_luciferin/wiki/Quick-start)

---

## Key Features

* **High performance** with ultra low CPU/GPU usage.
* Advanced algorithms for:

    * [**Smooth color transitions**](https://github.com/sblantipodi/firefly_luciferin/wiki/Smoothing-color-transitions)
    * [**Color correction**](https://github.com/sblantipodi/firefly_luciferin/wiki/Color-Grading-%28Hue-Saturation-and-Lightness-tuning%29)
* Flexible connectivity:

    * [**Wireless or cabled**](https://github.com/sblantipodi/firefly_luciferin/wiki/Remote-Access)
    * [**Local or remote control**](https://github.com/sblantipodi/firefly_luciferin/wiki/Remote-Access) with MQTT support
    * [**Home Assistant integration**](https://github.com/sblantipodi/firefly_luciferin/wiki/Home-Automation-configs)
* [**Multi-monitor support**](https://github.com/sblantipodi/firefly_luciferin/wiki/Multi-monitor-support) with single or multiple instances.
* [**Surround lighting**](https://github.com/sblantipodi/firefly_luciferin/wiki/Surround-lighting-with-satellites) using satellites.
* [**Web browser firmware installer**](https://sblantipodi.github.io/glow_worm_luciferin/) and [**Web Interface**](https://github.com/sblantipodi/firefly_luciferin/wiki/Remote-Access#luciferin-web-interface).
* [**Programmable firmware**](https://github.com/sblantipodi/firefly_luciferin/wiki/Supported-GPIO-and-Baud-Rate), allowing on-the-fly microcontroller settings changes.
* Easy [**software & firmware upgrades**](https://github.com/sblantipodi/firefly_luciferin/wiki/Luciferin-update-management) with one click.
* Automatic [**aspect ratio switching**](https://github.com/sblantipodi/firefly_luciferin/wiki/Aspect-ratio) based on content.
* Gamer-focused: no lag, stutter-free.
* Multi-platform: [**Windows**](https://github.com/sblantipodi/firefly_luciferin/wiki/Installers-and-binaries) and [**Linux**](https://github.com/sblantipodi/firefly_luciferin/wiki/Linux-support), macOS coming soon.
* [**Official PCB available**](https://github.com/sblantipodi/firefly_luciferin/wiki/Ready-to-print-PCB) for those who prefer ready-to-assemble hardware.
* Community support: [**Join Discord**](https://discord.gg/aXf9jeN).

---

## Firefly Luciferin (PC Software) + Glow Worm Luciferin (Firmware)

[![Firefly Luciferin Demo](https://github.com/sblantipodi/glow_worm_luciferin/blob/master/assets/img/pc_ambilight.png?raw=true)](https://www.youtube.com/watch?v=68pnR5HMCTU)

> Click the image above to watch it on YouTube.

---

### 🧠 AI Summary — Setup & Requirements

**Purpose:**
Firefly Luciferin is a Java application for PC (Windows/Linux, macOS coming soon) that captures the screen and sends color data to an LED strip positioned behind the monitor to create an Ambilight effect.

**What’s needed:**

* **PC:** Windows or Linux for screen capture.
* **Microcontroller:** ESP8266 or ESP32 (Wi-Fi capable, cheap, stable).
* **LED Strip:** WS2812B (RGB, 5V) or SK6812 (RGBW, 5V).
* **Power Supply:** 5V, matching LED power draw.
* **Optional:** MQTT server or Home Automation integration (Home Assistant, OpenHAB, etc.).

**Firmware Installation (Glow Worm Luciferin):**

* Install via [browser installer](https://sblantipodi.github.io/glow_worm_luciferin) or flash manually.
* Two versions:

    * **FULL firmware:** supports remote control, OTA updates, Wi-Fi/Ethernet required (USB optional).
    * **LIGHT firmware:** USB required, no remote features.
* Wi-Fi/Ethernet control is faster than USB.
* ESP must be configured for Wi-Fi/Ethernet when using FULL firmware.

**Hardware Setup:**

* Connect LED strip data pin to the ESP microcontroller.
* Stabilize circuit with capacitor, resistor, and logic-level converter if needed.
* Optional compatible PCB designs available in Wiki.
* For FULL firmware without USB: connect 5V pin directly to power supply.
* Place LED strip behind monitor using double-sided tape.

    * First LED must be in **bottom half** of monitor for auto-configuration.
    * Orientation can be clockwise or counterclockwise.

**Software Setup (Firefly Luciferin Java App):**

* Download from [Releases](https://github.com/sblantipodi/firefly_luciferin/releases).
* Select firmware type on first run (FULL or LIGHT).
* Configure LED count and orientation under “LEDs config”.
* The app auto-discovers the Glow Worm device.
* Start bias lighting by double-clicking the tray icon.
* Default firmware supports up to **100 LEDs**, updated automatically when bias light starts.

**Optional Integrations:**

* Remote control and effects: see *Remote-Access* section.
* Home Automation setup: see *Home-Automation-configs* section.

Ecco il **riassunto AI-ottimizzato** del secondo spezzone del Wiki:

---

### 🧠 AI Summary — Installation Options

**Firefly Luciferin (PC Software):**

* Cross-platform Java application for **Windows** and **Linux**.
* Available as:

    * `.exe` installer for **Windows**
    * `.deb` and `.rpm` packages for **Linux**
* Official downloads: [GitHub Releases](https://github.com/sblantipodi/firefly_luciferin/releases)
* Linux alternative sources:

    * **Flathub:** [`org.dpsoftware.FireflyLuciferin`](https://flathub.org/apps/org.dpsoftware.FireflyLuciferin)
    * **Snap Store** (see issue [#207](https://github.com/sblantipodi/firefly_luciferin/issues/207))
    * **AUR (Arch Linux):** `firefly-luciferin-git`, community maintained by *Ape (Lauri Niskanen)*

**Glow Worm Luciferin (Firmware):**

* Can be flashed easily using the **[Web Installer](https://sblantipodi.github.io/glow_worm_luciferin)**.
* Alternatively, firmware binaries can be downloaded from [GitHub Releases](https://github.com/sblantipodi/glow_worm_luciferin/releases) and flashed manually.
* Manual flashing can be done via **esptool** (see *How-to-flash* Wiki page).

---

### 🧠 AI Summary — FULL Firmware Configuration (WiFi, Ethernet, MQTT, Web UI)

**1. Web Installer Setup:**

* Available at [sblantipodi.github.io/glow_worm_luciferin](https://sblantipodi.github.io/glow_worm_luciferin).
* Select **Install FULL** and choose the correct COM port.
* After flashing, enter **WiFi credentials** directly.
* If wrong credentials are entered → reinstall firmware and select **“erase device”**.
* Once connected, click **“Visit Device”** to open the **Web Interface**.
* From the top-right menu:

    * Enable **MQTT** (optional).
    * Enable **Ethernet** (if using supported hardware).
* Alternative configuration (without Web Installer) possible via the **Luciferin Access Point** mode.

**2. Provisioning via Firefly Luciferin (no browser needed):**

* The **Firefly Luciferin** desktop app can provision the device directly.
* Lets users reconfigure WiFi or MQTT if the device goes offline (e.g., password changed).
* No reflashing needed — restores connection in seconds.

**3. Firefly Luciferin Settings for FULL Firmware:**

* Download app: [GitHub Releases](https://github.com/sblantipodi/firefly_luciferin/releases).
* In settings → **Mode tab**, select “FULL firmware”.
* In **Network tab**, optionally enable MQTT.
* **MQTT must match** on both Firefly and Glow Worm (either both enabled or both disabled).
* Wireless streaming (WiFi/Ethernet) is **recommended** and faster than USB.
* If multiple Glow Worm devices exist, select one in **Output device** under “Mode” tab.
* Firefly automatically syncs configuration changes to the Glow Worm device.

**4. Web Interface Features (FULL Firmware only):**

* Access via:

    * `http://<device_name>.local` (Bonjour/mDNS)
    * or `http://<device_IP>`
* Allows remote control of colors, effects, brightness without PC client.
* “**Save State**” stores power/effect/brightness so state persists after reboot.
* Integrated device list in Firefly Luciferin shows reachable devices with IPs.

**5. MQTT Support:**

* Supports remote control via MQTT clients (mobile or PC).
* Default topic:

  ```
  lights/glowwormluciferin/set
  ```
* Example commands:

    * Turn ON: `{"state": "ON"}`
    * Turn OFF: `{"state": "OFF"}`
    * Apply effect: `{"state": "ON", "effect": "rainbow"}`
* **Supported effects:** `bpm`, `fire`, `twinkle`, `rainbow`, `chase rainbow`, `solid rainbow`, `mixed rainbow`, `solid`.
* Fully compatible with **Home Assistant MQTT Auto Discovery** (one-click integration).

---

### 🧠 AI Summary — Supported Hardware, LED Strips, and Power

**1. Firmware Overview**

* **Glow Worm Luciferin** is firmware for **ESP microcontrollers**, fully supporting:
  `ESP8266`, `ESP32`, `ESP32-C3`, `ESP32-S2`, `ESP32-S3`.
* Compatible with **Luciferin Official PCB** (D1 Mini format).
* Other boards can be supported via manual compilation using the [Arduino Bootstrapper](https://github.com/sblantipodi/arduino_bootstrapper).

**2. Recommended and Tested Boards**

* **ESP8266:** Lolin D1 Mini (4MB for FULL firmware, 1MB for LIGHT).
* **ESP32:** Lolin D32, TinyPICO.
* **ESP32-C3 / S2 / S3:** Fully supported (various Wemos boards).
* **Ethernet boards supported:**

    * QuinLED-ESP32-Ethernet / Dig-Octa
    * LilyGO-T-ETH-POE / T-POE-Pro
    * WT32-ETH01
    * ESP32-POE, ESP32-POE-WROVER
    * ESP32-ETHERNET-KIT-VE
    * WESP32

**Note:** Ethernet boards must be **explicitly supported** by the firmware.

**3. LED Strip Compatibility**

* Supports **RGB (WS2812B)** and **RGBW (SK6812)** 5V LED strips.
* 60 LEDs/m recommended as balance between brightness and power draw.
* Other supported chips: `APA102`, `SK9812`.
* Configuration guide for color calibration: *Color Temperature and White Balance* Wiki section.

**4. Power Supply Recommendations**

* Use **5V power supply** sized for total LED count:

    * 60 LEDs → 5V / 3A
    * 120 LEDs → 5V / 6A
* Oversizing PSU improves stability and heat management.
* **Never undersize** the power supply.
* For FULL firmware without USB, 5V pin of MCU must connect to the PSU.

**5. Hardware Connection Notes**

* FULL firmware adds remote control (requires WiFi/MQTT).
* LIGHT firmware requires USB connection.
* Optional circuit stabilizers: capacitor, resistor, and logic level shifter.
* Reference wiring diagram available in Wiki (“ambilight_bb.png”).

**6. Official PCB**

* [Luciferin Official PCB](https://github.com/sblantipodi/firefly_luciferin/wiki/Ready-to-print-PCB) available for ESP8266; supports easy soldering and stable wiring.

**7. Prebuilt Boards Support**

* Supported boards:

    * **QuinLED-Dig-Uno** (GPIO16 default)
    * **QuinLED-Dig2Go** (GPIO16 default, limited to 5V/3A)
    * **Gledopto** (requires “inverted relay” GPIO setting).
* Always verify and configure correct **GPIO and baud rate** as per board specs.

---

### 🧠 AI Summary — Luciferin Official PCB, Assembly, and Case

**1. Overview**

* The **Official Luciferin PCB** is a ready-to-print circuit board for building a stable Ambilight controller compatible with *Firefly Luciferin* and *Glow Worm Luciferin* firmware.
* Format: **D1 Mini** (ESP8266 / ESP32 / ESP32-C3 / ESP32-S2 / ESP32-S3 / TinyPICO).
* Provides a clean and robust wiring layout, supports auto-brightness and relay control.
* Designed for DIY users; requires only basic soldering skills.

**2. How to Build the PCB**

* **Steps:**

    1. Download the official **Gerber file**.
    2. Order the PCB from a fabrication service (e.g. **JLCPCB**).
    3. Purchase all components (Amazon, Mouser, etc.).
    4. Solder components following the **silkscreen labels** on the PCB.
* **Main Components (BOM):**

    * ESP board (D1 Mini / TinyPICO variants).
    * 74AHCT125N logic level converter (align notch).
    * Capacitors: 1500µF 16V + 100nF.
    * Resistors: 3×330Ω.
    * Fuse holder (Keystone 3557-2) + ATO fuses (e.g. 7.5A for 6A LED load).
    * WS2812B LED strip, header pins, jumpers, terminal blocks.
    * *Optional:* LDR (GL5516) + 10kΩ resistor for auto-brightness.

**3. Electrical & Load Details**

* Power via first terminal block (+IN−).
* LED strip connected on both sides (+OUT−).
* Data line connected via 4th/5th terminal block (jumper-selectable GPIO).
* Tested load:

    * **10A (1oz copper)**
    * **15A (2oz copper)**
    * Max temp rise: +15 °C under full load.
* **Warning:** Do *not* place USB jumper when using USB cable.

**4. Upgrade Options**

* PCB is **upgradable** — can replace ESP8266 with ESP32 / S2 / S3 / C3.
* TinyPICO module supported via dedicated *Luciferin Module* adapter board (use GPIO5).
* Future expansion modules planned.

**5. Case**

* Official 3D-printable case available in **Desktop** and **Regular** variants.
* Supports both USB and Wi-Fi builds.
* Print specs:

    * PLA, 20% infill, 0.2 mm layer height, supports enabled.
    * Print lid upside down for best logo result.
* Designed by [Patrick Blom](https://github.com/patrick-blom); available on *Thingiverse* and *Thangs*.

**6. Safety & Disclaimer**

* For **educational/hobby** use only — not CE/FCC/EMC/RoHS compliant.
* User assumes full responsibility for assembly and usage.
* Recommended to ask questions on **Discord** before building.

---

### 🧠 AI Summary — LED Strip Support, Color Order, and Configuration

**1. Supported LED Types**
Luciferin supports both **RGB** and **RGBW** addressable LED strips:

* **Models:** WS2812B, SK6812, APA102, SK9812.
* Strip type selection available in Firefly Luciferin settings or via Web Interface.

**2. LED Modes**

* **RGB:** uses only RGB channels.
* **RGBW:** white color uses only the dedicated white LED (low power).
* **RGBW mixed (Brighter):** white reproduced using both RGB + W channels (maximum brightness, higher power).
* **RGBW (RGB only):** white reproduced using RGB channels only (white LED ignored).

**3. Color Order Configuration**
Luciferin supports a wide range of **color channel orders** (e.g., GRB, RGBW, BGRW, etc.) to match the wiring of different LED brands.

* Color order can be changed under **Firefly Luciferin → Devices tab** or through the **Web Interface**.
* Mismatched color order causes wrong colors (e.g., red shows as green).

**4. LED Hardware Recommendations**

* Compatible with **5V WS2812B (RGB)** and **SK6812 (RGBW)** LED strips.
* Recommended density: **60 LEDs/m** for good brightness vs. power balance.
* Typical links:

    * [WS2812B RGB 5V LED Strip](https://it.aliexpress.com/wholesale?SearchText=WS2812B+5v)
    * [SK6812 RGBW 5V LED Strip](https://it.aliexpress.com/wholesale?SearchText=SK6812+5v)

**5. Common Issues to Detect / Help Users With**

* Wrong colors → check **color order** setting.
* Dim white → verify **RGBW mode** type (may be using RGB only).
* High current draw or overheating → switch to standard RGBW mode (disable mixed mode).
* No light → confirm correct **LED model**, **data pin**, and **5V power**.

---

### 🧠 AI Summary — GPIO Configuration & Baud Rate (Luciferin Firmware)

**1. General Info**
Luciferin allows full customization of **GPIO pins** for LED, relay, button, and LDR sensors.
GPIOs can be modified at runtime from the **Firefly Luciferin app (Devices tab)** or via the **Web Interface** (on Full firmware).

---

**2. Default GPIO Mapping per Microcontroller**

| MCU          | LED GPIO                                                                                           | Relay  | Button | LDR    | Notes              |
| ------------ | -------------------------------------------------------------------------------------------------- | ------ | ------ | ------ | ------------------ |
| **ESP8266**  | GPIO2 (HW UART, best for both Wi-Fi & USB) / GPIO3 (HW DMA, fastest Wi-Fi only) / GPIO5 (standard) | GPIO12 | GPIO0  | GPIO17 | GPIO3 disables USB |
| **ESP32**    | GPIO2 (HW DMA)                                                                                     | GPIO12 | GPIO0  | GPIO36 | –                  |
| **ESP32-C3** | GPIO6 (HW DMA)                                                                                     | GPIO0  | GPIO9  | GPIO3  | –                  |
| **ESP32-S2** | GPIO16 (HW DMA)                                                                                    | GPIO9  | GPIO0  | GPIO3  | –                  |
| **ESP32-S3** | GPIO16 (HW DMA)                                                                                    | GPIO13 | GPIO0  | GPIO2  | –                  |

🧩 For **Luciferin Official PCB modules**, refer to:
[Supported GPIO on Modules](https://github.com/sblantipodi/firefly_luciferin/wiki/Supported-GPIO-on-Modules)

---

**3. GPIO Customization**

* Change GPIO from *Devices → double-click GPIO → press Enter*.
* UI update may take a few seconds; restart capture after changes.
* Relay polarity can be inverted (needed for **Gledopto** and some other boards).
* Full firmware allows remote GPIO configuration via **Web Interface**.

---

**4. Baud Rate Configuration**

* **Default:**

    * *Light firmware:* 500 000
    * *Full firmware:* 115 200
* **Performance tips:**

    * Increase baud rate → higher FPS.
    * Decrease baud rate → reduce flickering.
* **Max stable rates:**

    * *ESP8266 CH340:* up to 1 000 000
    * *ESP8266 CP2102:* up to 921 600
      ⚠️ If an unsupported baud rate is set, you must **reflash the firmware manually**.

---

**5. Common Issues to Detect / Help Users With**

* 💡 *“LEDs not lighting / colors wrong”* → check GPIO selection.
* 🔌 *“Relay stuck ON/OFF”* → try inverting relay signal.
* ⚡ *“Flickering”* → lower baud rate or verify grounding.
* 🚫 *“Lost USB connection after GPIO change”* → user likely selected GPIO3 on ESP8266.
* 🧱 *“Can’t reconnect after baud change”* → microcontroller doesn’t support that rate → reflash required.

---

### 🔧 Recovering Glow Worm Luciferin after WiFi/MQTT Credential Change

If your Glow Worm Luciferin device stops responding due to changed WiFi or MQTT credentials, you can reset it using one of the following methods:

---

#### **1. USB + Web Installer**

1. Connect the ESP device to your PC via USB.
2. Reinstall the firmware using the [**Web Installer**](https://sblantipodi.github.io/glow_worm_luciferin/).
3. Enter new WiFi/MQTT credentials during installation.

---

#### **2. USB + Firefly Luciferin Provisioning**

1. Connect the ESP device via USB.
2. Use **Firefly Luciferin** to provision the device directly without reflashing.
3. Update WiFi/MQTT credentials.

---

#### **3. Remote Recovery without USB**

If the ESP is far from your PC or USB is unavailable:

1. Power off the ESP.
2. Power it on; it will attempt the old network for 3 minutes.

    * If it fails, LEDs turn **red** for 3 minutes.
3. Reboot the ESP **three times**:

    * Each reboot cycles the LEDs through **Red → Green → Blue → Orange**.
4. When LEDs turn **orange**, the device starts a **WiFi Access Point**.
5. Connect your PC or smartphone to the **Luciferin WiFi network**.
6. Open a browser at [http://192.168.4.1](http://192.168.4.1) to reset credentials and configure the device.

---

💡 **Tip:** This procedure works for both **FULL** and **LIGHT firmware**.
If you are using FULL firmware, after recovery you can also configure MQTT remotely via **Firefly Luciferin** or the **Web Interface**.

<img align="center" width="450" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/web_installer/7_webinstaller.png?raw=true">  

---

# Multi-Monitor Setup with Luciferin

Luciferin fully supports **multi-monitor configurations** via USB or Wireless (MQTT).
If you use **NVIDIA Surround** or **AMD Eyefinity**, monitors are treated as a single display, so no additional configuration is needed.

You have two main options:

1. **One microcontroller per monitor** (1 LED strip per monitor)
2. **One microcontroller for all monitors** (1 LED strip spanning all monitors)

---

## Pros and Cons

### ✅ One microcontroller per monitor

* No daisy-chaining required, easier wiring.
* No bandwidth restrictions, allowing higher framerates.
* Per-monitor control: you can game on a single monitor while keeping other monitors active.

### ✅ One microcontroller for all monitors

* Effects like rainbow can sync across all monitors.
* Easier wiring and troubleshooting.

---

## Configuration Steps

### 1. Configure MQTT (FULL firmware only)

* If using FULL firmware and the same MQTT server for all monitors, configure MQTT first.
* Skip this step if using LIGHT firmware.

<img width="450" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/multi_display/multimonitor_1.jpg?raw=true">

---

### 2. Set up multi-display in Firefly Luciferin

* Open the **Devices** tab.
* Select **dual** or **triple display**.
* Choose **Single device** if using one microcontroller for all monitors.
* Click **Save**.

<img width="450" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/multi_display/multimonitor_3.jpg?raw=true">

---

### 3. Configure each monitor

* Luciferin creates a tray icon per monitor.

  <img width="100" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/multi_display/triple_monitor_tray_grey.jpg?raw=true">

* Right-click each tray icon → configure the **number of LEDs** on that monitor.

  <img width="450" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/multi_display/multimonitor_7.jpg?raw=true">

---

### 4. Configure output devices

* **One microcontroller per monitor:** assign an output device per monitor:

    * USB → use **COM ports**
    * Wireless → use **Device Name**

      <img width="450" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/multi_display/multimonitor_6.jpg?raw=true">

* **Single microcontroller:** Firefly generates a **Gold instance**, which acts as the master controlling all monitors.

  <img width="100" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/multi_display/gold_instance.jpg?raw=true">

---

### 5. Adjust screen capture quality

* In the **Mode tab** → **Capture quality**:

    * **Balanced (Recommended):** Good performance for most users
    * **High Quality:** More precise, higher system load
    * **Low Quality:** Less precise, reduces system load (useful for weaker PCs or multi-monitor setups)

This ensures smooth performance even on high-resolution or multi-screen setups.

---

# LED Power Saving and Relay Control

Luciferin can help **reduce power consumption** of your LED setup by turning off LEDs during inactivity or by controlling a relay to cut power completely.

---

## 1. Turn Off LEDs During Inactivity

<img align="center" width="450" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/power_saving.jpg?raw=true">

* Luciferin can detect inactivity on the LED strip.
* You can configure a time (in minutes) after which the LEDs turn off if there is **no activity on screen**.
* The energy-saving feature also triggers when the screensaver is active.
* If active, LEDs will automatically turn off when you **shut down the PC**.

---

## 2. Use a Relay to Cut Power to the LED Strip

Even when all LEDs are turned off, the tiny controller in each LED still draws power.
Example: a WS2812B strip with 300 LEDs can consume up to **6W** when "off".

**Adding a relay** prevents this unnecessary energy consumption.

Some devices (e.g., **DigUno**, certain **Gledopto** boards) have integrated relays. In these cases, the relay GPIO must still be configured.

<img align="center" width="250" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/relay.jpg?raw=true">

---

### Relay Installation Options

1. **DC path:** Place the relay after the LED power supply.
2. **AC path:** Place the relay before the LED power supply.

* The ESP can still be powered separately via USB (e.g., phone charger).
* Luciferin automatically:

    * Sends a **HIGH signal** to the GPIO **before LEDs turn on**.
    * Sends a **LOW signal** to the GPIO **after LEDs are powered off**.

---

### Default GPIOs for Relay Control

| Microcontroller | Relay GPIO      |
| --------------- | --------------- |
| ESP8266         | GPIO12          |
| ESP32           | GPIO12 & GPIO22 |

> Note: Some devices (like GLEDOPTO) require **inverted GPIO signals**. This can be enabled in the GPIO settings.

[Learn how to change GPIOs here](https://github.com/sblantipodi/firefly_luciferin/wiki/Supported-GPIO-and-Baud-Rate#how-to-change-the-gpio-in-use)

---

### ⚠ Important Safety Notes

* **Double-check your relay ratings**: DC rating is usually different from AC rating.
* **Do not use a relay when the ESP is powered via USB**, as this may cause damage.

---

# Pull-Up Resistors

A **pull-up resistor** is a simple but essential component in digital circuits.
It connects an input pin **to the positive supply voltage (VCC)** to ensure a defined logic level.

<p align="center">
<img align="middle" width="400" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/pullup_res.jpg?raw=true">  
</p>

---

### Why Pull-Up Resistors Are Needed

* Without a pull-up, an **input pin can “float”**.
* Floating pins may **randomly read HIGH or LOW** due to electrical noise.
* The pull-up **forces the input to HIGH** when nothing else is driving it.

---

### Luciferin Specifics

* **GPIO0** is used as the **hardware button input**.
* It is configured as **INPUT_PULLUP** by default, so you don’t need an external resistor.
* When the button is pressed, the pin is pulled **LOW**, triggering an action in Luciferin.

---


# Luciferin Surround Lighting

Bring your media to life with **Luciferin Satellites** that extend the immersive experience beyond a single display.

The **Satellites Manager** allows you to add smart lights or microcontrollers as separate channels, each reflecting a portion of the main screen.

<p align="center">
<img align="center" width="650" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/sat1.jpg?raw=true">  
</p>

---

## How Satellites Work

* Satellites mirror the main instance behind your monitor/TV.
* Each satellite can focus on **specific zones** (left, right, top, bottom, or all zones).
* Ideal for creating multi-channel ambient lighting setups.

<p align="center">
<img align="center" width="650" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/sat2.jpg?raw=true">  
</p>

---

## Configuration Steps

1. Open **Devices tab** → **Satellites Manager**.
2. Enter the **IP address** of the satellite device.
3. Select the **zone** of the display the satellite will cover.
4. Set the **number of LEDs** attached to the satellite.
5. Choose the **algorithm** (average or average on all LEDs).
6. Click the **`+` button** to add the satellite.

<p align="center">
<img align="center" width="450" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/satellite_1.jpg?raw=true">  
</p>

* Added satellites appear in the **satellites table**.
* Click **OK** to finalize.

<p align="center">
<img align="center" width="450" src="https://github.com/sblantipodi/firefly_luciferin/blob/master/data/img/satellite_2.jpg?raw=true">  
</p>

---

## Notes

* **Full firmware only** (requires WiFi).
* Supports unlimited satellites **without extra performance cost**.
* Screen capture starts only after all satellites are connected.

This feature allows for **multi-zone ambient lighting**, perfect for home theaters, gaming setups, or music visualization.

---

