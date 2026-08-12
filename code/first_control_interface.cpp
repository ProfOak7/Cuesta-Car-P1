/*
  ==========================================================
  Cuesta Robotics - CR-01
  Wi-Fi Web Remote v0.1
  ESP32 + L298N
  ==========================================================

  MOTOR WIRING:

  LEFT MOTOR
  ESP32 GPIO 33 -> L298N ENA
  ESP32 GPIO 25 -> L298N IN1
  ESP32 GPIO 26 -> L298N IN2

  RIGHT MOTOR
  ESP32 GPIO 32 -> L298N ENB
  ESP32 GPIO 27 -> L298N IN3
  ESP32 GPIO 14 -> L298N IN4

  ESP32 GND -> L298N GND

  This version:
  - Creates its own Wi-Fi network
  - Hosts a phone/laptop control webpage
  - Has forward/reverse/spin controls
  - Has PWM speed control
  - Stops automatically if control communication is lost
*/

#include <WiFi.h>
#include <WebServer.h>


// ==========================================================
// WI-FI SETTINGS
// ==========================================================

const char* ssid = "CR01-Robot";
const char* password = "cuestarobot";

// Give the robot a predictable address
IPAddress local_IP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);

WebServer server(80);


// ==========================================================
// MOTOR PINS
// ==========================================================

// LEFT MOTOR
const int LEFT_PWM = 33;
const int LEFT_IN1 = 25;
const int LEFT_IN2 = 26;

// RIGHT MOTOR
const int RIGHT_PWM = 32;
const int RIGHT_IN1 = 27;
const int RIGHT_IN2 = 14;


// ==========================================================
// MOTOR SETTINGS
// ==========================================================

int motorSpeed = 200;

// Safety timeout.
// If the robot hasn't received a drive command recently,
// it automatically stops.

unsigned long lastCommandTime = 0;
const unsigned long COMMAND_TIMEOUT = 500;

bool motorsRunning = false;


// ==========================================================
// MOTOR FUNCTIONS
// ==========================================================

void leftMotor(int speedValue) {

  // LEFT MOTOR IS PHYSICALLY INVERTED

  if (speedValue > 0) {

    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, HIGH);

    analogWrite(LEFT_PWM, speedValue);
  }

  else if (speedValue < 0) {

    digitalWrite(LEFT_IN1, HIGH);
    digitalWrite(LEFT_IN2, LOW);

    analogWrite(LEFT_PWM, -speedValue);
  }

  else {

    digitalWrite(LEFT_IN1, LOW);
    digitalWrite(LEFT_IN2, LOW);

    analogWrite(LEFT_PWM, 0);
  }
}


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

  motorsRunning = false;
}


// ==========================================================
// DRIVE FUNCTIONS
// ==========================================================

void driveForward() {

  leftMotor(motorSpeed);
  rightMotor(motorSpeed);

  motorsRunning = true;
  lastCommandTime = millis();
}


void driveBackward() {

  leftMotor(-motorSpeed);
  rightMotor(-motorSpeed);

  motorsRunning = true;
  lastCommandTime = millis();
}


void spinLeft() {

  leftMotor(-motorSpeed);
  rightMotor(motorSpeed);

  motorsRunning = true;
  lastCommandTime = millis();
}


void spinRight() {

  leftMotor(motorSpeed);
  rightMotor(-motorSpeed);

  motorsRunning = true;
  lastCommandTime = millis();
}


// ==========================================================
// WEB PAGE
// ==========================================================

const char webpage[] PROGMEM = R"rawliteral(

<!DOCTYPE html>

<html>

<head>

<meta name="viewport"
content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">

<title>CR-01 Robot</title>

<style>

body {

  font-family: Arial, sans-serif;

  text-align: center;

  background: #111827;

  color: white;

  margin: 0;

  padding: 20px;

  user-select: none;

  -webkit-user-select: none;

  touch-action: manipulation;
}


h1 {

  margin-bottom: 5px;

}


.subtitle {

  color: #9ca3af;

  margin-bottom: 25px;

}


.controller {

  max-width: 420px;

  margin: auto;
}


.driveButton {

  width: 110px;

  height: 90px;

  margin: 8px;

  border: none;

  border-radius: 18px;

  font-size: 42px;

  background: #374151;

  color: white;

  box-shadow: 0 5px 10px rgba(0,0,0,0.35);
}


.driveButton:active {

  transform: scale(0.94);

  background: #4b5563;
}


.stopButton {

  width: 110px;

  height: 90px;

  margin: 8px;

  border: none;

  border-radius: 18px;

  font-size: 20px;

  font-weight: bold;

  background: #dc2626;

  color: white;
}


.speedBox {

  margin-top: 30px;

  padding: 20px;

  background: #1f2937;

  border-radius: 18px;
}


input[type=range] {

  width: 90%;
}


#speedValue {

  font-size: 24px;

  font-weight: bold;
}


.status {

  margin-top: 20px;

  color: #10b981;

}

</style>

</head>


<body>

<div class="controller">

<h1>CR-01</h1>

<div class="subtitle">
Cuesta Robotics
</div>


<div>

