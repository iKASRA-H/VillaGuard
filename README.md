# Smart Home Security Upgrade (ESP32 + SIM800A)

This project is a **DIY upgrade** that modernizes an existing wired alarm system for a remote villa.  
Instead of replacing the alarm, we integrated a single **ESP32** with a **SIM800A GSM module** to read the alarm’s original wired signals and control its inputs/outputs, enabling SMS-based control and monitoring even where internet is unavailable.

## 🔧 Key Features
- **Wired-alarm integration:** Taps into the legacy wired alarm signals — reads the alarm’s command/input and controls its outputs without replacing the original panel.  
- **SMS Control & Alerts (SIM800A):** Send SMS commands to actuate relays (siren, lights, heating); receive alerts for alarm triggers, power failures, and temperature.  
- **Intrusion Detection:** When the wired alarm triggers (siren/trigger line), the ESP32 detects it and sends immediate SMS alerts.  
- **Environmental & Power Monitoring:** Reads indoor temperature via **SHT20** and monitors 12V power input to detect outages and switch to UPS backup.  
- **Persistent States:** Relay states are stored in nonvolatile memory so the system recovers correctly after power loss.  
- **Simple Remote Control:** Arm/disarm via SMS commands (no internet required).

## 🛠️ Hardware
- **ESP32 Development Board** (single node)  
- **SIM800A GSM Module**  
- **SHT20 Temperature & Humidity Sensor**  
- **Relay Modules** (to control siren, lights, heating)  
- **12V Power Supply + UPS backup**

## 📍 Overview
By reading the native wired alarm signals and controlling the same I/O lines, this project upgrades legacy systems with modern SMS-based remote control and monitoring while preserving the original alarm hardware — a low-cost, minimally invasive retrofit suitable for remote/off-grid properties.
![IMG_2901](https://github.com/user-attachments/assets/7cb64334-d6a0-47be-8c14-c28bafcd96bc)
![IMG_2813](https://github.com/user-attachments/assets/b1784713-9c4e-45cd-9395-616f9ba8b0bd)
