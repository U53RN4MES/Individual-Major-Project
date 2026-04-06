const int irPin = 8;

void setup() {
  Serial.begin(9600);
}

void loop() {
  int irState = digitalRead(irPin);
  Serial.println(irState);

  delay(200);
}
