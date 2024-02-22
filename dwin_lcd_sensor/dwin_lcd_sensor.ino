// Include the necessary libraries
#include <SoftwareSerial.h>

// Define the RX and TX pins for SoftwareSerial
const byte rxPin = 7;
const byte txPin = 8;

// Set up SoftwareSerial for communication with DWIN and NPK sensor
SoftwareSerial dwinSerial(rxPin, txPin);
SoftwareSerial mod(4, 5);  // RX, TX

// Data frames for the sensors
unsigned char nitrogen[8] = {0x5A, 0xA5, 0x05, 0x82, 0x51, 0x00, 0x00, 0x00};
unsigned char phosphorus[8] = {0x5A, 0xA5, 0x05, 0x82, 0x52, 0x00, 0x00, 0x00};
unsigned char potassium[8] = {0x5A, 0xA5, 0x05, 0x82, 0x53, 0x00, 0x00, 0x00};
unsigned char phlevel[8] = {0x5A, 0xA5, 0x05, 0x82, 0x54, 0x00, 0x00, 0x00};
unsigned char electrical[8] = {0x5A, 0xA5, 0x05, 0x82, 0x55, 0x00, 0x00, 0x00};
unsigned char moisture[8] = {0x5A, 0xA5, 0x05, 0x82, 0x56, 0x00, 0x00, 0x00};

float moist, ec, pH, nitroValue, phosValue, potas;
String season, texture, variety;

void setup()
{
  // Initialize Serial communication
  Serial.begin(115200);
  // Initialize SoftwareSerial communication with DWIN and NPK sensor
  dwinSerial.begin(115200);
  mod.begin(4800);
}

void loop()
{
  // Query data from the NPK sensor
  byte queryData[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
  byte receivedData[19];
  mod.write(queryData, sizeof(queryData));
  delay(1000);

  // Check if there is enough data available to read
  if (mod.available() >= sizeof(receivedData))
  {
    // Read the received data into the receivedData array
    mod.readBytes(receivedData, sizeof(receivedData));

    // Parse and print the received data in decimal format
    unsigned int soilHumidity = (receivedData[3] << 8) | receivedData[4];
    unsigned int soilTemperature = (receivedData[5] << 8) | receivedData[6];
    unsigned int soilConductivity = (receivedData[7] << 8) | receivedData[8];
    unsigned int soilPH = (receivedData[9] << 8) | receivedData[10];
    unsigned int nitrogen = (receivedData[11] << 8) | receivedData[12];
    unsigned int phosphorus = (receivedData[13] << 8) | receivedData[14];
    unsigned int potassium = (receivedData[15] << 8) | receivedData[16];

    moist = (soilHumidity / 10.0);
    ec = (soilConductivity / 1000.0);
    pH = (soilPH / 10.0);
    nitroValue = (nitrogen / 1000.0 * 10.0);
    phosValue = (phosphorus);
    potas = (potassium / 1000.0 / 39.0983 * 100.0);

    delay(5000);
  }

  // Convert float values to integers
  int n = static_cast<int>(nitroValue * 100);
  int p = static_cast<int>(phosValue * 100);
  int k = static_cast<int>(potas * 100);
  int h = static_cast<int>(pH * 100);
  int e = static_cast<int>(ec * 100);
  int m = static_cast<int>(moist * 100);

  // Update the data frames
  nitrogen[6] = highByte(n);
  nitrogen[7] = lowByte(n);
  dwinSerial.write(nitrogen, 8);

  phosphorus[6] = highByte(p);
  phosphorus[7] = lowByte(p);
  dwinSerial.write(phosphorus, 8);

  potassium[6] = highByte(k);
  potassium[7] = lowByte(k);
  dwinSerial.write(potassium, 8);

  phlevel[6] = highByte(h);
  phlevel[7] = lowByte(h);
  dwinSerial.write(phlevel, 8);

  electrical[6] = highByte(e);
  electrical[7] = lowByte(e);
  dwinSerial.write(electrical, 8);

  moisture[6] = highByte(m);
  moisture[7] = lowByte(m);
  dwinSerial.write(moisture, 8);

  delay(500);
}
