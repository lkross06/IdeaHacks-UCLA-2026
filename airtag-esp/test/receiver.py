import socket
import struct

from pos import *

# Listen on all interfaces, Port 4210
UDP_IP = "0.0.0.0"
UDP_PORT = 4210

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening on port {UDP_PORT}...")

try:
    while True:
        data, addr = sock.recvfrom(512) #sizeof(SensorPacket) == 36U
        values = struct.unpack('fffffffff', data)
        print(values)
except KeyboardInterrupt:
    print("Shutting down...");