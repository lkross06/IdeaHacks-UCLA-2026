#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <math.h>

// ─── WiFi Config (kept, but not used for control) ───────────────
const char* SSID     = "Lucas's iPhone";
const char* PASSWORD = "lebronpookie123";
const int UDP_PORT   = 4210;

WiFiUDP udp;

// ─── Pin Definitions ────────────────────────────────────────────
#define AIN1 25
#define AIN2 26
#define BIN1 27
#define BIN2 14
#define PWMA 32
#define PWMB 33
#define STBY 15

// ─── Stepper Sequence ───────────────────────────────────────────
const int stepSequence[4][4] = {
    {1, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 1, 0, 1},
    {1, 0, 0, 1}
};
int currentStep = 0;

// ─── Timing Control ─────────────────────────────────────────────
unsigned long setupEndTime = 0;
unsigned long motorStartTime = 0;

bool eventTriggered = false;
bool motorRunning   = false;

const unsigned long START_DELAY_MS = 5000;  // wait 5 seconds
const unsigned long RUN_TIME_MS    = 3000;  // run for 3 seconds

// ─── Motor Control ──────────────────────────────────────────────
void stepMotor(int direction) {
    currentStep = (currentStep + direction + 4) % 4;
    digitalWrite(AIN1, stepSequence[currentStep][0]);
    digitalWrite(AIN2, stepSequence[currentStep][1]);
    digitalWrite(BIN1, stepSequence[currentStep][2]);
    digitalWrite(BIN2, stepSequence[currentStep][3]);
}

void setMotorVelocity(float degreesPerSec) {
    if (fabs(degreesPerSec) < 2.0f) return;

    int direction = (degreesPerSec > 0) ? 1 : -1;
    float stepsPerSec = fabs(degreesPerSec) / 1.8f;
    int stepDelay_us = (int)(10000000.0f / stepsPerSec);

    stepMotor(direction);
    delayMicroseconds(constrain(stepDelay_us, 500, 2000000));
}

// ─── Setup ──────────────────────────────────────────────────────
void setup() {
    Serial.begin(115200);
    delay(500);

    pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
    pinMode(PWMA, OUTPUT); pinMode(PWMB, OUTPUT);
    pinMode(STBY, OUTPUT);

    digitalWrite(PWMA, HIGH);
    digitalWrite(PWMB, HIGH);
    digitalWrite(STBY, HIGH);

    // Optional WiFi init (not required for this behavior)
    // WiFi.begin(SSID, PASSWORD);
    // while (WiFi.status() != WL_CONNECTED) {
    //     delay(500);
    // }

    // udp.begin(UDP_PORT);

    Serial.println("setup complete");

    // Mark end of setup timing reference
    setupEndTime = millis();
}

// ─── Loop ───────────────────────────────────────────────────────
void loop() {
    unsigned long now = millis();

    if (!eventTriggered) Serial.println(now - setupEndTime);

    // ─── Trigger after 8 seconds ────────────────────────────────
    if (!eventTriggered && (now - setupEndTime >= START_DELAY_MS)) {
        motorRunning = true;
        motorStartTime = now;
        eventTriggered = true;
    }

    // ─── Run motor for 3 seconds ────────────────────────────────
    if (motorRunning) {
        setMotorVelocity(-2.0f);

        if (now - motorStartTime >= RUN_TIME_MS) {
            motorRunning = false;
        }
    }
}