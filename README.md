# 🌦️ Weather Station ESP32

ESP32-based weather station with OLED user interface, historical data logging and environmental monitoring.

---

## 🔧 Hardware

* ESP32-WROOM-32
* DHT22 (Temperature & Humidity)
* BMP280 (Air Pressure)
* SH1106 OLED Display (I2C)
* Push Button

---

## ✨ Features

* Temperature measurement
* Humidity measurement
* Air pressure measurement
* Sea-level pressure correction
* OLED user interface
* Min / Max statistics
* Historical data buffer
* Temperature graph
* Humidity graph
* Pressure graph
* Weather tendency detection
* Screen saver mode
* Button-controlled navigation

---

## 📊 Wiring

| Component | Signal | ESP32 Pin |
| --------- | ------ | --------- |
| DHT22     | Data   | GPIO17    |
| BMP280    | SDA    | GPIO21    |
| BMP280    | SCL    | GPIO22    |
| OLED      | SDA    | GPIO21    |
| OLED      | SCL    | GPIO22    |
| Button    | Signal | GPIO16    |
| LED       | Output | GPIO2     |

---

## 📁 Project Structure

```text
src/
include/
lib/
platformio.ini
```

---

## 🎯 Learning Goals

* ESP32 development
* Embedded software architecture
* Sensor integration
* OLED graphics
* Ring buffer data processing
* Historical trend analysis
* Modular C++ design

---

## 🚀 Planned Features

* WiFi connectivity
* NTP time synchronization
* OTA updates
* MQTT support
* Web dashboard
* Home Assistant integration

