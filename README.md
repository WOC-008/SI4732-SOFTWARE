# 📻 Four Band Radio + Bluetooth Player  
### 🔧 ATmega328P | SI4735 | Embedded System Project

![Arduino](https://img.shields.io/badge/Arduino-Compatible-blue?logo=arduino)
![C++](https://img.shields.io/badge/Language-C++-blue.svg)
![Status](https://img.shields.io/badge/Project-Active-success)
![Open Source](https://img.shields.io/badge/Open--Source-Yes-green)

---

## 🚀 Project Overview
This is a custom-built **four band radio system with Bluetooth audio support**, designed using the **ATmega328P microcontroller** and **SI4735 radio module**.

It integrates:
- FM / AM / SW / LW radio reception  
- Bluetooth audio mode switching  
- Rotary encoder tuning system  
- LCD real-time display  
- Custom button-based control logic  
- RDS text streaming (FM mode)

---

## 🎥 Demo Video
[![Watch Demo](https://img.shields.io/badge/YouTube-Watch%20Demo-red?logo=youtube)](https://youtu.be/XpOFM7QXYrg)

👉 Full video: https://youtu.be/XpOFM7QXYrg

---

## 📡 Features
- 📻 Multi-band radio (FM / AM / SW / LW)
- 🎧 Bluetooth audio mode
- 🎚 Rotary encoder frequency + volume control
- 🔘 Multi-function button system:
  - Short press → Mute / Play-Pause  
  - Double press → Next band  
  - Triple press → Previous band  
  - Long press → Volume mode  
  - Extended hold → Bluetooth mode  
- 📺 LCD UI with real-time updates
- 📡 RDS text scrolling (FM stations)
- ⚡ Startup animation system

---

## 🔧 Hardware Used
- ATmega328P microcontroller
- SI4735 radio receiver module
- 16x2 I2C LCD display
- Rotary encoder
- Push button input system
- Bluetooth audio module
- Power switching circuit

---

## 📚 Libraries Used
- Wire (built-in Arduino library)
- LiquidCrystal_I2C
- SI4735 Radio Library

---

## 📁 Files Included
- Arduino `.ino` source code  
- Circuit diagram (EasyEDA PDF)  
- Wiring documentation  
- Project images  

---

## ⚠️ Important Notes
- LCD I2C address may vary (0x27 / 0x3F)
- Encoder uses interrupt pins for smooth tuning
- SI4735 requires stable power supply
- Button system uses timing-based logic for multi-press detection

---

## 📡 RDS System
- Displays live station text in FM mode  
- Automatically scrolls long text  
- Falls back to custom message if no signal  
- Fully customizable in firmware  

---

## 👤 Author
**Chukwuebuka Nweke**

Embedded Systems & Firmware Development  
Focus: MCU systems, radio DSP control, embedded UI logic

---

## 📄 Disclaimer
This is an experimental embedded systems project.  
Use at your own risk when modifying hardware or firmware.

---

## 📌 License
Open-source for educational and personal use.# SI4732-SOFTWARE
You can change the bluetooth code4 if  does not meet your specifics
