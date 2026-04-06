void setup() {
  // USB to laptop — for Serial Monitor debugging
  Serial.begin(115200);
  
  // Communication with Arduino UNO
  // RX = GPIO44 (D7), TX = GPIO43 (D6)
  Serial1.begin(9600, SERIAL_8N1, 44, 43);
  
  Serial.println("ESP32 ready — waiting for CAPTURE command...");
}

void loop() {
  // Check if UNO sent something
  if (Serial1.available()) {
    String command = Serial1.readStringUntil('\n');
    command.trim();
    
    // Print what we received to Serial Monitor
    Serial.print("Received from UNO: ");
    Serial.println(command);

    // If it says CAPTURE, send back a hardcoded result
    if (command == "CAPTURE") {
      Serial1.println("CARDBOARD");
      Serial.println("Sent to UNO: CARDBOARD");
    }
  }
}