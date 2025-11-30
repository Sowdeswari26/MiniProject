#include <Servo.h>
#include <LiquidCrystal.h>

// 4x4 Keypad setup
const int ROWS = 4;
const int COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

int rowPins[ROWS] = {13, 12, 14, 44};  // R1, R2, R3, R4
int colPins[COLS] = {10, 11, 43, 9};  // C1, C2, C3, C4

// LCD RS, EN, D4, D5, D6, D7 => ESP32 pins
LiquidCrystal lcd(19, 21, 38, 42, 40, 41);

Servo servo;
int buzzer = 40;

String password = "";
String input = "";
bool settingPassword = true;

// Custom function to read keypad
char getKey() {
  for (int r = 0; r < ROWS; r++) {
    digitalWrite(rowPins[r], LOW);

    for (int c = 0; c < COLS; c++) {
      if (digitalRead(colPins[c]) == LOW) {
        while (digitalRead(colPins[c]) == LOW); // key debounce
        digitalWrite(rowPins[r], HIGH);
        return keys[r][c];
      }
    }

    digitalWrite(rowPins[r], HIGH);
  }
  return 0;
}

void setup() {
  Serial.begin(115200);

  // Keypad pins
  for (int r = 0; r < ROWS; r++) {
    pinMode(rowPins[r], OUTPUT);
    digitalWrite(rowPins[r], HIGH);
  }
  for (int c = 0; c < COLS; c++) {
    pinMode(colPins[c], INPUT_PULLUP);
  }

  // LCD setup
  lcd.begin(16, 2);

  // Servo setup
  servo.attach(21);

  pinMode(buzzer, OUTPUT);

  lcd.print("Set Password:");
}

void loop() {
  char key = getKey();

  if (key) {
    if (settingPassword) {
      // Setting password
      if (key == '#') {
        if (input.length() > 0) {
          password = input;
          input = "";
          settingPassword = false;

          lcd.clear();
          lcd.print("Password Set!");
          delay(1000);

          lcd.clear();
          lcd.print("Enter Password:");
        } else {
          lcd.clear();
          lcd.print("No input!");
          delay(1000);

          lcd.clear();
          lcd.print("Set Password:");
        }
      }
      else if (key == '*') {
        input = "";
        lcd.clear();
        lcd.print("Set Password:");
      }
      else {
        input += key;
        lcd.setCursor(0, 1);
        lcd.print(String(input.length(), HEX));  // shows * count
      }

    } else {
      // Checking password
      if (key == '#') {
        if (input == password) {
          lcd.clear();
          lcd.print("Welcome!");

          servo.write(180);   // unlock
          delay(2000);
          servo.write(0);     // lock

        } else {
          lcd.clear();
          lcd.print("Wrong!");

          digitalWrite(buzzer, HIGH);
          delay(1000);
          digitalWrite(buzzer, LOW);
        }

        input = "";
        lcd.clear();
        lcd.print("Enter Password:");
      }
      else if (key == '*') {
        input = "";
        lcd.clear();
        lcd.print("Enter Password:");
      }
      else {
        input += key;
        lcd.setCursor(0, 1);
        lcd.print(String(input.length(), HEX));  // mask input
      }
    }
  }
}
