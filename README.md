# 🌈 RGB Double Helix Lamp with Arduino Nano

This project is a customizable RGB lamp inspired by the original [RGB Double Helix Lamp - EasyPrint](https://www.printables.com/model/62043-rgb-double-helix-lamp-easyprint). 
It uses an **ESP32 C3** and the **FastLED** library to create dynamic LED lighting effects.

## 🛠️ Project Overview

- ESP32 code to control individually addressable RGB LEDs (WS2812 or similar)
- Multiple lighting effects (rainbow, fade, color transitions, etc.)
- 3D printed enclosure (external source)

This project builds upon my previous work in [HelixLamp](https://github.com/BogyS/HelixLamp), where I first tested color palettes and button logic.


## 📦 What You Need

- ESP32 C3 Super Mini
- HW-221 Level Shifter
- WS2812 LED strip
- 5V USB power supply
- Jumper wires, 100Ω resistor (on DIN line), 470µF capacitor, push button
- 3D printed parts: [Printables model link](https://www.printables.com/model/62043-rgb-double-helix-lamp-easyprint)

## 💡 Libraries Used

- [FastLED](https://github.com/FastLED/FastLED)
- [OneButton](https://github.com/mathertel/OneButton)
- [esp-fs-webserver](https://github.com/cotestatnt/esp-fs-webserver)

## 🕹️ How to Use

See full usage instructions here 👉 [USAGE.md](./doc/USAGE.md)



