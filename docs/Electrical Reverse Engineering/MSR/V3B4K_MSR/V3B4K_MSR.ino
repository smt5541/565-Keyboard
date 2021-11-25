//Pin Configuration:
//MSR Wire | Our Wire | Pin  | Function
//========================================
//Brown    | Yellow   | 2    | Data
//Red      | Green    | 3    | Clock
//Orange   | Blue     | 4    | Card Loaded
//Yellow   | Red      | 5V   |
//Green    | Black    | GND  |

#include <MagStripe.h>
#include <Wire.h>

static const byte READ_LED = 13;
MagStripe card;
static const byte DATA_BUFFER_LEN = 108;
static char data[DATA_BUFFER_LEN];


void setup() {
  pinMode(READ_LED, OUTPUT);
  digitalWrite(READ_LED, LOW);
  Serial.begin(115200);
  pinMode(2, INPUT);
  pinMode(3, INPUT);
  pinMode(4, INPUT);
  card.begin(2);
  Wire.begin(2);
  Wire.onRequest(sendUpdate);
}

void sendUpdate() {
  if (data[0] != '\0')
    Wire.write(data);
  else
    Wire.write('\0');
}

void loop() {
  if (!card.available()) {
    data[0] = '\0';
    return;
  }
  digitalWrite(READ_LED, HIGH);

  short chars = card.read(data, DATA_BUFFER_LEN);

  digitalWrite(READ_LED, LOW);

  if (chars < 0) {
    Serial.println("Error Reading Card");
    return;
  }

  Serial.println(data);
  
}