<button class="driveButton"
data-dir="forward">
&#9650;
</button>

</div>


<div>

<button class="driveButton"
data-dir="left">
&#9664;
</button>


<button class="stopButton"
onclick="stopRobot()">
STOP
</button>


<button class="driveButton"
data-dir="right">
&#9654;
</button>

</div>


<div>

<button class="driveButton"
data-dir="backward">
&#9660;
</button>

</div>


<div class="speedBox">

<div>
Speed
</div>

<br>

<input
type="range"
min="0"
max="255"
value="200"
id="speedSlider">

<p id="speedValue">
200
</p>

</div>


<div class="status" id="status">
READY
</div>

</div>


<script>

let driveTimer = null;
let currentDirection = null;


function sendCommand(direction) {

  fetch("/drive?dir=" + direction)
  .catch(() => {});

}


function startDriving(direction) {

  currentDirection = direction;

  sendCommand(direction);

  document.getElementById("status").innerHTML =
  direction.toUpperCase();

  clearInterval(driveTimer);


  // Keep sending a heartbeat while button is held.

  driveTimer = setInterval(function() {

    if(currentDirection) {

      sendCommand(currentDirection);

    }

  }, 150);

}


function stopRobot() {

  currentDirection = null;

  clearInterval(driveTimer);

  driveTimer = null;

  sendCommand("stop");

  document.getElementById("status").innerHTML =
  "STOPPED";

}


// Attach controls to every drive button

document.querySelectorAll(".driveButton").forEach(button => {

  const direction = button.dataset.dir;


  button.addEventListener("pointerdown", function(event) {

    event.preventDefault();

    startDriving(direction);

  });


  button.addEventListener("pointerup", function(event) {

    event.preventDefault();

    stopRobot();

  });


  button.addEventListener("pointercancel", function() {

    stopRobot();

  });

});


// If finger/mouse leaves the control interface,
// stop for safety.

window.addEventListener("pointerup", stopRobot);

window.addEventListener("blur", stopRobot);


// SPEED SLIDER

const slider =
document.getElementById("speedSlider");

const speedValue =
document.getElementById("speedValue");


slider.addEventListener("input", function() {

  speedValue.innerHTML = slider.value;

  fetch("/speed?value=" + slider.value)
  .catch(() => {});

});


</script>

</body>

</html>

)rawliteral";


// ==========================================================
// WEB REQUEST HANDLERS
// ==========================================================

void handleRoot() {

  server.send_P(
    200,
    "text/html",
    webpage
  );
}


void handleDrive() {

  String direction = server.arg("dir");


  if (direction == "forward") {

    driveForward();
  }

  else if (direction == "backward") {

    driveBackward();
  }

  else if (direction == "left") {

    spinLeft();
  }

  else if (direction == "right") {

    spinRight();
  }

  else {

    stopMotors();
  }


  server.send(
    200,
    "text/plain",
    "OK"
  );
}


void handleSpeed() {

  if (server.hasArg("value")) {

    int requestedSpeed =
    server.arg("value").toInt();

    motorSpeed =
    constrain(requestedSpeed, 0, 255);

    Serial.print("Speed: ");
    Serial.println(motorSpeed);
  }


  server.send(
    200,
    "text/plain",
    String(motorSpeed)
  );
}


// ==========================================================
// SETUP
// ==========================================================

void setup() {

  Serial.begin(115200);


  // --------------------------------------------------------
  // Motor setup
  // --------------------------------------------------------

  pinMode(LEFT_PWM, OUTPUT);
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);

  pinMode(RIGHT_PWM, OUTPUT);
  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);

  stopMotors();


  // --------------------------------------------------------
  // Wi-Fi Access Point
  // --------------------------------------------------------

  Serial.println();
  Serial.println("==============================");
  Serial.println(" CR-01 Wi-Fi Controller");
  Serial.println("==============================");


  WiFi.mode(WIFI_AP);


  WiFi.softAPConfig(
    local_IP,
    gateway,
    subnet
  );


  if (!WiFi.softAP(ssid, password)) {

    Serial.println("Wi-Fi AP FAILED!");

    while(true) {
      delay(1000);
    }
  }


  Serial.println();
  Serial.println("Wi-Fi started.");

  Serial.print("Network: ");
  Serial.println(ssid);

  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());


  // --------------------------------------------------------
  // Web server routes
  // --------------------------------------------------------

  server.on("/", handleRoot);

  server.on("/drive", handleDrive);

  server.on("/speed", handleSpeed);

  server.begin();


  Serial.println();
  Serial.println("Web server started.");
  Serial.println();
  Serial.println("Connect to:");
  Serial.println("CR01-Robot");
  Serial.println();
  Serial.println("Then open:");
  Serial.println("http://192.168.4.1");
}


// ==========================================================
// MAIN LOOP
// ==========================================================

void loop() {

  server.handleClient();


  // --------------------------------------------------------
  // FAILSAFE
  // --------------------------------------------------------

  if (
    motorsRunning &&
    (millis() - lastCommandTime > COMMAND_TIMEOUT)
  ) {

    stopMotors();

    Serial.println("FAILSAFE STOP");
  }
}