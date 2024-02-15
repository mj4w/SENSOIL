#include <EEPROM.h>

void setup() {
  Serial.begin(9600);
  int targetIndex = 1;
  int value = EEPROM.read(targetIndex);
  value++;

  EEPROM.write(targetIndex, value);

  Serial.print("EEPROM value at index ");
  Serial.println(value);
}

void loop() {
}
