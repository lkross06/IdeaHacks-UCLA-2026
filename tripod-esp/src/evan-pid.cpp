#include <Wire.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <math.h>

// ─── WiFi Config ────────────────────────────────────────────────
const char* SSID     = "Lucas's iPhone";
const char* PASSWORD = "lebronpookie123";
const int UDP_PORT   = 4210;

WiFiUDP udp;

// ─── Packet Definition ──────────────────────────────────────────
// Must match sender exactly — sizeof(SensorPacket) = 40
struct SensorPacket { // lucas def of packet 
    bool  id;           // 1 = person IMU, 0 = tripod IMU
    float ax, ay, az;
    float gx, gy, gz;
    float mx, my, mz;
};

// ─── Pin Definitions ────────────────────────────────────────────
#define AIN1 25
#define AIN2 26
#define BIN1 27
#define BIN2 14
#define PWMA 32
#define PWMB 33
#define STBY 15

// ─── Complementary Filter ───────────────────────────────────────
const float ALPHA = 0.98f;
float filteredHeading = 0.0f;
const float dt = 0.02f;

bool firstPacket = true;        // used to seed filteredHeading
int packetCount  = 0;           // used to guard calibration timing
const int MIN_PACKETS_BEFORE_CAL = 50;  // ~1 second at 50Hz

// ─── PID ────────────────────────────────────────────────────────
float Kp = 2.0f;
float Ki = 0.05f;
float Kd = 0.3f;
const float TAU = 0.05f;

float integral      = 0.0f;
float prevError     = 0.0f;
float filteredDeriv = 0.0f;

const float INTEGRAL_LIMIT = 30.0f;
const float OUTPUT_LIMIT   = 200.0f;

// ─── Calibration ────────────────────────────────────────────────
float thetaRef  = 0.0f;
bool calibrated = false;

// ─── Stepper ────────────────────────────────────────────────────
const int stepSequence[4][4] = {
    {1, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 1, 0, 1},
    {1, 0, 0, 1}
};
int currentStep = 0;

// ─── Helpers ────────────────────────────────────────────────────
float wrapAngle(float angle) {
    angle = fmod(angle + 180.0f, 360.0f);
    if (angle < 0.0f) angle += 360.0f;
    return angle - 180.0f;
}

float clamp(float value, float limit) {
    if (value >  limit) return  limit;
    if (value < -limit) return -limit;
    return value;
}

// ─── Sensor Fusion ──────────────────────────────────────────────
float computeHeading(SensorPacket& pkt) {
    float ax = pkt.ax;
    float ay = pkt.ay;
    float az = pkt.az;

    float pitch = atan2(-ax, sqrt(ay*ay + az*az));
    float roll  = atan2(ay, az);

    float mx = pkt.mx;
    float my = pkt.my;
    float mz = pkt.mz;

    float mx2 = mx * cos(pitch) + mz * sin(pitch);
    float my2 = mx * sin(roll) * sin(pitch) + my * cos(roll) - mz * sin(roll) * cos(pitch);

    float magHeading = atan2(-my2, mx2) * 180.0f / PI;
    if (magHeading < 0) magHeading += 360.0f;

    // Seed filter on first packet so we don't start from 0
    if (firstPacket) {
        filteredHeading = magHeading;
        firstPacket = false;
    }

    // gz is in rad/s, convert to deg/s for complementary filter
    float gyroRate    = pkt.gz * 180.0f / PI;
    float gyroHeading = filteredHeading + gyroRate * dt;
    filteredHeading   = ALPHA * gyroHeading + (1.0f - ALPHA) * magHeading;

    if (filteredHeading < 0)   filteredHeading += 360.0f;
    if (filteredHeading > 360) filteredHeading -= 360.0f;

    return filteredHeading;
}

