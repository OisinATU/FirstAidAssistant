# First Aid Assistant

## Overview

The First Aid Assistant is an Internet of Things (IoT) emergency response prototype developed using an ESP32 microcontroller. The system is designed to guide users through first aid procedures using a decision-tree interface while simultaneously monitoring physiological and location data.

The project combines embedded systems, sensor integration, wireless communication, and web technologies to create a portable emergency support device.

The system includes:

* MAX30102 pulse oximeter sensor for heart rate and SpO₂ monitoring
* GY-NEO6M GPS module for real-time location tracking
* ST7735 TFT display for emergency guidance and live data
* Speaker and LM386 amplifier for CPR metronome feedback
* Push button navigation system
* ESP32 web server for remote monitoring
* Blynk integration for IoT connectivity

---

# Features

## Emergency Decision Tree

The device guides the user through emergency response procedures using a step-by-step decision tree.

Example stages include:

* Safety check
* Responsiveness check
* Breathing assessment
* Calling emergency services
* Recovery position guidance
* CPR guidance
* AED instructions
* Patient monitoring

---

## Live Physiological Monitoring

The MAX30102 sensor provides:

* Heart Rate (BPM)
* Blood Oxygen Saturation (SpO₂)

The system uses infrared and red light measurements combined with the `spo2_algorithm` library to calculate physiological readings.

---

## GPS Tracking

The GY-NEO6M GPS module continuously updates:

* Latitude
* Longitude
* Altitude
* Satellite count

Location data can be displayed on the TFT screen and transmitted through the web dashboard.

---

## CPR Metronome

The system generates a CPR metronome tone at approximately 110 BPM to help users maintain the recommended chest compression rhythm.

---

## Web Dashboard

The ESP32 hosts a local web server that displays:

* Heart rate
* SpO₂
* GPS coordinates
* GPS fix status
* Satellite count
* Current decision-tree step
* System status

The dashboard updates live using JavaScript and JSON sensor data.

---

# Hardware Used

| Component          | Purpose               |
| ------------------ | --------------------- |
| ESP32 DevKit       | Main microcontroller  |
| MAX30102           | Pulse and SpO₂ sensor |
| GY-NEO6M           | GPS module            |
| ST7735 TFT Display | User interface        |
| LM386 Amplifier    | Audio amplification   |
| 8Ω Speaker         | CPR metronome output  |
| Push Buttons       | User input            |

---

# System Architecture

The project is divided into three main layers:

## Input Layer

* MAX30102 sensor
* GPS module
* Push buttons

## Processing Layer

* ESP32 microcontroller
* Decision-tree state machine
* Sensor processing algorithms

## Output Layer

* TFT display
* Speaker output
* Web dashboard
* Blynk connectivity

---

# Communication Protocols

| Protocol | Component            |
| -------- | -------------------- |
| I²C      | MAX30102 sensor      |
| UART     | GPS module           |
| SPI      | TFT display          |
| WiFi     | Web server and Blynk |

---

# Software and Libraries

## Development Environment

* Arduino IDE

## Main Libraries

```cpp
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <TinyGPSPlus.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"
#include <BlynkSimpleEsp32.h>
```

---

# Web Server

The ESP32 hosts a web server on port 80.

## Routes

| Route      | Function               |
| ---------- | ---------------------- |
| `/`        | Main dashboard webpage |
| `/sensors` | JSON sensor data       |

The dashboard updates live using JavaScript `fetch()` requests.

---

# Non-Blocking System Design

The project uses non-blocking programming techniques to ensure:

* Responsive button input
* Continuous GPS updates
* Stable web server communication
* Live Blynk updates
* Smooth sensor sampling

The system avoids long delays by using:

* `millis()` timing
* cooperative multitasking loops
* background update functions

---

# Example Sensor JSON Output

```json
{
  "pulse": 72,
  "spo2": 98,
  "lat": 53.2784,
  "lon": -9.0497,
  "gpsFix": 1,
  "sats": 7,
  "step": 5,
  "stepName": "Breathing",
  "status": "OK"
}
```

---

# Sustainability

The project supports:

* United Nations Sustainable Development Goal 3: Good Health and Well-being

The device is intended to assist users during the critical period before emergency services arrive.

---

# Future Improvements

Potential future improvements include:

* Battery integration
* Custom PCB design
* Waterproof enclosure
* Cloud connectivity
* Additional medical sensors
* Voice guidance
* Data logging
* Mobile application integration

---

# Author

Oisin Loftus

Bachelor of Engineering in Software & Electronic Engineering

Atlantic Technological University

2025/2026
