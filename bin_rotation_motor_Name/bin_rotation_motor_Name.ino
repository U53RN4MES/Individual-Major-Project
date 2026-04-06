#define STEPPER_PIN_1 9
#define STEPPER_PIN_2 10
#define STEPPER_PIN_3 11
#define STEPPER_PIN_4 12

// Compartment definitions — A=0, B=1, C=2
#define COMP_A 0
#define COMP_B 1
#define COMP_C 2

int step_number = 0;              // Tracks current coil step (0-3)
int currentCompartment = COMP_A; // Tracks which compartment is at output position

void setup() {
  Serial.begin(9600);

  // Set all stepper pins as outputs
  pinMode(STEPPER_PIN_1, OUTPUT);
  pinMode(STEPPER_PIN_2, OUTPUT);
  pinMode(STEPPER_PIN_3, OUTPUT);
  pinMode(STEPPER_PIN_4, OUTPUT);

  Serial.println("System ready. Align compartment A to output position then power on.");
  Serial.println("Type A, B or C to rotate to that compartment.");
}

void loop() {
  // Check if a character has been sent via Serial Monitor
  if (Serial.available() > 0) {
    char input = Serial.read();

    // Convert input character to compartment number
    int targetCompartment = -1;
    if (input == 'A' || input == 'a') targetCompartment = COMP_A;
    else if (input == 'B' || input == 'b') targetCompartment = COMP_B;
    else if (input == 'C' || input == 'c') targetCompartment = COMP_C;

    // Only proceed if a valid compartment was entered
    if (targetCompartment != -1) {
      goToCompartment(targetCompartment);
    }
  }
}

void goToCompartment(int target) {
  // No movement needed if already at target
  if (target == currentCompartment) {
    Serial.println("Already at that compartment. No rotation needed.");
    return;
  }

  // Calculate how many 120° forward rotations are needed
  // Result is always 1 or 2 since there are only 3 compartments
  // e.g. A->B = 1 step, A->C = 2 steps
  int forwardSteps = (target - currentCompartment + 3) % 3;

  Serial.print("Rotating to compartment ");
  printCompartment(target);

  // Rotate forward 120° for each step needed
  for (int i = 0; i < forwardSteps; i++) {
    rotateForward120();
  }

  // Update current compartment tracking
  currentCompartment = target;
  Serial.print("Now at compartment: ");
  printCompartment(currentCompartment);
  Serial.println();
}

// Prints the compartment letter to Serial Monitor
void printCompartment(int comp) {
  if (comp == COMP_A) Serial.println("A");
  else if (comp == COMP_B) Serial.println("B");
  else if (comp == COMP_C) Serial.println("C");
}

// Rotates the motor exactly 120 degrees forward
// 683 steps = 120 degrees based on this motor's gear ratio
void rotateForward120() {
  int stepsNeeded = 683;
  for (int i = 0; i < stepsNeeded; i++) {
    OneStep(false); // false = forward direction
    delay(20);      // 20ms between steps for reliable torque
  }
}

// Fires the correct coils in sequence to move one step
// dir=true: forward coil sequence
// dir=false: reverse coil sequence (not currently used but kept for reference)
void OneStep(bool dir) {
  if (dir) {
    // Forward coil sequence — energizes two coils at a time for max torque
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
    // Reverse coil sequence
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

  // Advance to next step in sequence, wrapping back to 0 after step 3
  step_number++;
  if (step_number > 3) {
    step_number = 0;
  }
}