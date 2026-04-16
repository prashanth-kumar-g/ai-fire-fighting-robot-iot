// AI-Enabled Smart Fire Fighting Robot Using IoT

#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>


/* ================= WIFI ================= */
void connectWiFi() 
{
  WiFi.mode(WIFI_STA);

  const char* ssids[] = {"WiFi-1 Name", "WiFi-2 Name", "WiFi-3 Name"};

  const char* passwords[] = {"WiFi-1 Password", "WiFi-2 Password", "WiFi-3 Password"};

  int wifiCount = sizeof(ssids) / sizeof(ssids[0]);

  for (int i = 0; i < wifiCount; i++) 
  {
    Serial.print("Trying WiFi: ");
    Serial.println(ssids[i]);

    WiFi.begin(ssids[i], passwords[i]);

    unsigned long startAttempt = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttempt < 8000) 
    {
      delay(500);
      Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) 
    {
      Serial.println("\n✅ Connected to: " + String(ssids[i]));
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
      return;
    }

    Serial.println("\n❌ Failed, trying next...");
  }

  Serial.println("❌ Could not connect to any WiFi");
}


/* ================= BACKEND ================= */
const char* serverUrl = "https://ai-fire-fighting-robot-iot.onrender.com/api/command";


/* ================= L298 ================= */
#define IN1 14
#define IN2 12
#define IN3 13
#define IN4 15
#define ENA 5
#define ENB 4

int straightSpeed = 120;
int turnSpeed = 180;

/* ================= PUMP / ALERT ================= */
#define PUMP_PIN 23
#define LED_PIN 21
#define BUZZER_PIN 22

/* ================= ULTRASONIC ================= */
#define TRIG_PIN 26
#define ECHO_PIN 27

/* ================= FLAME ================= */
#define FLAME_LEFT   32
#define FLAME_CENTER 33
#define FLAME_RIGHT  25

/* ================= PAN TILT ================= */
#define PAN_PIN 18
#define TILT_PIN 19

Servo panServo;
Servo tiltServo;

int panAngle = 90;
int tiltAngle = 90;

const int PAN_STEP = 15;
const int TILT_STEP = 15;

/* ================= STATE ================= */
String currentMode = "manual";
enum AutoState {AUTO_INIT, AUTO_WAIT_FIRE, AUTO_HANDLE_FIRE, AUTO_DONE};
AutoState autoState = AUTO_INIT;
bool fireDetected = false;


/* ================= SETUP ================= */
void setup() 
{
  Serial.begin(115200);

  pinMode(IN1, OUTPUT); 
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); 
  pinMode(IN4, OUTPUT);
  pinMode(ENA, OUTPUT); 
  pinMode(ENB, OUTPUT);

  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(FLAME_LEFT, INPUT);
  pinMode(FLAME_CENTER, INPUT);
  pinMode(FLAME_RIGHT, INPUT);

  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  stopMotors();

  panServo.attach(PAN_PIN);
  tiltServo.attach(TILT_PIN);

  panServo.write(panAngle);
  tiltServo.write(tiltAngle); 

  connectWiFi();
}


/* ================= AUTOMATIC MODE ================= */
void automaticLoop()
{
  checkFire();

  switch (autoState)
  {
    case AUTO_INIT:
      moveForward();
      delay(2000);
      stopMotors();
      autoState = AUTO_WAIT_FIRE;
      break;

    case AUTO_WAIT_FIRE:
      // just waiting
      break;

    case AUTO_HANDLE_FIRE:
      handleFire();   // this sets AUTO_DONE internally
      break;

    case AUTO_DONE:
      stopMotors();   // stay stopped forever
      digitalWrite(PUMP_PIN, LOW);
      break;
  }
}

/* ================= FIRE CHECK ================= */
void checkFire() 
{
  int left   = digitalRead(FLAME_LEFT);
  int center = digitalRead(FLAME_CENTER);
  int right  = digitalRead(FLAME_RIGHT);

  if (left == LOW || center == LOW || right == LOW) 
  {
    delay(50); // small debounce
    if (digitalRead(FLAME_LEFT) == LOW || digitalRead(FLAME_CENTER) == LOW || digitalRead(FLAME_RIGHT) == LOW) 
    {
      fireDetected = true;
      autoState = AUTO_HANDLE_FIRE;
    }
  }
}

