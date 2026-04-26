# IdeaHacks-UCLA-2026

## airtag-esp

Contains the embedded code for polling an Adafruit 9-DoF breakout board and sending packets over WiFi to UDP port 4210.

```
struct SensorPacket {
    bool flag;
    float ax, ay, az;
    float gx, gy, gz;
    float mx, my, mz;
}
```

## imu-web-server

Display the IMU packets

## tripod-esp

Contains the PID controller + K-filter + etc.