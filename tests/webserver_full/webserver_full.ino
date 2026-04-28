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
#define BLYNK_TEMPLATE_ID           "TMPL4R8ux29kx"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_AUTH_TOKEN "rjnqEU1McO9eAcAB1xfur04DDrVEu7nK"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

char ssid[] = "OisinrPhone";
char pass[] = "Password9";
BlynkTimer timer;

/*********** TFT (ST7735) ***********/
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4
Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

/*********** GPS (NEO-6M) ***********/
TinyGPSPlus gps;
HardwareSerial GPSSerial(2);
#define GPS_RX_PIN 16
#define GPS_TX_PIN 17
#define GPS_BAUD   9600

double lat = 0.0, lon = 0.0;
float  alt_m = 0.0;
int    sats = 0;
bool   gpsFix = false;

/*********** MAX30102 ***********/
MAX30105 particleSensor;

#define BUFFER_SIZE 100
uint32_t irBuffer[BUFFER_SIZE];
uint32_t redBuffer[BUFFER_SIZE];

int32_t spo2 = 0;
int8_t  validSPO2 = 0;

int32_t heartRate = 0;
int8_t  validHeartRate = 0;

/*********** Timing ***********/
unsigned long lastTftUpdate = 0;
const unsigned long TFT_INTERVAL_MS = 500;

/*********** Status ***********/
String statusText = "Booting";

/*********** DEMO MODE ***********/
bool demoMode = true;

// Hardcoded demo values for poster / screenshot
double demoLat = 53.2707;
double demoLon = -9.0568;
int demoPulse = 78;
int demoSpO2 = 97;

// Decision tree state
int currentStep = 1;
String currentStepName = "Safety Check";

/****** Webserver ******/
WebServer server(80);

void handleRoot() {
  server.send(200, "text/html", homePage);
}

void handleSensors() {
  int outPulse = demoMode ? demoPulse : ((validHeartRate && heartRate > 0) ? heartRate : -1);
  int outSpO2  = demoMode ? demoSpO2  : ((validSPO2 && spo2 > 0) ? spo2 : -1);
  double outLat = demoMode ? demoLat : lat;
  double outLon = demoMode ? demoLon : lon;

  String json = "{";
  json += "\"pulse\": " + String(outPulse) + ",";
  json += "\"spo2\": " + String(outSpO2) + ",";
  json += "\"lat\": " + String(outLat, 6) + ",";
  json += "\"lon\": " + String(outLon, 6) + ",";
  json += "\"step\": " + String(currentStep) + ",";
  json += "\"stepName\": \"" + currentStepName + "\",";
  json += "\"status\": \"" + statusText + "\"";
  json += "}";

  server.send(200, "application/json", json);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

/*********** UI ***********/
void drawUIFrame() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setRotation(1);
  tft.setTextWrap(false);

  tft.setTextSize(2);
  tft.setTextColor(ST77XX_GREEN);
  tft.setCursor(6, 6);
  tft.print("First Aid");
  tft.setCursor(6, 28);
  tft.print("Assistant");

  tft.drawLine(0, 50, 160, 50, ST77XX_WHITE);

  tft.setTextSize(1);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(6, 55);
  tft.print("Pulse/Oxygen");

  tft.setTextColor(ST77XX_CYAN);
  tft.setCursor(6, 95);
  tft.print("GPS");

  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(6, 67);  tft.print("HR:   ");
  tft.setCursor(6, 79);  tft.print("SpO2: ");

  tft.setCursor(6, 107); tft.print("Lat:  ");
  tft.setCursor(6, 118); tft.print("Lon:  ");
  tft.setCursor(6, 129); tft.print("Alt:  ");
}

void updateTFT() {
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE, ST77XX_BLACK);

  tft.setCursor(35, 67);
  if (validHeartRate && heartRate > 0) tft.printf("%3ld bpm   ", (long)heartRate);
  else                                 tft.print("-- bpm    ");

  tft.setCursor(45, 79);
  if (validSPO2 && spo2 > 0) tft.printf("%3ld %%   ", (long)spo2);
  else                       tft.print("-- %     ");

  tft.setCursor(35, 107);
  if (gpsFix) tft.printf("%8.5f ", lat);
  else        tft.print("searching ");

  tft.setCursor(35, 118);
  if (gpsFix) tft.printf("%8.5f ", lon);
  else        tft.print("          ");

  tft.setCursor(35, 129);
  if (gpsFix) tft.printf("%4.0f m  S:%02d", alt_m, sats);
  else        tft.printf("-- m  S:%02d", sats);
}

