#include <Wire.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "homepage.h"

#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

#include "MAX30105.h"
#include "spo2_algorithm.h"

/*********** BLYNK ***********/
#define BLYNK_TEMPLATE_ID   "TMPL4R8ux29kx"
#define BLYNK_TEMPLATE_NAME "Quickstart Template"
#define BLYNK_AUTH_TOKEN    "YOUR_BLYNK_TOKEN"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";
BlynkTimer timer;

/*********** TFT PINS ***********/
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4
Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

/*********** COLORS ***********/
#define MED_GREEN  0x0648
#define MED_WHITE  0xFFFF
#define MED_BLACK  0x0000
#define MED_RED    0xF800
#define PURE_GREEN 0x07E0

/*********** BUTTONS ***********/
#define BTN1_PIN 14
#define BTN2_PIN 13

bool lastBtn1 = HIGH;
bool lastBtn2 = HIGH;

/*********** SPEAKER ***********/
#define SPEAKER_PIN 25

const int CPR_BPM = 110;
const int CPR_INTERVAL = 60000 / CPR_BPM;
unsigned long lastBeatTime = 0;

/*********** GPS ***********/
TinyGPSPlus gps;
HardwareSerial GPSSerial(2);

#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_BAUD   9600

double lat = 0.0;
double lon = 0.0;
float alt_m = 0.0;
int sats = 0;
bool gpsFix = false;

/*********** MAX30102 ***********/
MAX30105 particleSensor;

#define BUFFER_SIZE 100
uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

int32_t spo2 = 0;
int8_t validSPO2 = 0;

int32_t heartRate = 0;
int8_t validHeartRate = 0;

/*********** STATUS ***********/
String statusText = "Booting";

/*********** DECISION TREE ***********/
int screenIndex = 0;
int lastScreenIndex = -1;

int currentStep = 1;
String currentStepName = "Safety";

struct Screen {
  const char* title;
  const char* line1;
  const char* line2;
  const char* opt1;
  const char* opt2;
};

Screen screens[] = {
  { "Safety",    "Is it safe to",      "approach?",          "1) Yes", "2) No" },
  { "Response",  "Responsive?",        "Talk + tap",         "1) Yes", "2) No" },
  { "Resp Check","Severe symptoms?",   "Pain / bleeding?",   "1) Yes", "2) No" },
  { "Call Help", "Shout for help.",    "Call 112 / 999",     "BTN1:Next", "" },
  { "Breathing", "Breathing normal?",  "Check 10 sec",       "1) Yes", "2) No" },
  { "Recovery",  "Recovery position.", "Keep airway open",   "BTN1:Next", "" },
  { "CPR",       "Start CPR now.",     "100-120 / min",      "BTN1:Next", "" },
  { "AED",       "AED available?",     "Attach if yes",      "1) Yes", "2) No" },
  { "Assess",    "Keep them calm.",    "Ask what happened",  "BTN1:Next", "" },
  { "Monitor",   "Monitor patient.",   "If worse -> CPR",    "BTN1:Next", "" },
  { "End",       "Guide complete.",    "Await ambulance",    "BTN1:Restart", "" }
};

/*********** WEBSERVER ***********/
WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", homePage);
}

