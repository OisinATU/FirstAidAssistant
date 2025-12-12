#include <TinyGPSPlus.h>
#include <HardwareSerial.h>

TinyGPSPlus gps;
HardwareSerial GPSSerial(2); 

#define RXPin 16 // GPS TX
#define TXPin 17 // GPS RX
#define GPSBaud 9600

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 + GY-NEO6M GPS Test");

  GPSSerial.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin);
}

void loop() {
  while (GPSSerial.available() > 0) {
    gps.encode(GPSSerial.read());
  }

  if (gps.location.isUpdated()) {
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);

    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);

    Serial.print("Satellites: ");
    Serial.println(gps.satellites.value());

    Serial.print("HDOP: ");
    Serial.println(gps.hdop.value());

    Serial.print("Altitude: ");
    Serial.println(gps.altitude.meters());

    Serial.println("------------------------");
  }
}
