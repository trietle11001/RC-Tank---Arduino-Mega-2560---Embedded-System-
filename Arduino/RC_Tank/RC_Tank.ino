/*
 * RC Tank Receiver — Elegoo Mega 2560
 * =====================================
 * Receives single-character commands from an HC-06 Bluetooth module
 * and drives two DC motors via an L298N motor driver.
 *
 * WIRING — L298N → Mega 2560
 * ───────────────────────────
 *  L298N Pin   →  Mega Pin   Notes
 *  ──────────────────────────────────────────────────────────────
 *  IN1         →  D8         Left motor direction A
 *  IN2         →  D9         Left motor direction B
 *  ENA         →  D10 (PWM)  Left motor speed (PWM)
 *
 *  IN3         →  D11        Right motor direction A
 *  IN4         →  D12        Right motor direction B
 *  ENB         →  D13 (PWM)  Right motor speed (PWM) — or use D6/D7
 *
 *  12V         →  12V battery (separate from logic supply)
 *  GND         →  GND (shared with Mega GND)
 *  5V (out)    →  Can power Mega 5V pin if your L298N board has a regulator
 *
 * WIRING — HC-06 → Mega 2560
 * ───────────────────────────
 *  HC-06 VCC  →  3.3V or 5V (check your module — most accept 5V)
 *  HC-06 GND  →  GND
 *  HC-06 TXD  →  Mega RX1 (pin 19)   ← Serial1
 *  HC-06 RXD  →  Mega TX1 (pin 18)   ← Serial1 (use a voltage divider if HC-06 is 3.3V logic)
 *
 * COMMANDS (ASCII characters)
 * ────────────────────────────
 *  F = Forward          B = Backward
 *  L = Turn Left        R = Turn Right
 *  S = Stop
 *  0 = Speed 100%       1-9 = Speed 10%-90%
 */

// ── Pin definitions ────────────────────────────────────────────────────────────
const int LEFT_IN1  = 8;    // Left motor direction
const int LEFT_IN2  = 9;
const int LEFT_ENA  = 10;   // Left motor PWM speed

const int RIGHT_IN3 = 11;   // Right motor direction
const int RIGHT_IN4 = 12;
const int RIGHT_ENB = 6;    // Right motor PWM speed (D6 is PWM on Mega)

// ── State ──────────────────────────────────────────────────────────────────────
int motorSpeed = 128;   // Default ~50% (0–255)


// ── Motor control helpers ──────────────────────────────────────────────────────

void setLeft(int dir, int spd) {
  // dir:  1 = forward,  -1 = backward,  0 = stop
  analogWrite(LEFT_ENA, spd);
  if (dir == 1)       { digitalWrite(LEFT_IN1, HIGH); digitalWrite(LEFT_IN2, LOW); }
  else if (dir == -1) { digitalWrite(LEFT_IN1, LOW);  digitalWrite(LEFT_IN2, HIGH); }
  else                { digitalWrite(LEFT_IN1, LOW);  digitalWrite(LEFT_IN2, LOW); }
}

void setRight(int dir, int spd) {
  analogWrite(RIGHT_ENB, spd);
  if (dir == 1)       { digitalWrite(RIGHT_IN3, HIGH); digitalWrite(RIGHT_IN4, LOW); }
  else if (dir == -1) { digitalWrite(RIGHT_IN3, LOW);  digitalWrite(RIGHT_IN4, HIGH); }
  else                { digitalWrite(RIGHT_IN3, LOW);  digitalWrite(RIGHT_IN4, LOW); }
}

void moveForward()  { setLeft( 1, motorSpeed); setRight( 1, motorSpeed); }
void moveBackward() { setLeft(-1, motorSpeed); setRight(-1, motorSpeed); }
void turnLeft()     { setLeft(-1, motorSpeed); setRight( 1, motorSpeed); } // pivot left
void turnRight()    { setLeft( 1, motorSpeed); setRight(-1, motorSpeed); } // pivot right
void stopMotors()   { setLeft( 0, 0);          setRight( 0, 0); }


// ── Setup ──────────────────────────────────────────────────────────────────────

void setup() {
  // Motor pins
  pinMode(LEFT_IN1,  OUTPUT);
  pinMode(LEFT_IN2,  OUTPUT);
  pinMode(LEFT_ENA,  OUTPUT);
  pinMode(RIGHT_IN3, OUTPUT);
  pinMode(RIGHT_IN4, OUTPUT);
  pinMode(RIGHT_ENB, OUTPUT);

  stopMotors();

  // Serial1 for HC-06 (pins 18/19 on Mega)
  Serial1.begin(9600);

  // Serial0 for debug output (open Arduino Serial Monitor at 9600 to see)
  Serial.begin(9600);
  Serial.println("RC Tank ready. Waiting for commands on Serial1…");
}


// ── Main loop ──────────────────────────────────────────────────────────────────

void loop() {
  if (Serial1.available() > 0) {
    char cmd = Serial1.read();
    Serial.print("Received: ");
    Serial.println(cmd);   // Debug — visible in Arduino Serial Monitor

    switch (cmd) {
      case 'F':
        moveForward();
        Serial.println("→ Forward");
        break;

      case 'B':
        moveBackward();
        Serial.println("→ Backward");
        break;

      case 'L':
        turnLeft();
        Serial.println("→ Turn Left");
        break;

      case 'R':
        turnRight();
        Serial.println("→ Turn Right");
        break;

      case 'S':
        stopMotors();
        Serial.println("→ Stop");
        break;

      case '0':
        motorSpeed = 255;
        Serial.println("→ Speed 100%");
        break;

      case '1': case '2': case '3': case '4': case '5':
      case '6': case '7': case '8': case '9':
        // Map '1'–'9'  →  26–230  (10% – 90% of 255)
        motorSpeed = map(cmd - '0', 1, 9, 26, 230);
        Serial.print("→ Speed ");
        Serial.print((cmd - '0') * 10);
        Serial.println("%");
        break;

      default:
        // Unknown command — ignore silently
        break;
    }
  }
}
