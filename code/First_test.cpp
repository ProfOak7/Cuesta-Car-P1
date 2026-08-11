/*
  Cuesta Robotics - CR-01
  First Drivetrain Test

  LEFT MOTOR:
  ENA -> GPIO 33
  IN1 -> GPIO 25
  IN2 -> GPIO 26

  RIGHT MOTOR:
  ENB -> GPIO 32
  IN3 -> GPIO 27
  IN4 -> GPIO 14

  ESP32 GND -> L298N GND
*/

const int LEFT_PWM = 33;
const int LEFT_IN1 = 25;
const int LEFT_IN2 = 26;

const int RIGHT_PWM = 32;
const int RIGHT_IN1 = 27;
const int RIGHT_IN2 = 14;

// Start at full power for this diagnostic.
// Our ~6 V AA supply + L298N voltage drop doesn't leave huge motor voltage.
const int TEST_SPEED = 255;


// -------------------- LEFT MOTOR --------------------

void leftMotor(int speedValue) {

  if (speedValue > 0) {
    digitalWrite(LEFT_IN1, HIGH);
    digitalWrite(LEFT_IN2, LOW);
    analogWrite(LEFT_PWM, speedValue);
  }

  else if (speedValue < 0) {
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, HIGH);
    analogWrite(LEFT_PWM, -speedValue);
  }

  else {
    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, LOW);
    analogWrite(LEFT_PWM, 0);
  }
}


// -------------------- RIGHT MOTOR --------------------

void rightMotor(int speedValue) {

  if (speedValue > 0) {
    digitalWrite(RIGHT_IN1, HIGH);
    digitalWrite(RIGHT_IN2, LOW);
    analogWrite(RIGHT_PWM, speedValue);
  }

  else if (speedValue < 0) {
    digitalWrite(RIGHT_IN1, LOW);
    digitalWrite(RIGHT_IN2, HIGH);
    analogWrite(RIGHT_PWM, -speedValue);
  }

  else {
    digitalWrite(RIGHT_IN1, LOW);
    digitalWrite(RIGHT_IN2, LOW);
    analogWrite(RIGHT_PWM, 0);
  }
}


void stopMotors() {
  leftMotor(0);
  rightMotor(0);
}


// -------------------- SETUP --------------------

void setup() {

  Serial.begin(115200);

  pinMode(LEFT_PWM, OUTPUT);
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);

  pinMode(RIGHT_PWM, OUTPUT);
  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);

  stopMotors();

  Serial.println("CR-01 MOTOR TEST");
  Serial.println("Starting in 3 seconds...");

  delay(3000);


  // LEFT MOTOR
  Serial.println("LEFT FORWARD");
  leftMotor(TEST_SPEED);
  delay(2000);
  stopMotors();
  delay(1000);

  Serial.println("LEFT REVERSE");
  leftMotor(-TEST_SPEED);
  delay(2000);
  stopMotors();
  delay(1500);


  // RIGHT MOTOR
  Serial.println("RIGHT FORWARD");
  rightMotor(TEST_SPEED);
  delay(2000);
  stopMotors();
  delay(1000);

  Serial.println("RIGHT REVERSE");
  rightMotor(-TEST_SPEED);
  delay(2000);
  stopMotors();
  delay(1500);


  // BOTH FORWARD
  Serial.println("BOTH FORWARD");
  leftMotor(TEST_SPEED);
  rightMotor(TEST_SPEED);
  delay(2000);
  stopMotors();
  delay(1500);


  // BOTH REVERSE
  Serial.println("BOTH REVERSE");
  leftMotor(-TEST_SPEED);
  rightMotor(-TEST_SPEED);
  delay(2000);
  stopMotors();
  delay(1500);


  // SPIN LEFT
  Serial.println("SPIN LEFT");
  leftMotor(-TEST_SPEED);
  rightMotor(TEST_SPEED);
  delay(1500);
  stopMotors();
  delay(1500);


  // SPIN RIGHT
  Serial.println("SPIN RIGHT");
  leftMotor(TEST_SPEED);
  rightMotor(-TEST_SPEED);
  delay(1500);
  stopMotors();


  Serial.println("TEST COMPLETE");
}


void loop() {
  // Run once only.
}