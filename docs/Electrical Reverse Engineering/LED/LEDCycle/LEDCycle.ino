/*
LED Board | Arduino
===================================
Black     | 5V w/ 220 Ohm Resistor
Brown     | D5
Red       | D4
Orange    | D3
Yellow    | D2
*/

#define wait 6
#define offline 7
#define message 8
#define other 9

void setup() {
  pinMode(wait, OUTPUT);
  pinMode(offline, OUTPUT);
  pinMode(message, OUTPUT);
  pinMode(other, OUTPUT);
  digitalWrite(wait, HIGH);
  digitalWrite(offline, HIGH);
  digitalWrite(message, HIGH);
  digitalWrite(other, HIGH);
  delay(2000);
}


void loop() {
  digitalWrite(wait, LOW);
  delay(500);
  digitalWrite(offline, LOW);
  delay(500);
  digitalWrite(message, LOW);
  delay(500);
  digitalWrite(other, LOW);
  delay(500);
  digitalWrite(wait, HIGH);
  delay(500);
  digitalWrite(offline, HIGH);
  delay(500);
  digitalWrite(message, HIGH);
  delay(500);
  digitalWrite(other, HIGH);
  delay(500);
}
