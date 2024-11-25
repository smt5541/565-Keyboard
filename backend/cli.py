import logging
from config import DEVICE_HOSTNAME, DEVICE_PORT
from udp import UDPClient
import argparse

logger = logging.getLogger(__name__)
logging.basicConfig(level=logging.DEBUG)
argparser = argparse.ArgumentParser(
    prog='cli',
    description="Command line interface for the Device RPC")
argparser.add_argument("method")
argparser.add_argument("params")
args = argparser.parse_args()

client = UDPClient(DEVICE_HOSTNAME, DEVICE_PORT)
if args.params:
    client.send(f"{{\"jsonrpc\": \"2.0\", \"method\": \"{args.method}\", \"params\": {args.params}}}")
else:
    client.send(f"{{\"jsonrpc\": \"2.0\", \"method\": \"{args.method}\"}}")