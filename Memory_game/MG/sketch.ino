#include <Arduino.h>

#define NUM_COLORS 5

// LED pins
int ledPins[NUM_COLORS] = {2, 4, 16, 17, 5};
// Button pins
int buttonPins[NUM_COLORS] = {19, 21, 22, 23, 25};
// Buzzer pin
int buzzer = 18;

// Game variables
int sequence[100]; // store sequence
int level = 1;
bool gameOver = false;

void setup() {
  Serial.begin(115200);
  
  for(int i = 0; i < NUM_COLORS; i++) {
    pinMode(ledPins[i], OUTPUT);
    pinMode(buttonPins[i], INPUT_PULLDOWN);
  }
  pinMode(buzzer, OUTPUT);
  randomSeed(analogRead(0)); // seed random
}

void loop() {
  if(gameOver) {
    Serial.println("Game Over!");
    while(1); // stop
  }

  Serial.print("Level: "); Serial.println(level);

  // Generate sequence
  for(int i=0; i<level; i++) {
    sequence[i] = random(NUM_COLORS);
    blinkLED(sequence[i]);
    delay(500);
  }

  // Get user input
  for(int i=0; i<level; i++) {
    int userInput = waitForButton();
    if(userInput != sequence[i]) {
      gameOverSound();
      gameOver = true;
      return;
    } else {
      correctSound();
    }
  }
  
  delay(1000);
  level++; // next level
}

// Function to blink LED
void blinkLED(int led) {
  digitalWrite(ledPins[led], HIGH);
  delay(500);
  digitalWrite(ledPins[led], LOW);
  delay(200);
}

// Wait for user button press and return the index
int waitForButton() {
  while(true) {
    for(int i=0; i<NUM_COLORS; i++) {
      if(digitalRead(buttonPins[i]) == HIGH) {
        // wait for release
        while(digitalRead(buttonPins[i]) == HIGH);
        delay(50);
        return i;
      }
    }
  }
}

// Correct buzzer sound (1 beep)
void correctSound() {
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(100);
}

// Wrong buzzer sound (2 beeps)
void gameOverSound() {
  for(int i=0; i<2; i++) {
    digitalWrite(buzzer, HIGH);
    delay(200);
    digitalWrite(buzzer, LOW);
    delay(100);
  }
}