/*********** GPS update ***********/
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
  if (gps.altitude.isUpdated())   alt_m = gps.altitude.meters();
  if (gps.satellites.isUpdated()) sats = gps.satellites.value();
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
    irBuffer[i]  = particleSensor.getIR();
    particleSensor.nextSample();
  }

  maxim_heart_rate_and_oxygen_saturation(
      irBuffer, BUFFER_SIZE,
      redBuffer,
      &spo2, &validSPO2,
      &heartRate, &validHeartRate
  );
}

/*********** BLYNK SEND ***********/
void sendToBlynk() {
  Blynk.virtualWrite(V0, (validHeartRate && heartRate > 0) ? (int)heartRate : -1);
  Blynk.virtualWrite(V1, (validSPO2 && spo2 > 0) ? (int)spo2 : -1);

  Blynk.virtualWrite(V2, lat);
  Blynk.virtualWrite(V3, lon);
  Blynk.virtualWrite(V4, sats);
  Blynk.virtualWrite(V5, gpsFix ? 1 : 0);
  Blynk.virtualWrite(V6, statusText);
}

BLYNK_CONNECTED() {
  Serial.println("Blynk connected!");
}

/*********** SETUP ***********/
void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("System Integration: TFT + MAX30102 + GPS + Blynk");

  // TFT
  tft.initR(INITR_BLACKTAB);
  drawUIFrame();

  // I2C
  Wire.begin(21, 22);

  // MAX30102
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 NOT FOUND. Check wiring.");
    statusText = "MAX30102 missing";
  } else {
    Serial.println("MAX30102 found.");
    particleSensor.setup();
    particleSensor.setPulseAmplitudeRed(0x1F);
    particleSensor.setPulseAmplitudeIR(0x1F);
    particleSensor.setPulseAmplitudeGreen(0);
    statusText = "Sensor OK";
  }

  // GPS
  GPSSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX_PIN, GPS_TX_PIN);
  Serial.println("GPS started.");

  // Blynk
  Serial.println("Starting Blynk...");
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  Serial.println("Blynk connected!");

  // Send to Blynk every 1 second
  timer.setInterval(1000L, sendToBlynk);

  // Webserver
  server.on("/", handleRoot);
  server.on("/sensors", handleSensors);
  server.onNotFound(handleNotFound);
  server.begin();

  Serial.println("HTTP server started");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Demo startup values for poster
  if (demoMode) {
    statusText = "Demo Active";
    currentStep = 3;
    currentStepName = "Call Emergency Services";
  }
}

/*********** LOOP ***********/
void loop() {
  Blynk.run();
  timer.run();
  server.handleClient();

  updateGPS();

  static unsigned long lastCompute = 0;
  if (millis() - lastCompute > 5000) {
    lastCompute = millis();

    if (!demoMode) {
      if (fingerPresent()) {
        computeSpO2AndHR();
        statusText = gpsFix ? "OK" : "No GPS fix";

        // Example: move to a later step when live readings are active
        currentStep = 4;
        currentStepName = "Check Breathing";

        Serial.print("HR: "); Serial.print(validHeartRate ? heartRate : -1);
        Serial.print("  SpO2: "); Serial.print(validSPO2 ? spo2 : -1);
        Serial.print("  | GPS Fix: "); Serial.print(gpsFix ? "YES" : "NO");
        Serial.print("  Lat: "); Serial.print(lat, 6);
        Serial.print("  Lon: "); Serial.print(lon, 6);
        Serial.print("  Sats: "); Serial.println(sats);
      } else {
        validHeartRate = 0;
        validSPO2 = 0;
        statusText = "No finger";
        currentStep = 2;
        currentStepName = "Check Response";
        Serial.println("No finger detected on MAX30102.");
      }
    }
  }

  if (millis() - lastTftUpdate > TFT_INTERVAL_MS) {
    lastTftUpdate = millis();
    updateTFT();
  }
}