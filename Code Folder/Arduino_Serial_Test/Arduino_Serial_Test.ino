#include <SoftwareSerial.h>

// Pin 6 = RX (receives from ESP32 TX)
// Pin 7 = TX (sends to ESP32 RX via voltage divider)
SoftwareSerial espSerial(6, 7);

void setup() {
  Serial.begin(9600);     // USB to laptop
  espSerial.begin(9600);  // communication with ESP32
  Serial.println("UNO ready");
}

void loop() {
  // Send CAPTURE to ESP32
  espSerial.println("CAPTURE");
  Serial.println("Sent: CAPTURE");

  // Wait up to 5 seconds for response
  unsigned long start = millis();
  while (millis() - start < 5000) {
    if (espSerial.available()) {
      String result = espSerial.readStringUntil('\n');
      result.trim();
      Serial.print("Received: ");
      Serial.println(result);
      break;
    }
  }

  delay(3000);
}