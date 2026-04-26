/*
 * ====================================================================
 * Project: EG Ultimate VCI by Gelderblom
 * Features: USB-C, WiFi (AP), Bluetooth (Spoofed MAC/Name)
 * Protocols: CAN-bus (SLCAN) & K-Line (10.4k)
 * Web Control: IP 192.168.4.1 for Profile switching and Wake-up
 * ====================================================================
 */

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <BluetoothSerial.h>
#include "driver/twai.h"
#include <Preferences.h>
#include <esp_bt_main.h>
#include <esp_bt_device.h>

// --- Hardware Pins (LilyGo T-CAN485) ---
#define CAN_TX_PIN 27
#define CAN_RX_PIN 26
#define CAN_SE_PIN 23 
#define KLINE_TX 17 // Connected to Transistor Shifter
#define KLINE_RX 16 // Connected to Voltage Divider

AsyncWebServer server(80);
BluetoothSerial SerialBT;
Preferences prefs;
WiFiServer wifiServer(35000); // Standard J2534 TCP Port
WiFiClient wifiClient;

// --- Profile Database (All thinkable standards & vendors) ---
struct Profile {
  const char* name;
  uint8_t mac[6];
  const char* bt_name;
  bool useKLine;
  uint32_t speed; // CAN Speed in kbps
};

Profile profiles[] = {
  {"EG Volvo DiCE", {0x00,0x0E,0xCF,0xDE,0xAD,0x01}, "DiCE-206751", false, 500},
  {"EG Renault Alliance", {0x00,0x05,0xB0,0xDE,0xAD,0x02}, "Alliance-VI", false, 500},
  {"EG VAG HEX-NET", {0x00,0x1D,0xA5,0xDE,0xAD,0x03}, "HEX-NET-V2", false, 500},
  {"EG GM MDI", {0x00,0x09,0x90,0xDE,0xAD,0x04}, "MDI-824512", false, 500},
  {"EG BMW ICOM", {0x00,0x1A,0x4B,0xDE,0xAD,0x05}, "ICOM-NEXT", false, 500},
  {"EG Toyota Techstream", {0x00,0x10,0x60,0xDE,0xAD,0x06}, "TIS-VCI", false, 500},
  {"EG Ford VCM II", {0x00,0x0D,0x3A,0xDE,0xAD,0x07}, "VCM-II-PRO", false, 500},
  {"EG Renault Legacy", {0x00,0x05,0xB0,0xDE,0xAD,0x08}, "Renault-K", true, 10400},
  {"EG VAG K-Line", {0x00,0x1D,0xA5,0xDE,0xAD,0x09}, "VAG-KKL", true, 10400},
  {"EG Generic OBD2", {0x00,0x11,0x22,0x33,0x44,0x55}, "OBD2-ADAPTER", false, 500}
};

int activeID = 0;

// --- Helper: Send data to all active interfaces ---
void broadcastToPC(String data) {
  Serial.print(data);       // USB-C
  SerialBT.print(data);     // Bluetooth
  if (wifiClient.connected()) {
    wifiClient.print(data); // WiFi TCP
  }
}

// --- ISO-TP Flow Control Handling ---
void handleISOTP(twai_message_t *msg) {
  if ((msg->data[0] >> 4) == 0x01) { // First Frame detected
    twai_message_t fc;
    fc.identifier = msg->identifier + 0x08; // Response offset
    fc.extd = msg->extd;
    fc.data_length_code = 8;
    fc.data[0] = 0x30; // Continue
    fc.data[1] = 0x00; // Block Size 0
    fc.data[2] = 0x00; // Separation Time 0
    twai_transmit(&fc, pdMS_TO_TICKS(10));
  }
}

// --- Manual Wake-Up Pulse (Fast Init & 5-Baud) ---
void triggerWake() {
  Serial.println("K-Line Wake-up pulse triggered.");
  Serial2.end(); // Stop UART to bit-bang
  pinMode(KLINE_TX, OUTPUT);
  
  // Fast Init (Standard for Renault/VAG)
  digitalWrite(KLINE_TX, LOW);
  delay(25); // 25ms Low pulse
  digitalWrite(KLINE_TX, HIGH);
  delay(25);
  
  Serial2.begin(10400, SERIAL_8N1, KLINE_RX, KLINE_TX);
}

