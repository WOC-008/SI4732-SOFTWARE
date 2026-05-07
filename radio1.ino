

#include <Wire.h>
#include <SI4735.h>
#include <LiquidCrystal_I2C.h>

// Pin definitions
#define BT_VOLUP 0
#define BT_VOLDOWN 1
#define PIN_MODE_BTN 4
#define PIN_PWR_SIM 6
#define PIN_BT_PWR 7

#define LONG_PRESS_MS 3000
#define RDS_MAX 150

const int PIN_ENC_FREQ_A = 2;
const int PIN_ENC_FREQ_B = 3;

volatile int encoderValue = 0;
volatile int lastEncoded = 0;


const float FM_MIN = 64.0;
const float FM_MAX = 108.0;
const float FM_STEP = 0.1;

const int AM_MIN = 520;
const int AM_MAX = 1710;
const int AM_STEP = 10;

const int SW_MIN = 2300;
const int SW_MAX = 26100;
const int SW_STEP = 5;

const int LW_MIN = 144;
const int LW_MAX = 285;
const int LW_STEP = 10;

const int volmin = 0;
const int volmax = 63;
const int volchange = 1;

const int stepDelay = 300;
const int holdDelay = 500;

//change address as requid
LiquidCrystal_I2C lcd(0x27, 16, 2);
SI4735 radio;

enum Mode {
  MODE_FM,
  MODE_AM,
  MODE_SW,
  MODE_LW,
  MODE_BT
};

Mode currentMode = MODE_FM;

float freqFM = 95.3;
int freqAM = 1000;
int freqSW = 16000;
int freqLW = 150;

int volStep = 40;
int lastVolStep = 0;
int dotCount = 0;
int scrollPos = 0;  // scrolling position
int btn = 0;
int lastbtn = 0;

unsigned long startupStart = 0;
unsigned long lastDotTime = 0;
unsigned long lastfreqCheck = 0;
unsigned long lastModeCheck = 0;
unsigned long modeBtnPressTime = 0;
unsigned long longpressTime = 0;
unsigned long pressStartTime = 0;
unsigned long lastlcdupdate = 0;
unsigned long lastupdatee = 0;
unsigned long actionint = 0;
unsigned long LAST_RDS_INTERVA = 0;
unsigned long wait = 0;
unsigned long volumeExitTime = 0;

const unsigned long muteCheckInterval = 100;
const unsigned long ModeCheckInterval = 75;
const unsigned long norm = 6000;
const unsigned long RDS_INTERVAL = 470;

bool radioIsOn = true;
bool btModeDisplayed = false;
bool run = true;
bool first = true;
bool isHolding = false;
bool Muted = false;
bool btMode = false;
bool startupInProgress = true;
bool modeBtnHeld = false;
bool volaction = false;
bool clrtogo = false;
bool VOLUME = false;
bool firstpress = false;
bool second_press = false;
bool THIRDPRESS = false;
bool volumeExitPending = false;
bool lastVolumeState = false;

char rdsText[RDS_MAX];  // buffer for full RDS text

int lastValue = 0;

void setup() {
  pinMode(PIN_ENC_FREQ_A, INPUT);
  pinMode(PIN_ENC_FREQ_B, INPUT);
  pinMode(A3, INPUT);
  pinMode(PIN_MODE_BTN, INPUT_PULLUP);
  pinMode(PIN_BT_PWR, OUTPUT);
  pinMode(PIN_PWR_SIM, OUTPUT);
  pinMode(BT_VOLUP, OUTPUT);
  pinMode(BT_VOLDOWN, OUTPUT);
  digitalWrite(PIN_BT_PWR, HIGH);
  digitalWrite(PIN_PWR_SIM, LOW);
  digitalWrite(BT_VOLUP, LOW);
  digitalWrite(BT_VOLDOWN, LOW);

  attachInterrupt(digitalPinToInterrupt(PIN_ENC_FREQ_A), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PIN_ENC_FREQ_B), updateEncoder, CHANGE);

  lcd.init();
  lcd.clear();
  delay(400);
  lcd.backlight();
  delay(1500);
  lcd.setCursor(3, 0);
  lcd.print("W");
  delay(25);
  lcd.print("E");
  delay(25);
  lcd.print("L");
  delay(25);
  lcd.print("C");
  delay(25);
  lcd.print("O");
  delay(25);
  lcd.print("M");
  delay(25);
  lcd.print("E");
  delay(800);
  lcd.setCursor(0, 1);
  lcd.print("E");
  delay(200);
  lcd.print("M");
  delay(200);
  lcd.print("M");
  delay(200);
  lcd.print("A");
  delay(85);
  lcd.print(" ");
  delay(200);
  lcd.print("C");
  delay(200);
  lcd.print("H");
  delay(200);
  lcd.print("U");
  delay(200);
  lcd.print("K");
  delay(200);
  lcd.print("W");
  delay(200);
  lcd.print("U");
  delay(200);
  lcd.print("E");
  delay(200);
  lcd.print("B");
  delay(200);
  lcd.print("U");
  delay(200);
  lcd.print("K");
  delay(200);
  lcd.print("A");
  delay(4000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("FOUR BAND RADIO+");
  lcd.setCursor(0, 1);
  lcd.print("BLUETOOTH PLAYER");
  delay(4360);

  startupStart = millis();

  radio.setup(5, 0);
  delay(600);
  radio.setFM();
  radio.setFrequency((uint16_t)(freqFM * 100));
  radio.setVolume(0);
}

