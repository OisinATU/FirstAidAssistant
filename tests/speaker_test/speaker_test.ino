#define SPEAKER_PIN 25

const int BPM = 110;
const int BEAT_MS = 60000 / BPM;   // 1 minute/ Beats
const int TICK_MS = 50;            // tick length
const int FREQ_HZ = 2000;          

void setup() {
  ledcAttach(SPEAKER_PIN, 1000, 8); // pin, base freq, resolution
  ledcWrite(SPEAKER_PIN, 0);
}

void loop() {
  // Tick
  ledcWriteTone(SPEAKER_PIN, FREQ_HZ);
  ledcWrite(SPEAKER_PIN, 160);
  delay(TICK_MS);

  // Silence for the rest of the beat
  ledcWrite(SPEAKER_PIN, 0);
  delay(BEAT_MS - TICK_MS);
}