/* ================= FIRE HANDLING ================= */
void handleFire() 
{
  // Move closer to fire
  moveForward();
  delay(800);
  stopMotors();

  // Alerts
  digitalWrite(LED_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);

  // Aim nozzle
  panServo.write(90);
  tiltServo.write(130);
  delay(400);

  // Pump ON
  digitalWrite(PUMP_PIN, HIGH);

  // Sweep spray
  for (int angle = 70; angle <= 110; angle += 3) 
  {
    panServo.write(angle);
    delay(30);
  }
  for (int angle = 110; angle >= 70; angle -= 3) 
  {
    panServo.write(angle);
    delay(30);
  }

  delay(3000);

  // Pump/Alerts OFF
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  stopMotors();

  fireDetected = false;
  autoState = AUTO_DONE;   // STOP FOREVER
}


/* ================= MANUAL MODE ================= */
void manualLoop() 
{
  if (WiFi.status() == WL_CONNECTED) 
  {
    HTTPClient http;
    http.begin(serverUrl);
    int httpCode = http.GET();

    if (httpCode == 200) 
    {
      String payload = http.getString();
      StaticJsonDocument<256> doc;
      deserializeJson(doc, payload);

      String newMode = doc["mode"] | currentMode;

      if (newMode != currentMode)
      {
        currentMode = newMode;

        if (currentMode == "automatic")
        {
          autoState = AUTO_INIT;
          fireDetected = false;
          stopMotors();
          return;
        }
      }

      String moveCmd = doc["move"] | "stop";
      String pumpCmd = doc["pump"] | "off";
      String panCmd  = doc["pan"]  | "none";
      String tiltCmd = doc["tilt"] | "none";

      /* -------- CAR -------- */
      if (moveCmd == "forward") 
      {
        moveForward();
      }
      else if (moveCmd == "backward") 
      {
        moveBackward();
      }
      else if (moveCmd == "left") 
      {
        turnLeft();
      }
      else if (moveCmd == "right") 
      {
        turnRight();
      }
      else 
      {
        stopMotors();
      }

      /* -------- PUMP -------- */
      if (pumpCmd == "on") 
      {
        digitalWrite(PUMP_PIN, HIGH);
        digitalWrite(LED_PIN, HIGH);
        digitalWrite(BUZZER_PIN, HIGH);
      }
      else 
      {
        digitalWrite(PUMP_PIN, LOW);
        digitalWrite(LED_PIN, LOW);
        digitalWrite(BUZZER_PIN, LOW);
      }

      // -------- PAN CONTROL --------
      if (panCmd == "left") 
      {
        panAngle -= PAN_STEP;
      }
      else if (panCmd == "right") 
      {
        panAngle += PAN_STEP;
      }
      // Clamp pan angle
      panAngle = constrain(panAngle, 15, 165);
      panServo.write(panAngle);

      // -------- TILT CONTROL --------
      if (tiltCmd == "up") 
      {
        tiltAngle -= TILT_STEP;
      }
      else if (tiltCmd == "down") 
      {
        tiltAngle += TILT_STEP;
      }
      // Clamp tilt angle
      tiltAngle = constrain(tiltAngle, 45, 165);
      tiltServo.write(tiltAngle);
    }
    http.end();
  }
}

/* ================= MOTOR ================= */
void stopMotors() 
{
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW); 
  digitalWrite(IN4, LOW);
  analogWrite(ENA, 0);
  analogWrite(ENB, 0);
}

void moveForward() 
{
  digitalWrite(IN1, LOW); 
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW); 
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, straightSpeed);
  analogWrite(ENB, straightSpeed);
}

void moveBackward() 
{
  digitalWrite(IN1, HIGH); 
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH); 
  digitalWrite(IN4, LOW);
  analogWrite(ENA, straightSpeed);
  analogWrite(ENB, straightSpeed);
}

void turnLeft() 
{
  digitalWrite(IN1, HIGH); 
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);  
  digitalWrite(IN4, HIGH);
  analogWrite(ENA, turnSpeed);
  analogWrite(ENB, turnSpeed);
}

void turnRight() 
{
  digitalWrite(IN1, LOW);  
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH); 
  digitalWrite(IN4, LOW);
  analogWrite(ENA, turnSpeed);
  analogWrite(ENB, turnSpeed);
}


/* ================= ULTRASONIC SAFETY ================= */
long getDistance() 
{
  digitalWrite(TRIG_PIN, LOW); 
  delayMicroseconds(2);
  
  digitalWrite(TRIG_PIN, HIGH); 
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH, 25000);
  return duration * 0.034 / 2;
}


/* ================= LOOP ================= */
void loop() 
{
  long d = getDistance();
  if (d > 0 && d <= 10) 
  {
    stopMotors();
    digitalWrite(PUMP_PIN, LOW);
    return;
  }

  if (currentMode == "automatic") 
  {
    automaticLoop();
  } 
  else 
  {
    manualLoop();
  }

  delay(20);
}