// ─── PID ────────────────────────────────────────────────────────
float pidCompute(float error) {
    error = wrapAngle(error);

    float P = Kp * error;

    integral += error * dt;
    integral = clamp(integral, INTEGRAL_LIMIT);
    float I = Ki * integral;

    float rawDeriv = (error - prevError) / dt;
    float alpha = dt / (TAU + dt);
    filteredDeriv = alpha * rawDeriv + (1.0f - alpha) * filteredDeriv;
    float D = Kd * filteredDeriv;

    prevError = error;

    return clamp(P + I + D, OUTPUT_LIMIT);
}

void pidReset() {
    integral      = 0.0f;
    prevError     = 0.0f;
    filteredDeriv = 0.0f;
}

// ─── Motor ──────────────────────────────────────────────────────
void stepMotor(int direction) {
    currentStep = (currentStep + direction + 4) % 4;
    digitalWrite(AIN1, stepSequence[currentStep][0]);
    digitalWrite(AIN2, stepSequence[currentStep][1]);
    digitalWrite(BIN1, stepSequence[currentStep][2]);
    digitalWrite(BIN2, stepSequence[currentStep][3]);
}

void setMotorVelocity(float degreesPerSec) {
    if (fabs(degreesPerSec) < 2.0f) return;  // dead zone

    int direction = (degreesPerSec > 0) ? 1 : -1;
    float stepsPerSec = fabs(degreesPerSec) / 1.8f;
    int stepDelay_us = (int)(1000000.0f / stepsPerSec);

    stepMotor(direction);
    delayMicroseconds(constrain(stepDelay_us, 500, 20000));
}

// ─── Setup ──────────────────────────────────────────────────────
void setup() { // setup basically just turns eveyrthing to high and sets pin modes and takes in wifi communication protocol 
    Serial.begin(115200);

    pinMode(AIN1, OUTPUT); pinMode(AIN2, OUTPUT);
    pinMode(BIN1, OUTPUT); pinMode(BIN2, OUTPUT);
    pinMode(PWMA, OUTPUT); pinMode(PWMB, OUTPUT);
    pinMode(STBY, OUTPUT);

    digitalWrite(PWMA, HIGH); 
    digitalWrite(PWMB, HIGH);
    digitalWrite(STBY, HIGH);

    WiFi.begin(SSID, PASSWORD);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    Serial.print("Tripod IP: ");
    Serial.println(WiFi.localIP());

    udp.begin(UDP_PORT);
    Serial.println("Waiting for IMU packets...");
}

// ─── Loop ───────────────────────────────────────────────────────
void loop() {
    unsigned long loopStart = millis();

    // ── Receive packet ───────────────────────────────────────────
    SensorPacket pkt;
    int packetSize = udp.parsePacket(); //receives new packet every iteration 

    // ── Calibration ─────────────────────────────────────────────
    // Only allow after filter has had time to settle
    if (packetSize == sizeof(SensorPacket)) {
        udp.read((uint8_t*)&pkt, sizeof(SensorPacket));

        // Ignore packets not from person IMU
        if (pkt.id != 0) return;

        // Update heading only when packet arrives
        filteredHeading = computeHeading(pkt);
        packetCount++;

        // Notify once filter has settled
        if (packetCount == MIN_PACKETS_BEFORE_CAL) {
            // if filter has had time to settle -> calibrated goes to true
            thetaRef   = filteredHeading;
            calibrated = true;
            pidReset();
            Serial.print("Calibrated at: ");
            Serial.println(thetaRef);
        }
    }

    // ── PID + Motor — always runs every tick ─────────────────────
    // Uses last known filteredHeading even if no packet this tick
    if (calibrated) {
        float error    = filteredHeading - thetaRef;
        float motorCmd = pidCompute(error); //pid feedback applied only if calibrated
        setMotorVelocity(motorCmd);
    }

    // ── Hold 50Hz ────────────────────────────────────────────────
    unsigned long elapsed = millis() - loopStart;
    if (elapsed < 20) delay(20 - elapsed);
}