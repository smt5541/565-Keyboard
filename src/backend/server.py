import logging
import json
import time

from config import DEVICE_HOSTNAME, DEVICE_PORT, SERVER_PORT, SERVER_HOSTNAME
from rpc import RPC
from udp import UDPClient, UDPServer

logger = logging.getLogger(__name__)

def main():
    logging.basicConfig(level=logging.DEBUG)
    client = UDPClient(DEVICE_HOSTNAME, DEVICE_PORT)
    server = UDPServer(SERVER_HOSTNAME, SERVER_PORT)
    last_heartbeat_sent = 0

    def perform_state_acknowledge():
        """Provide the Device with the current System State"""
        client.send(RPC.state_acknowledge())
        client.send(RPC.display_update(["Testing", "Display Update"]))
        client.send(RPC.led_update([0,0,0]))

    # Listen for incoming messages
    while True:
        # If heartbeat hasn't been sent in a bit, send one
        if int(time.time()) - last_heartbeat_sent >= 10:
            client.send(RPC.host_heartbeat())
            last_heartbeat_sent = int(time.time())
        # If there's data, read it
        if server.has_data():
            data, addr = server.receive(1024)
            msg = json.loads(data.decode())
            if msg["method"] == "StateRequest":
                perform_state_acknowledge()
            if msg["method"] == "CardSwipe":
                if msg["params"]["success"]:
                    if msg["params"]["data"] == ";11026222405040=1802=0000000000000004?8":
                        client.send(RPC.display_update(["Welcome User", "Seth"]))
                    elif msg["params"]["data"] == ";11026222404030=1802=0000000000000004?>":
                        client.send(RPC.display_update(["Welcome User", "Ada"]))
                    else:
                        client.send(RPC.display_update(["Unknown Card", ""]))
                else:
                    client.send(RPC.display_update(["Card Read Failed", ""]))

if __name__ == "__main__":
    main()