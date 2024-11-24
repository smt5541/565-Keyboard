import socket

SERVER_ADDRESS = ('192.168.0.221', 5541)  # Replace with your ESP32's IP address
MESSAGE = b"{\"jsonrpc\": \"2.0\", \"method\": \"LEDUpdate\", \"params\": {\"leds\": [1,1,1,1]}}"

def udp_client():
    # Create a UDP socket


if __name__ == "__main__":
    udp_client()