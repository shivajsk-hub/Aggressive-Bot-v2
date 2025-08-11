# Aggressive-Bot-V2 🤖
*Arduino + Tinkercad combat bot simulation* with 6 servos, LCD HUD, ultrasonic sensing, tilt/light inputs, and power-friendly motion control.

[![Made with Arduino](https://img.shields.io/badge/made%20with-Arduino-00979D.svg)](#)
[![Simulation Tinkercad](https://img.shields.io/badge/simulation-Tinkercad-orange.svg)](#)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

---

## ✨ Highlights
- *LCD HUD* (16×2) shows mode, temp, and sweep %  
- *6× servos* with smooth, staggered updates (reduced current spikes)  
- *Ultrasonic + tilt + light + TMP36* integrated  
- *Demo Mode* cycles behaviors for clean recording  
- *Single-file code* (Aggressive-Bot-v2.ino) easy to run

---

## 🧱 Hardware (as built in Tinkercad)
- Arduino Uno  
- 6× SG90/MG90S servos (external 5–6V regulator, common GND)  
- Ultrasonic sensor (front)  
- TMP36 (A0), Tilt (D7), Light (D13), Buzzer (A5)  
- LCD 16×2 (parallel, RS=12, E=11, D4=5, D5=4, D6=3, D7=2, RW=GND, VO=pot)  
- Breadboard, jumpers, 470–1000 µF cap on servo rail

> 📸 See docs/wiring-diagram.png for the exact connections.

---

## 🧭 Pin Map (current build)
| Part | Pin |
|---|---|
| Servos | D6, D10, D8, A1, A2, A3 |
| Ultrasonic (front, single-wire) | D9 |
| Tilt (SW-200D, INPUT_PULLUP) | D7 |
| Light sensor (digital) | D13 |
| TMP36 | A0 |
| Buzzer | A5 |
| LCD RS,E,D4–D7 | D12, D11, D5, D4, D3, D2 |

---

## 🚀 Run It
1. Open **Aggressive-Bot-v2.ino** in Arduino IDE.
2. Board: *Arduino Uno*.
3. Libraries: built-ins (Servo, LiquidCrystal).
4. Upload.  
5. (Optional) In code, set DEMO_MODE 1 for auto cycling, 0 for sensor-driven.

---

## 🎥 Demo
- YouTube: (add your link)  
- Backup (Drive): (add your link)

---

## 🖼 Screenshots
- Circuit: docs/wiring-diagram.png  
- LCD output: docs/lcd.jpg (optional)

---

## 🛣 Roadmap
- [ ] Add DC motors back (move LCD to I²C backpack / PCA9685 for servos)  
- [ ] IMU (MPU6050) for balance & gait control  
- [ ] Bluetooth (HC-05) manual override  
- [ ] Webots/Unity 3D model for full virtual demo

---

## 📝 Notes
- Use a *separate 5–6 V regulator* for servos; *common GND* with Arduino.  
- Add 470–1000 µF across servo rail to reduce brownouts.  
- Tinkercad I²C parts are flaky; LCD is parallel here on purpose.

---

## 🧑‍💻 Author
*Jarfulavath Shivakumar* — Tinkercad build + Arduino integration  
GitHub: https://github.com/shivajsk-hub

## 📄 License
MIT — see [LICENSE](LICENSE).
