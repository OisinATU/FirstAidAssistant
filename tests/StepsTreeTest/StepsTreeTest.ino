#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>

/*********** TFT PINS ***********/
#define TFT_CS   5
#define TFT_DC   2
#define TFT_RST  4

// Custom medical colors
#define MED_GREEN  0x0648
#define MED_WHITE   0xFFFF
#define MED_BLACK   0x0000
#define MED_RED     0xF800
#define PURE_GREEN  0x07E0

bool lastBtn1 = HIGH;
bool lastBtn2 = HIGH;


// Beeper
#define SPEAKER_PIN 25

const int CPR_BPM = 110;
const int CPR_INTERVAL = 60000 / CPR_BPM;   // about 545 ms
unsigned long lastBeatTime = 0;


#define BTN1_PIN 13
#define BTN2_PIN 14

int screenIndex = 6;

struct Screen {
  const char* title;
  const char* line1;
  const char* line2;
  const char* opt1;   // can be "" if not used
  const char* opt2;   // can be "" if not used
};


Screen screens[] = {

  { "Safety",    "Is it safe to",         "approach?",          "1) Yes", "2) No" },

  { "Response",  "Responsive?",           "Talk + tap",         "1) Yes", "2) No" },

  { "Resp Check","Severe symptoms?",      "Pain / bleeding?",   "1) Yes", "2) No" },

  { "Call Help", "Shout for help.",       "Call 112 / 999",     "BTN1:Next", ""   },

  { "Breathing", "Breathing normal?",     "Check 10 sec",       "1) Yes", "2) No" },

  { "Recovery",  "Recovery position.",    "Keep airway open",   "BTN1:Next", ""   },

  { "CPR",       "Start CPR now.",        "100-120 / min",      "BTN1:Next", ""   },

  { "AED",       "AED available?",        "Attach if yes",      "1) Yes", "2) No" },

  { "Assess",    "Keep them calm.",       "Ask what happened",  "BTN1:Next", ""   },

  { "Monitor",   "Monitor patient.",      "If worse -> CPR",    "BTN1:Next", ""   },

  { "End",       "Guide complete.",       "Await ambulance",    "BTN1:Restart", "" }
};

const int SCREEN_COUNT = sizeof(screens) / sizeof(screens[0]);



Adafruit_ST7735 tft(TFT_CS, TFT_DC, TFT_RST);

void drawScreen(int i) {
  tft.fillScreen(MED_WHITE);
  tft.setTextWrap(true);

  // Header
  tft.fillRect(0, 0, 160, 30, PURE_GREEN);
  tft.setTextSize(2);
  tft.setTextColor(MED_WHITE);
  tft.setCursor(10, 8);
  tft.print(screens[i].title);

  // Body
  tft.setTextSize(1);
  tft.setTextColor(MED_BLACK);
  tft.setCursor(10, 45);
  tft.print(screens[i].line1);
  tft.setCursor(10, 58);
  tft.print(screens[i].line2);

  // Special: Call Help screen (index 3)
  if (i == 3) {
    tft.setCursor(10, 72);
    tft.print("Location:");
    tft.setCursor(10, 84);
    tft.print("53.2707, -9.0568");  // fake location
  }

    // Special: Monitor screen (index 9)
  if (i == 9) {
    tft.setCursor(100, 74);
    tft.setTextColor(MED_RED);
    tft.print("Pulse:   ");
    tft.setCursor(100, 84);
    tft.setTextColor(MED_BLACK);
    tft.print(78);
    tft.print(" bpm");
  }

  // Options
  tft.setTextColor(MED_GREEN);

  // Move options slightly lower if Call Help screen
  if (i == 3 || i ==9) {
    tft.setCursor(20, 100);
    tft.print(screens[i].opt1);
    tft.setCursor(20, 115);
    tft.print(screens[i].opt2);
  } else {
    tft.setCursor(20, 90);
    tft.print(screens[i].opt1);
    tft.setCursor(20, 110);
    tft.print(screens[i].opt2);
  }
}


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

/*********** SETUP ***********/
void setup() {

  // Setup for the tft display. Three different options. Mine has a blacktab configuration.
  tft.initR(INITR_BLACKTAB);
  tft.setRotation(3); // 270 degree rotation

  pinMode(BTN1_PIN, INPUT_PULLUP); // Connected to internal pullup to give a definite high value
  pinMode(BTN2_PIN, INPUT_PULLUP);

  ledcAttach(SPEAKER_PIN, 1000, 8);
  ledcWrite(SPEAKER_PIN, 0);

  drawScreen(screenIndex);
}

/*********** LOOP ***********/
void loop() {

    // Syntax for cleaner code
    bool btn1 = digitalRead(BTN1_PIN) == LOW;
    bool btn2 = digitalRead(BTN2_PIN) == LOW;


switch(screenIndex) {

  case 0:
    if(btn1) screenIndex = 1;
    if(btn2) screenIndex = 10;
  break;

  case 1:
    if(btn1) screenIndex = 2;
    if(btn2) screenIndex = 3;
  break;


  case 2:
    if(btn1) screenIndex = 8;
    if(btn2) screenIndex = 3;
  break;

  case 3:
    if(btn1) screenIndex = 4;
  break;

  case 4:
    if(btn1) screenIndex = 5;
    if(btn2) screenIndex = 6;
  break;

  case 5:
    if(btn1) screenIndex = 9;
  break;

  case 6:
    if(btn1) screenIndex = 7;
  break;

  case 7:
    if(btn1) screenIndex = 6;
    if(btn2) screenIndex = 6;
  break;

  case 8:
    if(btn1) screenIndex = 9;
  break;

  case 9:
    if(btn1) screenIndex = 6;
    if(btn2) screenIndex = 10;
  break;

  case 10:
    if(btn1) screenIndex = 0;
  break;
}

  // behaviour based on screen
  if (screenIndex == 6) {
    runCprMetronome();
  } else {
    ledcWrite(SPEAKER_PIN, 0);
  }


}
