Event-Driven Safety Protection Kernel

Overview

This project implements an automotive-inspired safety protection controller using ESP32, Wokwi Simulator, and Blynk IoT Cloud. The system continuously monitors battery voltage and detects fault conditions using a non-blocking event-driven architecture.

Features

- Undervoltage Detection
- Overvoltage Detection
- Sensor Fault Detection
- Rapid Voltage Fluctuation Detection
- Relay Cutoff Simulation
- Buzzer Alerts
- LCD Warning Messages
- Automatic Recovery Logic
- Blynk IoT Dashboard Monitoring

Hardware Used

- ESP32
- Potentiometer
- Push Button
- Buzzer
- Green LED
- Red LED
- LCD1602 I2C

Software Used

- Arduino Framework
- Wokwi Simulator
- Blynk IoT Cloud

Fault States

- NORMAL
- UNDERVOLTAGE_FAULT
- OVERVOLTAGE_FAULT
- SENSOR_FAULT
- FLUCTUATION_FAULT

Author

Subhronil Haldar
