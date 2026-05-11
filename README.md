# Smart Beehive IoT Node

An industrial-grade IoT edge device prototype built with **ESP32** and **FreeRTOS**. This system monitors hive health and environment, utilizing a multi-tasking architecture to ensure high-frequency local feedback and secure cloud synchronization.

![Smart Beehive Simulation](beehive_wokwi.png)

## 🚀 Features
* **Multithreaded Execution:** Utilizes FreeRTOS to decouple sensor sampling/OLED updates (500ms) from MQTT network operations (5s).
* **Secure Telemetry:** Communicates via **MQTT over TLS (Port 8883)** to a private HiveMQ Cloud cluster.
* **Local Visualization:** Real-time data rendering on a 128x64 SSD1306 OLED via I2C.
* **Remote Command Interface:** Subscribes to command topics for remote actuator (Fan/Relay) control.

## 🛠️ Tech Stack
* **Hardware:** ESP32, DHT22 (One-Wire), SSD1306 OLED (I2C).
* **Framework:** Arduino + FreeRTOS.
* **Protocols:** MQTT, TLS/SSL, I2C, One-Wire.
* **Environment:** VS Code + PlatformIO.

## 📁 Project Structure
* `src/main.cpp`: Main firmware logic and FreeRTOS task management.
* `diagram.json`: Wokwi simulation configuration.
* `platformio.ini`: Dependency and build settings.

## 🔗 Simulation Link
[Click here to view the live simulation on Wokwi](https://wokwi.com/projects/463757426410822657)
