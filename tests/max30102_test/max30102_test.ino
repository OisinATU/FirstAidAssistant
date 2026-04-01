#include <Wire.h>
#include "MAX30105.h"
#include "spo2_algorithm.h"

MAX30105 particleSensor;

const byte BUFFER_LENGTH = 100;
uint32_t irBuffer[BUFFER_LENGTH];
uint32_t redBuffer[BUFFER_LENGTH];
int32_t spo2; 
int8_t validSPO2;
int32_t heartRate;
int8_t validHeartRate;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 NOT FOUND");
    while (1);
  }

  particleSensor.setup();
  particleSensor.setPulseAmplitudeRed(0x1F);
  particleSensor.setPulseAmplitudeIR(0x1F);

  Serial.println("Place your finger on the sensor.");
}

void loop() {
  // Collect 100 samples
  for (int i = 0; i < BUFFER_LENGTH; i++) {
    redBuffer[i] = particleSensor.getRed();
    irBuffer[i] = particleSensor.getIR();
    delay(20);
  }

  // Run the algorithm
  maxim_heart_rate_and_oxygen_saturation(
    irBuffer, BUFFER_LENGTH,
    redBuffer,
    &spo2, &validSPO2,
    &heartRate, &validHeartRate
  );

  Serial.print("HR: ");
  if (validHeartRate)
    Serial.print(heartRate);
  else
    Serial.print("--");

  Serial.print("   SpO2: ");
  if (validSPO2)
    Serial.println(spo2);
  else
    Serial.println("--");

  delay(200);
}
