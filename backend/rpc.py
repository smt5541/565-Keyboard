import json

class RPC:
    """RPC Message Generation and Parsing"""

    @classmethod
    def display_update(cls, rows: list[str]) -> str:
        """
        Generate a DisplayUpdate RPC
        :param rows: Strings to display on the Display, refer to device documentation for indexing
        :return: String containing the generated DisplayUpdate message
        """
        return f"{{\"jsonrpc\": \"2.0\", \"method\": \"DisplayUpdate\", \"params\": {{\"rows\": {json.dumps(rows)}}}}}"

    @classmethod
    def trigger_buzzer(cls) -> str:
        """TODO: Generate a TriggerBuzzer RPC"""
        return "Not Implemented"

    @classmethod
    def led_update(cls, leds: list[int]) -> str:
        """
        Generate a LEDUpdate RPC
        For the leds parameter, devices that are binary but will accept values >= 1 as on, analog devices will use PWM
        :param leds: Numbers 0-255 to run on each LED
        :return: String containing the generated LEDUpdate message
        """
        return f"{{\"jsonrpc\": \"2.0\", \"method\": \"LEDUpdate\", \"params\": {{\"leds\": {json.dumps(leds)}}}}}"

    @classmethod
    def host_heartbeat(cls) -> str:
        """
        Generate a Host Heartbeat RPC
        :return: String containing the generated HostHeartbeat message
        """
        return "{\"jsonrpc\": \"2.0\", \"method\": \"HostHeartbeat\"}"

    @classmethod
    def state_acknowledge(cls) -> str:
        """
        Generate a StateAcknowledge RPC
        :return: String containing the generated StateAcknowledge message
        """
        return "{\"jsonrpc\": \"2.0\", \"method\": \"StateAcknowledge\"}"