void loop() {
  if (startupInProgress) {
    animateStartup();
    return;
  }

  if (first) {
    lcd.clear();
    volumedisplay();
    updateradioDisplay();
    first = false;
  }
  if (volumeExitPending) {
    if (millis() - volumeExitTime >= 2000) {
      volumeExitPending = false;
      if (!VOLUME) {
        updateradioDisplay();
      }
    }
  }

  handleModeButton();

  if (millis() - LAST_RDS_INTERVA >= RDS_INTERVAL) {
    LAST_RDS_INTERVA = millis();
    updateRDS();
  }

  int valueCopy;
  int diff;

  // safely copy interrupt value
  noInterrupts();
  valueCopy = encoderValue;
  interrupts();

  diff = (valueCopy - lastValue) / 4;

  if (diff != 0) {
    lastValue += diff * 4;

    if (VOLUME && !Muted) {
      handlevolumeControl(diff);
    } else if (!Muted || Muted) {
      handleFrequencyControl(diff);
    }
  }

  if (btMode) {
    if (radioIsOn) {
      radio.powerDown();
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("LOADING...");
      lcd.setCursor(0, 1);
      lcd.print("BLUETOOTH MODE");
      digitalWrite(PIN_BT_PWR, LOW);
      delay(800);
      simulatePowerButtonPress();
      radioIsOn = false;
    }
    if (!btModeDisplayed) {
      displayBTMode();
      btModeDisplayed = true;
    }
    return;
  }


  // Radio mode:
  if (!radioIsOn) {
    digitalWrite(PIN_BT_PWR, HIGH);
    btModeDisplayed = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RADIO MODE");
    radio.setup(5, 0);
    delay(1360);
    currentMode = MODE_FM;
    radio.setFM();
    radio.setFrequency((uint16_t)(freqFM * 100));
    updateRadio();
    lcd.clear();
    updateradioDisplay();
    int voldisp = (volStep * 1);
    lcd.setCursor(0, 1);
    lcd.print("V:");
    if (voldisp < 10) {
      lcd.print(0);
    }
    lcd.print(voldisp);
    radioIsOn = true;
  }

  delay(10);
}

void animateStartup() {
  if (run) {
    lcd.clear();
    run = false;
  }

  unsigned long now = millis();
  unsigned long elapsed = now - startupStart;

  lcd.setCursor(3, 0);
  lcd.print("STARTING");

  if (now - lastDotTime >= 300) {
    lastDotTime = now;
    dotCount = (dotCount + 1) % 4;
  }
  lcd.setCursor(4, 1);
  lcd.print("RADIO");
  for (int i = 0; i < dotCount; i++) {
    lcd.print(".");
  }
  if (elapsed > 4000) {
    startupInProgress = false;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RADIO MODE");
    radio.setVolume(5);
    delay(400);
    radio.setVolume(10);
    delay(400);
    radio.setVolume(20);
    delay(400);
    radio.setVolume(30);
    delay(1100);
    lcd.clear();
    updateRadio();
  }
}

