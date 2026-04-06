const int dirPin = 2;
const int stepPin = 3;
const int irPin = 8;

bool motorEnabled = false;


void setup() {
  Serial.begin(9600);
  
  pinMode(dirPin, OUTPUT);
  pinMode(stepPin, OUTPUT);
  pinMode(irPin, INPUT);

  delay(2000);

  digitalWrite(dirPin, HIGH);
}

void loop() {
  int irState = digitalRead(irPin);
  // Serial.println(irState);
  // delay(500);

  if (irState == HIGH){
    motorEnabled = true;
  }
  else{
    motorEnabled = false;
  }

  if (motorEnabled){
    digitalWrite(stepPin, HIGH);
    delayMicroseconds(500);
    digitalWrite(stepPin, LOW);
    delayMicroseconds(500);
  }
  else{
    delay(5);
  }
}