void setup() {
  // 1. Core Serial (USB-C Mode)
  Serial.begin(921600);

  // 2. Load Saved Preferences
  prefs.begin("master-vci", false);
  activeID = prefs.getInt("id", 0);

  // 3. Hardware Line Toggle
  pinMode(CAN_SE_PIN, OUTPUT);
  digitalWrite(CAN_SE_PIN, profiles[activeID].useKLine ? HIGH : LOW);

  // 4. Bluetooth Spoofing
  esp_base_mac_addr_set(profiles[activeID].mac);
  SerialBT.begin(profiles[activeID].bt_name);

  // 5. WiFi Access Point & Web Server Setup
  WiFi.softAP("EG_Ultimate_VCI", "admin123");
  wifiServer.begin();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *f){
    String h = "<html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    h += "<style>button{width:100%;padding:15px;margin:5px;font-size:16px;} body{font-family:sans-serif;text-align:center;}</style></head><body>";
    h += "<h2>EG Ultimate VCI Controller</h2>";
    h += "<p>Mode: <b style='color:green;'>" + String(profiles[activeID].name) + "</b></p>";
    
    if(profiles[activeID].useKLine) {
      h += "<a href='/wake'><button style='background-color:red; color:white;'>TRIGGER K-LINE WAKEUP</button></a><hr>";
    }

    for(int i=0; i < (sizeof(profiles)/sizeof(Profile)); i++) {
      h += "<a href='/set?i="+String(i)+"'><button>"+String(profiles[i].name)+"</button></a><br>";
    }
    h += "</body></html>";
    f->send(200, "text/html", h);
  });

  server.on("/wake", HTTP_GET, [](AsyncWebServerRequest *f){
    triggerWake();
    f->send(200, "text/plain", "K-Line wake pulse sent!");
  });

  server.on("/set", HTTP_GET, [](AsyncWebServerRequest *f){
    prefs.putInt("id", f->getParam("i")->value().toInt());
    f->send(200, "text/plain", "Profile Saved. Rebooting...");
    delay(1000); ESP.restart();
  });

  server.begin();

  // 6. Protocol Specific Setup
  if(profiles[activeID].useKLine) {
    Serial2.begin(10400, SERIAL_8N1, KLINE_RX, KLINE_TX);
  } else {
    twai_general_config_t g = TWAI_GENERAL_CONFIG_DEFAULT((gpio_num_t)CAN_TX_PIN, (gpio_num_t)CAN_RX_PIN, TWAI_MODE_NORMAL);
    twai_timing_config_t t = (profiles[activeID].speed == 250) ? TWAI_TIMING_CONFIG_250KBITS() : TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t fc = TWAI_FILTER_CONFIG_ACCEPT_ALL();
    twai_driver_install(&g, &t, &fc);
    twai_start();
  }
}

void loop() {
  // Handle continuous WiFi listening
  if (!wifiClient.connected()) {
    wifiClient = wifiServer.available();
  }

  // --- INTERFACE 1: CAN-BUS MODE ---
  if(!profiles[activeID].useKLine) {
    twai_message_t m;
    if(twai_receive(&m, 1) == ESP_OK) {
      handleISOTP(&m); // Handle multi-frame requests

      // SLCAN/Lawicel String Building
      String s = (m.extd ? "T" : "t");
      char b[16];
      if (m.extd) sprintf(b, "%08X", m.identifier);
      else sprintf(b, "%03X", m.identifier);
      s += b;
      s += m.data_length_code;
      for(int i=0; i<m.data_length_code; i++) {
        sprintf(b, "%02X", m.data[i]);
        s += b;
      }
      s += "\r";
      broadcastToPC(s);
    }
  } 
  
  // --- INTERFACE 2: K-LINE MODE ---
  else {
    if(Serial2.available()) { 
      uint8_t c = Serial2.read(); 
      Serial.write(c); 
      SerialBT.write(c); 
      if (wifiClient.connected()) wifiClient.write(c);
    }
    // Echo serial back down the K-Line
    if(SerialBT.available()) Serial2.write(SerialBT.read());
    if(Serial.available()) Serial2.write(Serial.read());
    if(wifiClient.connected() && wifiClient.available()) Serial2.write(wifiClient.read());
  }
}
