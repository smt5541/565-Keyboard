import json
import socket
import select
import time

# Define the server's IP address and port
UDP_SERVER_IP = "0.0.0.0"  # Listen on all available interfaces
UDP_SERVER_PORT = 5541     # Same port as the Arduino sender
DEVICE_ADDRESS = ("192.168.0.221", 5541)

# Create a UDP socket
server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to the address and port
server.bind((UDP_SERVER_IP, UDP_SERVER_PORT))

print(f"Listening for UDP packets on {UDP_SERVER_IP}:{UDP_SERVER_PORT}")

# Set the socket to non-blocking
server.setblocking(0)

def udp_client(message: bytes):
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as client_sock:
        # Send data to the UDP server
        print(f"Sending message: {message.decode()}")
        client_sock.sendto(message, DEVICE_ADDRESS)

def send_state_acknowledge():
    udp_client(b'{"jsonrpc": "2.0", "method": "StateAcknowledge"}')

def send_display_update(lines: list[str]):
    udp_client(f'{{"jsonrpc": "2.0", "method": "DisplayUpdate", "params": {{"rows": {json.dumps(lines)}}}}}'.encode())

def perform_state_acknowledge():
    send_state_acknowledge()
    send_display_update(["Testing", "Display Update"])

last_heartbeat_sent = 0
def send_host_heartbeat():
    global last_heartbeat_sent
    udp_client(b'{"jsonrpc": "2.0", "method": "HostHeartbeat"}')
    last_heartbeat_sent = int(time.time())
# Listen for incoming messages
while True:
    # Use select to check if data is available to be read
    readable, _, _ = select.select([server], [], [], 0.1)  # 0.1 second timeout
    if int(time.time()) - last_heartbeat_sent >= 15:
        send_host_heartbeat()
    if readable:
        data, addr = server.recvfrom(1024)  # buffer size of 1024 bytes
        print(f"Received message: {data.decode()} from {addr}")
        rpc = json.loads(data.decode())
        if rpc["method"] == "StateRequest":
            perform_state_acknowledge()
    else:
        # No data available, perform other tasks here if needed
        continue
        # print("No UDP packets received, waiting...")