void handleSensors() {
  int outPulse = (validHeartRate && heartRate > 0) ? heartRate : -1;
  int outSpO2  = (validSPO2 && spo2 > 0) ? spo2 : -1;

  String json = "{";
  json += "\"pulse\": " + String(outPulse) + ",";
  json += "\"spo2\": " + String(outSpO2) + ",";
  json += "\"lat\": " + String(lat, 6) + ",";
  json += "\"lon\": " + String(lon, 6) + ",";
  json += "\"gpsFix\": " + String(gpsFix ? 1 : 0) + ",";
  json += "\"sats\": " + String(sats) + ",";
  json += "\"step\": " + String(currentStep) + ",";
  json += "\"stepName\": \"" + currentStepName + "\",";
  json += "\"status\": \"" + statusText + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void handleNotFound() {
  server.send(404, "text/plain", "File Not Found");
}

/*********** TFT UI ***********/
void drawScreen(int i) {
  currentStep = i + 1;
  currentStepName = screens[i].title;

  tft.fillScreen(MED_WHITE);
  tft.setTextWrap(true);

  tft.fillRect(0, 0, 160, 30, PURE_GREEN);
  tft.setTextSize(2);
  tft.setTextColor(MED_WHITE);
  tft.setCursor(10, 8);
  tft.print(screens[i].title);

  tft.setTextSize(1);
  tft.setTextColor(MED_BLACK);
  tft.setCursor(10, 45);
  tft.print(screens[i].line1);
  tft.setCursor(10, 58);
  tft.print(screens[i].line2);

  if (i == 3) {
    tft.setCursor(10, 72);
    tft.print("Location:");

    tft.setCursor(10, 84);
    if (gpsFix) {
      tft.print(lat, 4);
      tft.print(", ");
      tft.print(lon, 4);
    } else {
      tft.print("Searching GPS...");
    }
  }

  if (i == 9) {
    tft.setCursor(90, 72);
    tft.setTextColor(MED_RED);
    tft.print("Pulse:");

    tft.setCursor(90, 84);
    tft.setTextColor(MED_BLACK);

    if (validHeartRate && heartRate > 0) {
      tft.print(heartRate);
      tft.print(" bpm");
    } else {
      tft.print("-- bpm");
    }

    tft.setCursor(90, 96);
    tft.print("SpO2:");

    tft.setCursor(90, 108);
    if (validSPO2 && spo2 > 0) {
      tft.print(spo2);
      tft.print("%");
    } else {
      tft.print("--%");
    }
  }

  tft.setTextColor(MED_GREEN);

  if (i == 3 || i == 9) {
    tft.setCursor(20, 115);
    tft.print(screens[i].opt1);
  } else {
    tft.setCursor(20, 90);
    tft.print(screens[i].opt1);
    tft.setCursor(20, 110);
    tft.print(screens[i].opt2);
  }
}

/*********** GPS ***********/
void updateGPS() {
  while (GPSSerial.available() > 0) {
    gps.encode(GPSSerial.read());
  }

  if (gps.location.isUpdated()) {
    gpsFix = gps.location.isValid();

    if (gpsFix) {
      lat = gps.location.lat();
      lon = gps.location.lng();
    }
  }

  if (gps.altitude.isUpdated()) {
    alt_m = gps.altitude.meters();
  }

  if (gps.satellites.isUpdated()) {
    sats = gps.satellites.value();
  }
}

/*********** MAX30102 ***********/
bool fingerPresent() {
  return particleSensor.getIR() > 50000;
}

void computeSpO2AndHR() {
  for (int i = 0; i < BUFFER_SIZE; i++) {
    while (particleSensor.available() == false) {
      particleSensor.check();
      updateGPS();
      Blynk.run();
      timer.run();
      server.handleClient();
    }

    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(
    irBuffer,
    BUFFER_SIZE,
    redBuffer,
    &spo2,
    &validSPO2,
    &heartRate,
    &validHeartRate
  );
}

/*********** CPR METRONOME ***********/
void runCprMetronome() {
  unsigned long now = millis();

  if (now - lastBeatTime >= CPR_INTERVAL) {
    lastBeatTime = now;

    ledcWriteTone(SPEAKER_PIN, 2000);
    ledcWrite(SPEAKER_PIN, 160);
    delay(50);
    ledcWrite(SPEAKER_PIN, 0);
  }
}

/*********** BLYNK ***********/
void sendToBlynk() {
  Blynk.virtualWrite(V0, (validHeartRate && heartRate > 0) ? (int)heartRate : -1);
  Blynk.virtualWrite(V1, (validSPO2 && spo2 > 0) ? (int)spo2 : -1);
  Blynk.virtualWrite(V2, lat);
  Blynk.virtualWrite(V3, lon);
  Blynk.virtualWrite(V4, sats);
  Blynk.virtualWrite(V5, gpsFix ? 1 : 0);
  Blynk.virtualWrite(V6, statusText);
  Blynk.virtualWrite(V7, currentStepName);
}

BLYNK_CONNECTED() {
  Serial.println("Blynk connected!");
}

/*********** BUTTON LOGIC ***********/
void updateDecisionTree() {
  bool btn1Now = digitalRead(BTN1_PIN);
  bool btn2Now = digitalRead(BTN2_PIN);

  bool btn1Pressed = (lastBtn1 == HIGH && btn1Now == LOW);
  bool btn2Pressed = (lastBtn2 == HIGH && btn2Now == LOW);

  switch (screenIndex) {
    case 0:
      if (btn1Pressed) screenIndex = 1;
      if (btn2Pressed) screenIndex = 10;
      break;

    case 1:
      if (btn1Pressed) screenIndex = 2;
      if (btn2Pressed) screenIndex = 3;
      break;

    case 2:
      if (btn1Pressed) screenIndex = 8;
      if (btn2Pressed) screenIndex = 3;
      break;

    case 3:
      if (btn1Pressed) screenIndex = 4;
      break;

    case 4:
      if (btn1Pressed) screenIndex = 5;
      if (btn2Pressed) screenIndex = 6;
      break;

    case 5:
      if (btn1Pressed) screenIndex = 9;
      break;

    case 6:
      if (btn1Pressed) screenIndex = 7;
      break;

    case 7:
      if (btn1Pressed) screenIndex = 6;
      if (btn2Pressed) screenIndex = 6;
      break;

    case 8:
      if (btn1Pressed) screenIndex = 9;
      break;

    case 9:
      if (btn1Pressed) screenIndex = 6;
      if (btn2Pressed) screenIndex = 10;
      break;

    case 10:
      if (btn1Pressed) screenIndex = 0;
      break;
  }

  lastBtn1 = btn1Now;
  lastBtn2 = btn2Now;
}

/*********** SETUP ***********/
void setup() {
  Serial.begin(115200);
  delay(300);

  Serial.println("First Aid Assistant starting...");

  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3);

  pinMode(BTN1_PIN, INPUT_PULLUP);
  pinMode(BTN2_PIN, INPUT_PULLUP);

  ledcAttach(SPEAKER_PIN, 1000, 8);
  ledcWrite(SPEAKER_PIN, 0);

  Wire.begin(21, 22);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 NOT FOUND");
    statusText = "MAX30102 missing";
  } else {
    Serial.println("MAX30102 found");
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x1F);
    particleSensor.setPulseAmplitudeIR(0x1F);
    particleSensor.setPulseAmplitudeGreen(0);
    statusText = "Sensor OK";
  }

  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS started");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  timer.setInterval(1000L, sendToBlynk);

  server.on("/", handleRoot);
  server.on("/sensors", handleSensors);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("HTTP server started");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  drawScreen(screenIndex);
  lastScreenIndex = screenIndex;
}

