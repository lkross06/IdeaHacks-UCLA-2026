#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_L3GD20_U.h>
#include <Adafruit_LSM303_U.h>
#include <WiFi.h>
#include <Arduino.h>

const char* ssid = "Lucas's iPhone";
const char* password = "lebronpookie123";
const char* laptop_ip = "172.20.10.6";
const int udp_port = 4210;
WiFiUDP udp;

Adafruit_L3GD20_Unified       gyro = Adafruit_L3GD20_Unified(20);
Adafruit_LSM303_Accel_Unified accel = Adafruit_LSM303_Accel_Unified(30301);
Adafruit_LSM303_Mag_Unified   mag   = Adafruit_LSM303_Mag_Unified(30302);
sensors_event_t a, m, g;

#define I2C_SDA 11  //black
#define I2C_SCL 12  //brown

struct __attribute__((packed)) SensorPacket {
  float ax, ay, az;
  float gx, gy, gz;
  float mx, my, mz;
};
SensorPacket packet;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("9-DOF Sensor Test (L3GD20H + LSM303)");

  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000); //400 kHz

  if(!accel.begin() || !mag.begin() || !gyro.begin()) {
    Serial.println("Could not find a valid 9-DOF sensor, check wiring!");
    while(1);
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
  }

  Serial.println("Setup complete!");
  delay(500);
}

void loop() {
  accel.getEvent(&a);
  mag.getEvent(&m);
  gyro.getEvent(&g);

  packet.ax = a.acceleration.x;
  packet.ay = a.acceleration.y;
  packet.az = a.acceleration.z;

  packet.gx = g.gyro.x;
  packet.gy = g.gyro.y;
  packet.gz = g.gyro.z;

  packet.mx = m.magnetic.x;
  packet.my = m.magnetic.y;
  packet.mz = m.magnetic.z;

  udp.beginPacket(laptop_ip, udp_port);
  udp.write((uint8_t*)&packet, sizeof(packet)); //write raw bytes
  udp.endPacket();

  delay(10);  //magnetometer bottlenecks at 100Hz
}