void handleModeButton() {
  static bool lastState = HIGH;
  bool state = digitalRead(PIN_MODE_BTN);

  if (lastState == HIGH && state == LOW) {
    modeBtnPressTime = millis();
    modeBtnHeld = false;
  } else if (lastState == LOW && !VOLUME && state == LOW && (!modeBtnHeld && (millis() - modeBtnPressTime > LONG_PRESS_MS))) {
    modeBtnHeld = true;
    btMode = !btMode;
  } else if (lastState == LOW && state == HIGH && !firstpress && !modeBtnHeld && (millis() - modeBtnPressTime < 1200)) {
    firstpress = true;
  } else if (lastState == LOW && state == HIGH && firstpress && !second_press && !modeBtnHeld && (millis() - modeBtnPressTime < 1200)) {
    second_press = true;
  } else if (lastState == LOW && state == HIGH && firstpress && !THIRDPRESS && second_press && !modeBtnHeld && (millis() - modeBtnPressTime < 1200)) {
    THIRDPRESS = true;
  }

  if (THIRDPRESS == true && !VOLUME && !modeBtnHeld && (millis() - modeBtnPressTime < 1250)) {
    second_press = false;
    firstpress = false;
    THIRDPRESS = false;
    if (!btMode) {
      currentMode = (Mode)((currentMode + 3) % 4);
      switch (currentMode) {
        case MODE_FM:
          radio.setFM();
          radio.setFrequency((uint16_t)(freqFM * 100));
          updateradioDisplay();
          break;
        case MODE_AM:
          radio.setAM();
          radio.setFrequency(freqAM);
          updateradioDisplay();
          break;
        case MODE_SW:
          radio.setAM();
          radio.setFrequency(freqSW);
          updateradioDisplay();
          break;
        case MODE_LW:
          radio.setAM();
          radio.setFrequency(freqLW);
          updateradioDisplay();
          break;
      }
    } else {
      PREV();
    }
  }

  if (!modeBtnHeld && second_press == true && !VOLUME && (millis() - modeBtnPressTime > 1250)) {
    second_press = false;
    firstpress = false;
    if (!btMode) {
      currentMode = (Mode)((currentMode + 1) % 4);
      switch (currentMode) {
        case MODE_FM:
          radio.setFM();
          radio.setFrequency((uint16_t)(freqFM * 100));
          updateradioDisplay();
          break;
        case MODE_AM:
          radio.setAM();
          radio.setFrequency(freqAM);
          updateradioDisplay();
          break;
        case MODE_SW:
          radio.setAM();
          radio.setFrequency(freqSW);
          updateradioDisplay();
          break;
        case MODE_LW:
          radio.setAM();
          radio.setFrequency(freqLW);
          updateradioDisplay();
          break;
      }
    } else {
      NEXT();
    }
  }

  if (firstpress == true && (millis() - modeBtnPressTime >= 1250)) {
    firstpress = false;
    BT_PLAY_PUASE();
    mute();
  }

  if (!Muted && lastState == LOW && !btMode && state == HIGH && (!modeBtnHeld && (millis() - modeBtnPressTime > 1430))) {
    VOLUME = !VOLUME;
  }
  if (lastState == LOW && !Muted && !btMode && state == LOW && (!modeBtnHeld && (millis() - modeBtnPressTime > 1450))) {
    if (!VOLUME) {
      lcd.setCursor(0, 0);
      lcd.print("Select Volume   ");
    } else {
      lcd.setCursor(0, 0);
      lcd.print("Set at");
      lcd.setCursor(7, 0);
      lcd.print(volStep);
      lcd.print("         ");
    }
  }
  // detect exit from volume mode
  if (!VOLUME && lastVolumeState == true) {
    volumeExitTime = millis();
    volumeExitPending = true;
  }
  lastVolumeState = VOLUME;
  lastState = state;
}

void PREV() {
  digitalWrite(BT_VOLUP, HIGH);
  delay(350);
  digitalWrite(BT_VOLUP, LOW);
}

void NEXT() {
  digitalWrite(BT_VOLDOWN, HIGH);
  delay(350);
  digitalWrite(BT_VOLDOWN, LOW);
}

void mute() {
  if (Muted == false && radioIsOn) {
    radio.setVolume(0);
    Muted = true;
  } else if (radioIsOn) {
    int radioVol = ((volStep * 63) / 63);
    radio.setVolume(radioVol);
    Muted = false;
  }
  if (Muted && !btMode) {
    lcd.setCursor(0, 1);
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print("MUTE");
  } else if (!btMode) {
    int voldisp = (volStep * 1);
    lcd.setCursor(0, 1);
    lcd.print("V:");
    lcd.print(voldisp);
  }
}

void simulatePowerButtonPress() {   // Enable pin drive
  digitalWrite(PIN_PWR_SIM, HIGH);  // Drive gate HIGH (turn on N-MOS)
  delay(3500);                      // Hold for 3.5 seconds
  digitalWrite(PIN_PWR_SIM, LOW);   // Float pin (let external resistor pull gate LOW)
}

void BT_PLAY_PUASE() {
  if (btMode) {
    digitalWrite(PIN_PWR_SIM, HIGH);
    delay(400);  // Hold for 3.5 seconds
    digitalWrite(PIN_PWR_SIM, LOW);
  }
}

void updateRadio() {
  int radioVol = ((volStep * 63) / 63);
  radio.setVolume(radioVol);
}

void handlevolumeControl(int diff) {
  if (!btMode) {
    volStep += diff * volchange;
    volStep = constrain(volStep, volmin, volmax);
    volumedisplay();
    updateRadio();
  }
}

