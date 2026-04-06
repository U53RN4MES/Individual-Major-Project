#include <SoftwareSerial.h>

// ===== CONVEYOR PINS =====
const int dirPin = 2;
const int stepPin = 3;
const int irPin = 8;

// ===== BIN STEPPER PINS =====
#define STEPPER_PIN_1 9
#define STEPPER_PIN_2 10
#define STEPPER_PIN_3 11
#define STEPPER_PIN_4 12

// ===== COMPARTMENT DEFINITIONS =====
#define COMP_PLASTIC   0  // Compartment A
#define COMP_CARDBOARD 1  // Compartment B
#define COMP_METAL     2  // Compartment C

// ===== ESP32 SERIAL =====
// Pin 6 = RX, Pin 7 = TX
SoftwareSerial espSerial(6, 7);

// ===== VARIABLES =====
String lastResult = "";
int step_number = 0;             // Tracks current coil step (0-3) for bin motor
int currentCompartment = COMP_PLASTIC; // Bin starts at plastic compartment

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);

  // Conveyor motor pins
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(irPin, INPUT);
  digitalWrite(dirPin, HIGH);

  // Bin stepper pins
  pinMode(STEPPER_PIN_1, OUTPUT);
  pinMode(STEPPER_PIN_2, OUTPUT);
  pinMode(STEPPER_PIN_3, OUTPUT);
  pinMode(STEPPER_PIN_4, OUTPUT);

  delay(2000);
  Serial.println("UNO ready. Align bin compartment A (Plastic) to output position.");
}

void loop() {
  // Detect = LOW(0), No detect = HIGH(1)
  int irState = digitalRead(irPin);

  if (irState == LOW) {
    Serial.println("Object detected — stopping conveyor");
    espSerial.println("CAPTURE");
    Serial.println("Sent: CAPTURE");

    // Wait up to 10 seconds for ESP32 classification result
    unsigned long start = millis();
    bool received = false;
    while (millis() - start < 10000) {
      if (espSerial.available()) {
        lastResult = espSerial.readStringUntil('\n');
        lastResult.trim();
        Serial.print("Result: ");
        Serial.println(lastResult);
        received = true;
        break;
      }
    }

    if (!received) {
      Serial.println("No response from ESP32 — continuing");
    } else {
      // Rotate bin to correct compartment based on classification
      if (lastResult == "Plastic") {
        Serial.println("Waste type: Plastic — rotating to compartment A");
        goToCompartment(COMP_PLASTIC);
      } else if (lastResult == "Cardboard") {
        Serial.println("Waste type: Cardboard — rotating to compartment B");
        goToCompartment(COMP_CARDBOARD);
      } else if (lastResult == "Metal") {
        Serial.println("Waste type: Metal — rotating to compartment C");
        goToCompartment(COMP_METAL);
      } else {
        Serial.println("Unknown waste type — bin stays in place");
      }
    }

    // Push waste off conveyor into bin for 2.5 seconds
    Serial.println("Pushing waste off conveyor...");
    unsigned long pushStart = millis();
    while (millis() - pushStart < 2500) {
      stepMotor();
    }
    Serial.println("Ready for next item...");

  } else {
    // No object — keep conveyor running
    stepMotor();
  }
}

// ===== CONVEYOR FUNCTIONS =====

void stepMotor() {
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(500);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(500);
}

// ===== BIN FUNCTIONS =====

void goToCompartment(int target) {
  if (target == currentCompartment) {
    Serial.println("Already at correct compartment. No rotation needed.");
    return;
  }

  // Calculate how many 120° forward rotations are needed
  int forwardSteps = (target - currentCompartment + 3) % 3;

  for (int i = 0; i < forwardSteps; i++) {
    rotateForward120();
  }

  currentCompartment = target;
  Serial.print("Bin now at compartment: ");
  if (currentCompartment == COMP_PLASTIC)   Serial.println("A (Plastic)");
  if (currentCompartment == COMP_CARDBOARD) Serial.println("B (Cardboard)");
  if (currentCompartment == COMP_METAL)     Serial.println("C (Metal)");
}

void rotateForward120() {
  int stepsNeeded = 683;
  for (int i = 0; i < stepsNeeded; i++) {
    OneStep(false);
    delay(20);
  }
}

void OneStep(bool dir) {
  if (dir) {
    switch (step_number) {
      case 0:
        digitalWrite(STEPPER_PIN_1, HIGH);
        digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 1:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, HIGH);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 2:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, HIGH);
        digitalWrite(STEPPER_PIN_4, HIGH);
        break;
      case 3:
        digitalWrite(STEPPER_PIN_1, HIGH);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, HIGH);
        break;
    }
  } else {
    switch (step_number) {
      case 0:
        digitalWrite(STEPPER_PIN_1, HIGH);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, HIGH);
        break;
      case 1:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, LOW);
        digitalWrite(STEPPER_PIN_3, HIGH);
        digitalWrite(STEPPER_PIN_4, HIGH);
        break;
      case 2:
        digitalWrite(STEPPER_PIN_1, LOW);
        digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, HIGH);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
      case 3:
        digitalWrite(STEPPER_PIN_1, HIGH);
        digitalWrite(STEPPER_PIN_2, HIGH);
        digitalWrite(STEPPER_PIN_3, LOW);
        digitalWrite(STEPPER_PIN_4, LOW);
        break;
    }
  }
  step_number++;
  if (step_number > 3) {
    step_number = 0;
  }
}