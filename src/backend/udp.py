import logging
import select
import socket
from typing import Tuple, Any

logger = logging.getLogger(__name__)

class UDPClient:
    """Simple UDP Client Wrapper"""

    def __init__(self, host: str, port: int):
        """
        Create a UDP Client
        :param host: The host to send UDP packets to
        :param port: The port on the host to send UDP packets to
        """
        self.device_address = (host, port)

    def send(self, message: str, encoding: str = "utf-8") -> None:
        """
        Send a UDP message
        :param message: The message to send
        :param encoding: The encoding to send it as, defaults to utf-8
        """
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as client_sock:
            logger.debug(f"Sending message: {message}")
            client_sock.sendto(message.encode(encoding), self.device_address)

class UDPServer:
    """Simple UDP Server Wrapper"""

    def __init__(self, host, port):
        """
        Create a UDP Server
        :param host: The host to bind on
        :param port: The port to listen on
        """
        self.server = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.server.bind((host, port))
        logger.info(f"Listening for UDP packets on {host}:{port}")

        # Set the socket to non-blocking
        self.server.setblocking(0)

    def has_data(self) -> bool:
        """
        Get whether data is available on the Server
        :return: True if data is available, False otherwise
        """
        readable, _, _ = select.select([self.server], [], [], 0.1)  # 0.1 second timeout
        return len(readable) > 0

    def receive(self, size) -> Tuple[bytes, Any]:
        """
        Receive a UDP message
        :param size: The number of bytes to receive
        :return: Tuple of the data and the sender's IP address
        """
        data, addr = self.server.recvfrom(size)
        logger.debug(f"Received message: {data.decode()} from {addr}")
        return data, addr