void handleFrequencyControl(int diff) {
  if (!btMode) {
    switch (currentMode) {
      case MODE_FM:
        freqFM += diff * FM_STEP;
        if (freqFM < FM_MIN) {
          freqFM = FM_MAX;
        }
        if (freqFM > FM_MAX) {
          freqFM = FM_MIN;
        }
        freqFM = constrain(freqFM, FM_MIN, FM_MAX);
        radio.setFrequency((uint16_t)(freqFM * 100));
        updateradioDisplay();
        break;
      case MODE_AM:
        freqAM += diff * AM_STEP;
        if (freqAM < AM_MIN) {
          freqAM = AM_MAX;
        }
        if (freqAM > AM_MAX) {
          freqAM = AM_MIN;
        }
        freqAM = constrain(freqAM, AM_MIN, AM_MAX);
        radio.setFrequency(freqAM);
        updateradioDisplay();
        break;
      case MODE_SW:
        freqSW += diff * SW_STEP;
        if (freqSW < SW_MIN) {
          freqSW = SW_MAX;
        }
        if (freqSW > SW_MAX) {
          freqSW = SW_MIN;
        }
        freqSW = constrain(freqSW, SW_MIN, SW_MAX);
        radio.setFrequency(freqSW);
        updateradioDisplay();
        break;
      case MODE_LW:
        freqLW += diff * LW_STEP;
        if (freqLW < LW_MIN) {
          freqLW = LW_MAX;
        }
        if (freqLW > LW_MAX) {
          freqLW = LW_MIN;
        }
        freqLW = constrain(freqLW, LW_MIN, LW_MAX);
        radio.setFrequency(freqLW);
        updateradioDisplay();
        break;
      default:
        break;
    }
  }
}


void updateradioDisplay() {
  if (!btMode) {
    lcd.setCursor(0, 0);
    switch (currentMode) {
      case MODE_FM:
        lcd.print("               ");
        lcd.setCursor(0, 0);
        lcd.print("FM ");
        lcd.print(freqFM, 1);
        lcd.print(" MHz  ");
        volumedisplay();
        break;
      case MODE_AM:
        lcd.print("               ");
        lcd.setCursor(0, 0);
        lcd.print("AM ");
        lcd.print(freqAM);
        lcd.print(" kHz  ");
        volumedisplay();
        break;
      case MODE_SW:
        lcd.print("               ");
        lcd.setCursor(0, 0);
        lcd.print("SW ");
        lcd.print(freqSW / 1000.0, 3);
        lcd.print(" MHz ");
        volumedisplay();
        break;
      case MODE_LW:
        lcd.print("               ");
        lcd.setCursor(0, 0);
        lcd.print("LW ");
        lcd.print(freqLW);
        lcd.print(" kHz ");
        volumedisplay();
        break;
      default:
        break;
    }
  }
}

void updateRDS() {
  if (wait - millis() >= 2000 && wait - millis() <= 2030 && !VOLUME) {
    updateradioDisplay();
  }
  if (radioIsOn) {

    radio.rdsBeginQuery();  // same as getRdsStatus()

    const char *rt = radio.getRdsText2A();  // returns pointer to radio-text buffer

    // If text pointer is null or empty, scroll char
    if (currentMode != MODE_FM || rt == NULL || rt[0] == '\0') {
      rt = "            Still Your Boy CHUKWUEBUKA (GOD_IS_WITH_US), Have A Blessful Day And Keep Listening To The RADIO While It Last.";
    }

    // 3. Copy text into local buffer
    strncpy(rdsText, rt, RDS_MAX - 1);
    rdsText[RDS_MAX - 1] = '\0';

    int len = strlen(rdsText);

    // 4. Prepare 10-character display
    char disp[11];
    if (len <= 11) {
      strncpy(disp, rdsText, 11);
      disp[11] = '\0';
    } else {
      for (int i = 0; i < 11; i++) {
        int idx = (scrollPos + i) % len;
        disp[i] = rdsText[idx];
      }
      disp[11] = '\0';

      scrollPos++;
      if (scrollPos >= len) scrollPos = 0;
    }
    // 5. Print it
    lcd.setCursor(5, 1);
    lcd.print(disp);
  }
}

void displayBTMode() {
  if (btMode) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("BLUETOOTH MODE  ");
    lcd.setCursor(0, 1);
    lcd.print("CONNECT=> TM-078");
  }
}

void volumedisplay() {
  if (radioIsOn && !btMode && !Muted) {
    int voldisp = (volStep * 1);
    lcd.setCursor(0, 1);
    lcd.print("V:");
    if (voldisp < 10) {
      lcd.print(0);
    }
    lcd.print(voldisp);
  } else if (radioIsOn && !btMode) {
    lcd.setCursor(0, 1);
    lcd.print("    ");
    lcd.setCursor(0, 1);
    lcd.print("MUTE");
  }
}

void updateEncoder() {
  int MSB = digitalRead(PIN_ENC_FREQ_A);
  int LSB = digitalRead(PIN_ENC_FREQ_B);

  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) {
    encoderValue++;
  }

  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) {
    encoderValue--;
  }

  lastEncoded = encoded;
}
