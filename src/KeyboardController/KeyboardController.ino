#include <Adafruit_NeoPixel.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <ESP32Time.h>
#include <NTPClient.h>

#define RGBLED_PIN 38
#define NUM_RGBLEDS 1
#define WIFI_SSID "ShrimpsIsBugs"
#define WIFI_PASS "5h1mp5camp1!"
#define NUM_LEDS 4
#define LED_WAIT 0
#define LED_OFFLINE 1
#define LED_MESSAGE_PENDING 2
#define LED_UNLABELED 3
#define DISPLAY_NUM_ROWS 2
#define RPC_SEND_HOST "192.168.0.127"
#define RPC_SEND_PORT 5541
#define HEARTBEAT_DELAY_SEC 15

Adafruit_NeoPixel rgbleds(NUM_RGBLEDS, RGBLED_PIN, NEO_GRB + NEO_KHZ800);
WiFiUDP rpcUdp;
WiFiUDP ntpUdp;
NTPClient timeClient(ntpUdp);
unsigned long lastHostHeartbeat = 0;
ESP32Time RTC(-18000);
const int unexposed_leds[] = {LED_OFFLINE};
int led_state[NUM_LEDS];
bool initialConnect = false;
bool expectingState = false;
const char *DEVICE_BOOTING = "Device Booting";
const char *CONNECTIVITY_ERROR = "Connectivity Error";
const char *WIFI_DISCONNECTED = "WiFi Disconnected";
const char *PENDING_STATE_SYNC = "Pending State Sync";
const char *HOST_OFFLINE = "Host Offline";
const char *RPC_MSG_STATE_REQUEST = "{\"jsonrpc\": \"2.0\", \"method\": \"StateRequest\"}";

void rgbled_off() {
  rgbleds.begin();
  rgbleds.clear();
  rgbleds.setPixelColor(0, rgbleds.Color(0, 0, 0));
  rgbleds.show();
}

void led_dbg() {
  Serial.println("led_dbg: LED State:");
  Serial.printf("led_dbg: Wait: %d\n", led_state[LED_WAIT]);
  Serial.printf("led_dbg: Offline: %d\n", led_state[LED_OFFLINE]);
  Serial.printf("led_dbg: Message Pending: %d\n", led_state[LED_MESSAGE_PENDING]);
  Serial.printf("led_dbg: Unlabeled: %d\n", led_state[LED_UNLABELED]);
}

void led_init() {
  for (int i = 0; i < NUM_LEDS; i++) {
    led_state[i] = 0;
  }
  led_dbg();
}

void led_set(int led, bool on) {
  if (led == LED_OFFLINE) {
    rgbleds.setPixelColor(0, on ? rgbleds.Color(50, 50, 0) : rgbleds.Color(0, 0, 0));
    rgbleds.show();
  }
  led_state[led] = (int) on;
  led_dbg();
}

void wifi_init() {
  WiFi.mode(WIFI_AP_STA);
}

