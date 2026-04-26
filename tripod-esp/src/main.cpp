#include <Arduino.h>

// --- Pin Definitions ---
const int pinPWMA = 32;
const int pinAIN1 = 25;
const int pinAIN2 = 26;
const int pinPWMB = 33;
const int pinBIN1 = 27;
const int pinBIN2 = 14;
const int pinSTBY = 15;

// --- PWM Settings ---
const int freq = 500;      
const int resolution = 8;
const int chanA = 0;
const int chanB = 1;

void setup() {
    Serial.begin(115200);

    pinMode(pinAIN1, OUTPUT);
    pinMode(pinAIN2, OUTPUT);
    pinMode(pinBIN1, OUTPUT);
    pinMode(pinBIN2, OUTPUT);
    pinMode(pinSTBY, OUTPUT);

    ledcSetup(chanA, freq, resolution);
    ledcSetup(chanB, freq, resolution);
    ledcAttachPin(pinPWMA, chanA);
    ledcAttachPin(pinPWMB, chanB);

    digitalWrite(pinSTBY, HIGH);
    Serial.println("System Ready!");
}

void setMotor(char motor, int speed) {
    bool reverse = speed < 0;
    int absSpeed = abs(speed);

    if (motor == 'A') {
        digitalWrite(pinAIN1, !reverse);
        digitalWrite(pinAIN2, reverse);
        ledcWrite(chanA, absSpeed);
    } else {
        digitalWrite(pinBIN1, !reverse);
        digitalWrite(pinBIN2, reverse);
        ledcWrite(chanB, absSpeed);
    }
}

void loop() {
    // Step 1: A+ B+
    setMotor('A', 255);
    setMotor('B', 255);
    delay(2); 

    // Step 2: A- B+
    setMotor('A', -255);
    setMotor('B', 255);
    delay(2);

    // Step 3: A- B-
    setMotor('A', -255);
    setMotor('B', -255);
    delay(2);

    // Step 4: A+ B-
    setMotor('A', 255);
    setMotor('B', -255);
    delay(2);
}