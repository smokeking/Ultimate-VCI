🚀 EG Ultimate VCI by Gelderblom
A complete, all-in-one vehicle communication interface firmware designed for the LilyGo T-CAN485 development board. This firmware enables your hardware to bridge physical vehicle networks (CAN-Bus and K-Line) to a PC or mobile device via high-speed USB-C, Bluetooth, and WiFi simultaneously.

🎯 Key Feature: It includes a live-switching profile engine that spoofs MAC addresses and broadcast names, allowing the hardware to emulate multiple high-end dealer tools.

⚡ Features
Tri-Interface Bridging: Streams diagnostic data simultaneously over USB-C (Serial), Bluetooth Serial, and WiFi TCP (Port 35000).

Dealer Tool Emulation: Built-in profiles with unique MAC addresses and names to satisfy strict vendor software checks.

Web Dashboard: Accessible at http://192.168.4.1 to swap profiles on the fly or fire manual wake-up pulses.

Protocol Support:
CAN-bus (SLCAN/Lawicel): Standard 250k/500k speeds with ISO-TP flow control handling.

K-Line: Traditional 10.4k baud operation with hardware line toggling.

Manual Wake-up Sequences: Trigger Fast Init and 5-Baud pulses via the web GUI to wake up stubborn legacy ECUs.

🚘 Supported Profiles
Profile Name	Broadcast Identity	Target Ecosystem	Protocol
EG Volvo DiCE	DiCE-206751	Volvo VIDA	CAN (500k)
EG Renault Alliance	Alliance-VI	Renault Clip	CAN (500k)
EG VAG HEX-NET	HEX-NET-V2	VCDS / VagCom	CAN (500k)
EG GM MDI	MDI-824512	GM GDS2 / Techline	CAN (500k)
EG BMW ICOM	ICOM-NEXT	BMW ISTA	CAN (500k)
EG Toyota Techstream	TIS-VCI	Toyota Techstream	CAN (500k)
EG Ford VCM II	VCM-II-PRO	Ford IDS	CAN (500k)
EG Renault Legacy	Renault-K	Renault DDT2000	K-Line (10.4k)
EG VAG K-Line	VAG-KKL	VCDS-Lite	K-Line (10.4k)
EG Generic OBD2	OBD2-ADAPTER	Generic ELM Apps	CAN (500k)

🛠️ Hardware Requirements
Board: LilyGo T-CAN485 (ESP32-based).
K-Line Hardware modification:

A transistor shifter on Pin 17 (TX) to pull up/down the vehicle's 12V K-Line.

A voltage divider on Pin 16 (RX) to safely read the 12V line at ESP32-friendly 3.3V levels.

💻 Software Setup
1. Arduino IDE
Open the Arduino IDE.

Install the ESP32 board package.

Install the following libraries via the Library Manager or GitHub:

ESPAsyncWebServer
AsyncTCP

Select your ESP32 board and flash the .ino sketch provided in this repository.

3. Windows Registry Fix (.reg)
Many legacy dealer apps expect to talk to low-index COM ports and require specific keys to recognize third-party PassThru devices.

Double-click the provided VCI_Fix.reg file.
Merge the file into your registry to clear standard COM limits and register the device under PassThruSupport.04.04.

🕹️ How to Use
Connect the board to your vehicle's OBD-II port.
Search for WiFi networks on your phone or PC and connect to EG_Ultimate_VCI (Password: admin123).

Open a browser and navigate to 192.168.4.1.
Click on your desired profile. The device will save the setting and automatically reboot into its new hardware identity.

If using a K-Line profile, use the red "Trigger Wakeup" button on the web page to initialize communication with the vehicle ECU.
