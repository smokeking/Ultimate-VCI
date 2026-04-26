# 🏎️ LilyGo T-CAN485 Ultimate VCI

An open-source, multi-mode Vehicle Communication Interface (VCI) built on the **LilyGo T-CAN485 (ESP32)**. This project emulates professional diagnostic tools (Volvo DiCE, GM MDI, Renault Alliance, VAG HEX-NET) for use with dealer-level software.

## 🚀 Features
- **Multi-Mode Connectivity:** Supports USB-C, WiFi (Access Point), and Bluetooth Serial.
- **Vendor Spoofing:** Change Bluetooth MAC and Device Name via a web dashboard to trick dealer software.
- **Dual Protocol:** High-speed CAN-bus (500k/250k) and K-Line (10.4k) support.
- **Web Dashboard:** On-the-fly profile switching and manual ECU "Wake-up" (5-baud/Fast Init) at `192.168.4.1`.
- **SLCAN Compatible:** Works natively with **SavvyCAN** and J2534 PassThru wrappers.

## 🛠️ Hardware Setup
### 1. CAN-Bus
Uses the onboard SN65HVD231 transceiver.
- **CAN_H:** OBD2 Pin 6
- **CAN_L:** OBD2 Pin 14

### 2. K-Line Shifter (Required for Older Cars)
Since the ESP32 is 3.3V and K-Line is 12V, we need a simple transistor level shifter:

**TX (GPIO 17):** NPN Transistor to pull OBD Pin 7 to GND.
**RX (GPIO 16):** Voltage divider (22k/10k) to drop 12V K-Line to 3.3V.

**Pull-up:** 1k Ohm resistor from OBD Pin 7 to 12V.

## Software Installation
1. **Flash Firmware:** Upload the provided `.ino` file to your LilyGo T-CAN485 using the Arduino IDE (Requires `ESPAsyncWebServer` and `AsyncTCP` libraries).
2. **Install Driver:** 
   - Download the `slcan-j2534.dll` 
   - Place it in `C:\Windows\System32\`.
3. **Registry Fix:** Run the included `vci_registry.reg` file to register the device as a PassThru interface in Windows.

## 📱 How to Use
1. Connect to the WiFi network: **VCI_ULTIMATE_DASH** (Pass: `admin123`).
2. Open `192.168.4.1` in your browser.
3. Select your desired vehicle profile (e.g., Renault Talisman or Volvo DiCE).
4. The device will reboot with the spoofed MAC/Name.
5. Open your diagnostic software (VIDA, GDS2, VCDS-Lite) and select **LilyGo Ultimate VCI**.

## 📂 Project Structure
- `/Firmware`: ESP32 Source code.
- `/Drivers`: Registry files and J2534 DLLs.
- `/Hardware`: Wiring diagrams for the K-Line shifter.

## 📝 Change Log
- **v1.2-alpha:** Added Web Dashboard and Manual K-Line Wake-up pulse.
- **v1.1-alpha:** Added K-Line support and Transistor Shifter logic.
- **v1.0-alpha:** Initial release with CAN-to-Bluetooth/USB bridge.
