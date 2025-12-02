# 📡 PowDew-Sense: Zero-Component Human Presence Detector

### 🌟 Project Overview

**PowDew-Sense** is an innovative, cost-effective project that demonstrates **Human Presence Detection** without relying on traditional external sensors (like PIR, ultrasonic, or dedicated mmWave radar). It converts a core concept from wireless communications—**RF Power Density (PowDew) Attenuation**—into a practical, functional sensing tool.

The system uses an **ESP8266** microcontroller connected to a mobile phone's Wi-Fi hotspot to measure the real-time **Received Signal Strength Indicator (RSSI)**. When a person steps between the two devices, their body absorbs and reflects the 2.4 GHz radio waves, causing a predictable, sharp drop in the measured RSSI (the "PowDew drop"). This drop signals the presence of a human.

### 🔬 Core Principle: PowDew Sensing

The project leverages the phenomenon of **signal attenuation** caused by obstacles.

1. **Stable Baseline:** When the line of sight is clear, the signal strength (RSSI) is high and stable (e.g., $\approx -50\text{ dBm}$). This represents the ambient **PowDew** at the receiver.
2. **The Block:** A human body, being largely water, acts as an effective radio frequency absorber.
3. **The PowDew Drop:** When a person blocks the signal path, the absorbed energy causes a significant, measurable drop in the RSSI (e.g., to $\approx -70\text{ dBm}$).
4. **Sensing:** The system interprets this sharp, consistent drop as **Human Presence**.

### 🛠️ Requirements

#### Hardware

| Component | Role | Notes |
| :--- | :--- | :--- |
| **1x ESP8266 Board** | Receiver & Web Server | NodeMCU or any ESP8266/ESP32 running Arduino code. |
| **1x Mobile Phone** | Transmitter (AP) & Client (PWA) | Must have a **Personal Hotspot** feature and a modern web browser. |
| **1x Micro-USB Cable** | Power & Programming | Standard cable for the ESP8266. |

#### Software & Libraries

* **Arduino IDE**
* **ESP8266 Board Support Package**
* **Libraries:**
* `ESP8266WiFi.h`
* `ESPAsyncTCP.h`
* `ESPAsyncWebServer.h` (Crucial for non-blocking Web Server)

### ⚙️ How It Works (System Architecture)

The system works by having the ESP8266 act as a client connected to the phone's Wi-Fi network.

1. **Connection (Station Mode):** The ESP8266 is configured as a Wi-Fi **Station (`WIFI_STA`)** and connects to the mobile phone's personal hotspot (the AP). This allows the ESP to reliably measure the RSSI of that connection using `WiFi.RSSI()`.
2. **Web Server:** The ESP8266 hosts a non-blocking web server on its IP address.
* The main HTML/JavaScript files are served to the connecting phone's browser (the PWA).
* A dedicated **JSON endpoint** (`/rssi_data`) is created to serve the current RSSI value.
3. **PWA (Real-Time Visualization):**
* The JavaScript uses **AJAX** to poll the `/rssi_data` endpoint every $200\text{ ms}$.
* The script applies conditional logic to the received RSSI value, updating a visual gauge and color-coded status message (**GREEN** for clear, **RED** for blocked).

#### RSSI to Status Mapping:

| RSSI Range (dBm) | Status (Interpretation) | PWA Visual |
| :--- | :--- | :--- |
| $\text{RSSI} > -50$ | **High PowDew (Clear)** | Green |
| $-75 < \text{RSSI} \le -50$ | **Medium PowDew (Attenuated)** | Yellow |
| $\text{RSSI} \le -75$ | **Low PowDew (Presence Detected)** | Red |

### 🚀 Setup Instructions (Quick Start)

1. **Configure Hotspot:** On your mobile phone, enable your personal hotspot and note the SSID and Password.
2. **Update Code:** In the Arduino sketch, update the `ssid` and `password` variables with your hotspot credentials.
```cpp
const char* ssid = "YOUR_HOTSPOT_NAME";
const char* password = "YOUR_HOTSPOT_PASSWORD";
```
3. **Upload:** Compile and upload the code to your ESP8266 board.
4. **Connect:** Open the Serial Monitor to find the assigned **IP Address** (e.g., `192.168.43.xxx`).
5. **Test:** Open your phone's browser and navigate to the displayed IP address.

Place the phone and ESP8266 approximately $3\text{ meters}$ apart, then walk between them to observe the immediate, dramatic change in the visual gauge!