/*********** LOOP ***********/
void loop() {
  Blynk.run();
  timer.run();
  server.handleClient();

  updateGPS();
  updateDecisionTree();

  static unsigned long lastCompute = 0;
  if (millis() - lastCompute > 5000) {
    lastCompute = millis();

    if (fingerPresent()) {
      computeSpO2AndHR();
      statusText = gpsFix ? "OK" : "No GPS fix";

      Serial.print("HR: ");
      Serial.print(validHeartRate ? heartRate : -1);
      Serial.print(" SpO2: ");
      Serial.print(validSPO2 ? spo2 : -1);
      Serial.print(" GPS Fix: ");
      Serial.println(gpsFix ? "YES" : "NO");
    } else {
      validHeartRate = 0;
      validSPO2 = 0;
      statusText = "No finger";
      Serial.println("No finger detected");
    }
  }

  if (screenIndex == 6) {
    runCprMetronome();
  } else {
    ledcWrite(SPEAKER_PIN, 0);
    ledcWriteTone(SPEAKER_PIN, 0);
    lastBeatTime = millis();
  }

  static unsigned long lastScreenRefresh = 0;

  if (screenIndex != lastScreenIndex || millis() - lastScreenRefresh > 1000) {
    drawScreen(screenIndex);
    lastScreenIndex = screenIndex;
    lastScreenRefresh = millis();
  }
}