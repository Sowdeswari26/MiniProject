/* ESP32 Electronic Voting Machine (4 candidates)
   - Candidate buttons: GPIO13,12,14,27
   - LEDs (optional): GPIO2,4,5,18
   - Confirm button: GPIO33
   - Admin button:   GPIO32 (hold 3s to toggle Admin mode)
   - Buzzer: GPIO15
   - I2C LCD: SDA=21, SCL=22 (uses LiquidCrystal_I2C)
   - Uses Preferences to persist votes
*/

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>

#define NUM_CAND 4
// Buttons
const int btnPins[NUM_CAND] = {13, 32, 36, 34};
const int confirmPin = 33;
const int adminPin = 12;

// LEDs (optional)
const int ledPins[NUM_CAND] = {23,19, 4, 15};

// Buzzer
const int buzzerPin = 27;

// Display
LiquidCrystal_I2C lcd(0x27, 16, 2); // adjust 0x27 if your backpack uses different address

// Persistence
Preferences prefs;
const char *prefNamespace = "evm";

// Vote storage
unsigned long votes[NUM_CAND] = {0};

volatile int selectedCandidate = -1;
bool adminMode = false;

unsigned long lastSelectMillis = 0;
const unsigned long confirmTimeout = 5000; // ms to confirm

// debouncing helpers
unsigned long lastDebounceTime[NUM_CAND+2];
const unsigned long debounceDelay = 50;

void beep(int ms) {
  tone(buzzerPin, 1000, ms);
}

void loadVotes() {
  prefs.begin(prefNamespace, true);
  for (int i = 0; i < NUM_CAND; ++i) {
    votes[i] = prefs.getULong(String("c") + i, 0);
  }
  prefs.end();
}

void saveVote(int idx) {
  prefs.begin(prefNamespace, false);
  votes[idx]++;
  prefs.putULong(String("c") + idx, votes[idx]);
  prefs.end();
}

void showIdle() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Press to Vote");
  lcd.setCursor(0,1);
  lcd.print("Admin: hold Btn");
}

void showSelection(int idx) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Selected:");
  lcd.setCursor(0,1);
  lcd.print("C");
  lcd.print(idx+1);
  lcd.print("  Confirm->");
}

void flashLed(int idx, int times=2, int tms=120) {
  for (int i=0;i<times;i++){
    digitalWrite(ledPins[idx], HIGH);
    delay(tms);
    digitalWrite(ledPins[idx], LOW);
    delay(80);
  }
}

void showThanks() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Vote Casted!");
  lcd.setCursor(0,1);
  lcd.print("Thank You");
}

void showAdmin() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("ADMIN MODE");
  for (int i = 0; i < NUM_CAND && i < 2; ++i) {
    lcd.setCursor(0, 1);
  }
  delay(400);
}

// Big Admin display scrolls counts
void displayCounts() {
  lcd.clear();
  for (int i = 0; i < NUM_CAND; ++i) {
    lcd.setCursor(0,0);
    lcd.print("C");
    lcd.print(i+1);
    lcd.print(": ");
    lcd.print(votes[i]);
    // second line show hint
    lcd.setCursor(0,1);
    lcd.print("Hold Admin to exit");
    delay(1200);
    lcd.clear();
  }
  lcd.setCursor(0,0);
  lcd.print("End of counts");
  lcd.setCursor(0,1);
  lcd.print("Hold Admin to exit");
  delay(800);
}

void setupPins() {
  for (int i = 0; i < NUM_CAND; ++i) {
    pinMode(btnPins[i], INPUT_PULLUP);
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], LOW);
    lastDebounceTime[i] = 0;
  }
  pinMode(confirmPin, INPUT_PULLUP);
  lastDebounceTime[NUM_CAND] = 0;
  pinMode(adminPin, INPUT_PULLUP);
  lastDebounceTime[NUM_CAND+1] = 0;
  pinMode(buzzerPin, OUTPUT);
}

void setup() {
  Serial.begin(115200);
  Wire.begin();
  lcd.init();
  lcd.backlight();
  setupPins();

  prefs.begin(prefNamespace, true);
  prefs.end();

  loadVotes();
  showIdle();
}

bool readButtonDebounced(int pinIndex, int pin) {
  int reading = digitalRead(pin);
  unsigned long now = millis();
  if (reading == LOW) {
    if (now - lastDebounceTime[pinIndex] > debounceDelay) {
      lastDebounceTime[pinIndex] = now;
      return true;
    }
  } else {
    lastDebounceTime[pinIndex] = now;
  }
  return false;
}

void loop() {
  // Check Admin hold (3s) to toggle admin mode
  if (digitalRead(adminPin) == LOW) {
    unsigned long start = millis();
    while (digitalRead(adminPin) == LOW) {
      if (millis() - start > 3000) {
        adminMode = !adminMode;
        if (adminMode) {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Entering Admin");
          delay(800);
        } else {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("Exiting Admin");
          delay(600);
        }
        break;
      }
    }
    // wait a bit to avoid retrigger
    delay(400);
  }

  if (adminMode) {
    // continuously show counts until admin toggled out
    displayCounts();
    delay(200);
    continue;
  }

  // Normal voting mode
  // Check candidate buttons
  bool anyPressed = false;
  for (int i = 0; i < NUM_CAND; ++i) {
    if (digitalRead(btnPins[i]) == LOW) {
      anyPressed = true;
      // simple debounce
      if (readButtonDebounced(i, btnPins[i])) {
        selectedCandidate = i;
        lastSelectMillis = millis();
        showSelection(i);
        flashLed(i, 2, 120);
      }
    }
  }

  // If a selection is active, wait for confirm
  if (selectedCandidate != -1) {
    // check confirm press
    if (digitalRead(confirmPin) == LOW) {
      if (readButtonDebounced(NUM_CAND, confirmPin)) {
        // commit vote
        saveVote(selectedCandidate);
        beep(120);
        flashLed(selectedCandidate, 4, 80);
        showThanks();
        delay(1000);
        selectedCandidate = -1;
        showIdle();
        continue;
      }
    }

    // timeout cancel
    if (millis() - lastSelectMillis > confirmTimeout) {
      // cancel selection
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Cancelled");
      delay(600);
      selectedCandidate = -1;
      showIdle();
    }
  }

  delay(40);
}
