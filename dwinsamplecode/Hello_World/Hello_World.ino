#include <Arduino.h>
#include <SoftwareSerial.h>

// Define SoftwareSerial pins
#define RX_PIN 17
#define TX_PIN 16

SoftwareSerial DGUS_SERIAL(RX_PIN, TX_PIN);

void setup() {
  Serial.begin(9600); // Serial communication for debugging
  DGUS_SERIAL.begin(115200); // Adjust the baud rate based on your DWIN display settings
}

void loop() {
  // Check if there is data available on the serial port
  if (DGUS_SERIAL.available() > 0) {
    // Read the incoming data
    char incomingChar = DGUS_SERIAL.read();

    // Process the incoming data as needed
    Serial.print("Received data from DWIN: ");
    Serial.println(incomingChar);
  }

  // Your main loop code here
}
