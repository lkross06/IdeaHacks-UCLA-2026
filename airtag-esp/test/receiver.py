import socket
import struct

UDP_IP = "0.0.0.0"
UDP_PORT = 4210

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening on port {UDP_PORT}...")

try:
    while True:
        data, addr = sock.recvfrom(512) #sizeof(SensorPacket) == 40U
        values = struct.unpack('bfffffffff', data)
        # Assuming values = struct.unpack('bfffffffff', data)
        print(f"ID: {values[0]} | Accel: {values[1]:011.8f}, {values[2]:011.8f}, {values[3]:011.8f} | Gyro: {values[4]:011.8f}, {values[5]:011.8f}, {values[6]:011.8f} | Mag: {values[7]:011.8f}, {values[8]:011.8f}, {values[9]:011.8f}")
except KeyboardInterrupt:
    print("Shutting down...");