void wifi_connect() {
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.printf("wifi_connect: Connecting to WiFi (%s)", WIFI_SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.print("\nwifi_connect: Connected! IP: ");
  Serial.println(WiFi.localIP());
}

/**
 * Initialize the UDP Server
 * @returns true if initialized, false otherwise
 */
bool udp_init() {
  return rpcUdp.begin(5541);
}

bool udp_send(char *hostname, int port, char *data) {
  bool status = true;
  Serial.printf("udp_send: Begin Packet to %s:%d\n", hostname, port);
  status = rpcUdp.beginPacket(hostname, port);
  if (!status) {
    Serial.println("Unable to Begin Packet");
    return status;
  }
  Serial.printf("udp_send: Write \"%s\"\n", data);
  int numBytes = rpcUdp.write((const uint8_t*) data, strlen(data));
  Serial.printf("udp_send: Wrote %d Bytes\n", numBytes);
  Serial.println("udp_send: End Packet");
  status = rpcUdp.endPacket();
  Serial.printf("udp_send: End Packet: %d\n", (int)status);
  return status;
}

/**
 * Receive bytes into a buffer pointed to by output, if bytes are available
 * @param output Pointer where the buffer should be allocated and filled, caller must free if not NULL
 * @returns true if a message was received, false otherwise
 */
bool udp_receive(char **output) {
  int numBytes = rpcUdp.parsePacket();
  if (numBytes > 0) { // If we have a packet available, allocate a receive buffer and receive data
    Serial.printf("udp_receive: %d Bytes Available\n", numBytes);
    Serial.println("udp_receive: Receiving Packet");
    *output = (char*) malloc(numBytes+1);
    rpcUdp.read(*output, numBytes);
    char *endOfOutput = *output + numBytes;
    *endOfOutput = '\0';
    Serial.printf("udp_receive: Packet Contents: %s\n", *output);
    return true;
  } else { // No packet available, return NULL
    output = NULL;
    return false;
  }
}

bool json_rpc_deserialize(JsonDocument *doc, const char *input) {
  DeserializationError error = deserializeJson(*doc, input);
  if (error) {
    Serial.printf("json_rpc_deserialize: error deserializing: %s", error.c_str());
    return false;
  }
  return true;
}

void rpc_ledupdate(JsonDocument rpc) {
  JsonArray leds = rpc["params"]["leds"].as<JsonArray>();

  // Iterate over the "leds" array and print the values
  int i = 0;
  for (JsonVariant v : leds) {
    for (int unexposed_led : unexposed_leds) {
      if (i == unexposed_led) {
        Serial.printf("rpc_ledupdate: LED %d Unchanged (Unexposed) at %d\n", i, led_state[i]);
        i++;
        break;
      }
    }
    if (i >= NUM_LEDS) {
      break;
    }
    int led = v.as<int>();
    Serial.printf("rpc_ledupdate: LED %d: %d\n", i, led);  // Print each LED value (0, 1, etc.)
    led_state[i] = led;
    i++;
  }
  led_dbg();
}

void rpc_displayupdate(JsonDocument rpc) {
  JsonArray rows = rpc["params"]["rows"].as<JsonArray>();
  const char *rowsCstr[rows.size()];
  int i = 0;
  for (JsonVariant v : rows) {
    if (i >= DISPLAY_NUM_ROWS) {
      break;
    }
    const char* row = v.as<const char*>();
    rowsCstr[i] = row;
    i++;
  }

  lcd_mock(rowsCstr);
}

void process_rpc(JsonDocument rpc) {
  const char *method = rpc["method"];
  if (method == NULL) {
    Serial.println("process_rpc: NULL method Error");
  } else {
    Serial.printf("process_rpc: Method '%s'\n", method);
    lastHostHeartbeat = RTC.getEpoch();
    if (strcmp(method, "DisplayUpdate") == 0) {
      rpc_displayupdate(rpc);
    } else if (strcmp(method, "TriggerBuzzer") == 0) {
      //TODO: Handle Buzzer/Beeper Trigger
    } else if (strcmp(method, "LEDUpdate") == 0) {
      rpc_ledupdate(rpc);
    } else if (strcmp(method, "HostHeartbeat") == 0) {
      return; // All messages with a method set count as heartbeat to avoid oversensitivity
    } else if (strcmp(method, "StateAcknowledge") == 0) {
      expectingState = false;
      led_set(LED_OFFLINE, false);
    } else {
      Serial.printf("process_rpc: Unknown Method '%s'\n", method);
    }
  }
}

void lcd_mock(const char *lines[]) {
  for (int i = 0; i < DISPLAY_NUM_ROWS; i++) {
    Serial.printf("lcd_mock: Line %d: %s\n", i, lines[i]);
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  rgbled_off();
  wifi_init();
  udp_init();
  timeClient.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  if (WiFi.status() != WL_CONNECTED) {
    led_set(LED_OFFLINE, true);
    lcd_mock(new const char*[]{initialConnect ? CONNECTIVITY_ERROR : DEVICE_BOOTING, WIFI_DISCONNECTED});
    Serial.println("WiFi Disconnected, attempting to reconnect...");
    wifi_connect();
    lcd_mock(new const char*[]{initialConnect ? CONNECTIVITY_ERROR : DEVICE_BOOTING, PENDING_STATE_SYNC});
    timeClient.update();
    RTC.setTime(timeClient.getEpochTime());
    Serial.println(RTC.getTime("%A, %B %d %Y %H:%M:%S"));
    lastHostHeartbeat = RTC.getEpoch();
    expectingState = true;
    initialConnect = true;
  }
  if (!expectingState && (RTC.getEpoch() - lastHostHeartbeat) > HEARTBEAT_DELAY_SEC) {
    led_set(LED_OFFLINE, true);
    lcd_mock(new const char*[]{CONNECTIVITY_ERROR, HOST_OFFLINE});
    expectingState = true;
  }
  if (expectingState) {
    udp_send(RPC_SEND_HOST, RPC_SEND_PORT, (char*) RPC_MSG_STATE_REQUEST);
    delay(1000);
  }
  char *data = NULL;
  bool received = udp_receive(&data);
  if (received) {
    JsonDocument rpc;
    bool des_success = json_rpc_deserialize(&rpc, (const char *) data);
    Serial.printf("loop: des_success: %d\n", des_success);
    if (des_success) {
      process_rpc(rpc);
    }
    free(data);
  }
}
