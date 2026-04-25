import socket
import struct

UDP_IP = "0.0.0.0"
UDP_PORT = 4210

# Setup Socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))
sock.setblocking(False)

def receive_packet():
    """
    Attempts to read one UDP packet.
    Returns (acc, gyro, mag) or (None, None, None) if no data is ready.
    """
    try:
        data, addr = sock.recvfrom(1024)
        # Unpack 9 floats (36 bytes)
        values = struct.unpack('fffffffff', data)
        acc = values[0:3]
        gyro = values[3:6]
        mag = values[6:9]
        return acc, gyro, mag
    except (BlockingIOError, struct.error):
        return None, None, None