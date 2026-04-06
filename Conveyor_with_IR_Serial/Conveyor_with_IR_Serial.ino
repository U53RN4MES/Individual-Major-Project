#include <SoftwareSerial.h>

// ===== PINS =====
const int dirPin = 2;
const int stepPin = 3;
const int irPin = 8;

// ===== ESP32 SERIAL =====
// Pin 6 = RX, Pin 7 = TX
SoftwareSerial espSerial(6, 7);

// ===== VARIABLES =====
String lastResult = "";


void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(irPin, INPUT);

  digitalWrite(dirPin, HIGH);
  delay(2000);
  Serial.println("UNO ready");
}

// ===== MOVE CONVEYOR ONE STEP =====
void stepMotor(){
  digitalWrite(stepPin, HIGH);
  delayMicroseconds(500);
  digitalWrite(stepPin, LOW);
  delayMicroseconds(500);
}

void loop() {
  //Detect =  LOW(0), No detect = HIGH(1)
  int irState = digitalRead(irPin);
  // Serial.println(irState);
  // delay(500);


  if (irState == LOW) {
    Serial.println("Object detected — stopping conveyor");
    espSerial.println("CAPTURE");
    Serial.println("Sent: CAPTURE");

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
    }

    // Push waste off conveyor for 1 second
    Serial.println("Pushing waste off conveyor...");
    unsigned long pushStart = millis();
    while (millis() - pushStart < 2500) {
      stepMotor();
    }
    Serial.println("Ready for next item...");

  } else {
    stepMotor();
  }
}
