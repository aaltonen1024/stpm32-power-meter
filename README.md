This repository provides a minimal, efficient Arduino library for the STPM32 (and compatible STPM3x) energy metering IC, along with a sample project for real-time measurement and OLED display of:

- RMS Voltage
- RMS Current
- Active Power
- Active Energy
- Electricity Cost (in Rupiah)

The library and example are designed for simplicity and easy integration with ESP32/ESP8266 or Arduino projects.

Features
- Minimal footprint: Only the essential measurement functions are included.

- Fast SPI communication with the STPM32.

- OLED display (SSD1306) support for real-time feedback.

- Mode selection via button for multiple display screens.
