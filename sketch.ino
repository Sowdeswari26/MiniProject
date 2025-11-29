// Pin Definitions
const int mainRed = 2;
const int mainYellow = 4;
const int mainGreen = 5;

const int sideRed = 12;
const int sideYellow = 13;
const int sideGreen = 14;

const int pedestrianBtn = 15;
const int pirSensor = 27; // optional

// Timing
int greenTime = 5000;  // 5 sec
int yellowTime = 2000; // 2 sec

void setup() {
  pinMode(mainRed, OUTPUT);
  pinMode(mainYellow, OUTPUT);
  pinMode(mainGreen, OUTPUT);

  pinMode(sideRed, OUTPUT);
  pinMode(sideYellow, OUTPUT);
  pinMode(sideGreen, OUTPUT);

  pinMode(pedestrianBtn, INPUT_PULLUP);
  pinMode(pirSensor, INPUT);

  Serial.begin(115200);
}

void loop() {
  // Main road green
  digitalWrite(mainGreen, HIGH);
  digitalWrite(mainRed, LOW);
  digitalWrite(mainYellow, LOW);

  digitalWrite(sideRed, HIGH);
  digitalWrite(sideGreen, LOW);
  digitalWrite(sideYellow, LOW);

  waitForGreenOrPedestrian(greenTime);

  // Main road yellow
  digitalWrite(mainGreen, LOW);
  digitalWrite(mainYellow, HIGH);
  delay(yellowTime);
  digitalWrite(mainYellow, LOW);
  digitalWrite(mainRed, HIGH);

  // Side road green
  digitalWrite(sideRed, LOW);
  digitalWrite(sideGreen, HIGH);
  delay(greenTime);

  // Side road yellow
  digitalWrite(sideGreen, LOW);
  digitalWrite(sideYellow, HIGH);
  delay(yellowTime);
  digitalWrite(sideYellow, LOW);
  digitalWrite(sideRed, HIGH);
}

void waitForGreenOrPedestrian(int duration) {
  int interval = 100;
  int elapsed = 0;
  while (elapsed < duration) {
    if (digitalRead(pedestrianBtn) == LOW) {
      Serial.println("Pedestrian crossing requested!");
      pedestrianCrossing();
      break;
    }
    elapsed += interval;
    delay(interval);
  }
}

void pedestrianCrossing() {
  // Blink all red for pedestrian crossing
  for (int i = 0; i < 5; i++) {
    digitalWrite(mainRed, HIGH);
    digitalWrite(sideRed, HIGH);
    delay(500);
    digitalWrite(mainRed, LOW);
    digitalWrite(sideRed, LOW);
    delay(500);
  }
  // Restore main road red & side road green
  digitalWrite(mainRed, HIGH);
  digitalWrite(sideRed, LOW);
  digitalWrite(sideGreen, HIGH);
}

