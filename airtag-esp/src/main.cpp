#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_LSM303_U.h>
#include <WiFi.h>

Adafruit_L3GD20_Unified       gyro = Adafruit_L3GD20_Unified(20);
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);
WiFiUDP udp;

#define I2C_SDA 11  //black
#define I2C_SCL 12  //brown

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("9-DOF Sensor Test (L3GD20H + LSM303)");

  Wire.begin(I2C_SDA, I2C_SCL);

  if(!accel.begin() || !mag.begin() || !gyro.begin()) {
    Serial.println("Could not find a valid 9-DOF sensor, check wiring!");
    while(1);
  }

  Serial.println("Setup complete!");
  delay(500);
}

void loop() {
  sensors_event_t a, m, g;

  accel.getEvent(&a);
  mag.getEvent(&m);
  gyro.getEvent(&g);

  // Create a CSV-style string to send
  String packet = String(a.acceleration.x) + "," + String(a.acceleration.y) + "," + String(a.acceleration.z) + "|" +
                  String(g.gyro.x) + "," + String(g.gyro.y) + "," + String(g.gyro.z);

  Serial.println(packet);

  delay(10);
}