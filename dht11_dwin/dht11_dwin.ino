#include <SoftwareSerial.h>

unsigned char Buffer[9];
#define nitro_value 0x51
#define phos_value 0x52
#define potas_value 0x53
#define pH_value 0x54
#define ec_value 0x55
#define moist_value 0x56

unsigned char Nitro[8] = {0x5A, 0xA5, 0x05, 0x82, nitro_value, 0x00, 0x00, 0x00};
unsigned char Phos[8] = {0x5A, 0xA5, 0x05, 0x82, phos_value, 0x00, 0x00, 0x00};
unsigned char Potas[8] = {0x5A, 0xA5, 0x05, 0x82, potas_value, 0x00, 0x00, 0x00};
unsigned char PH[8] = {0x5A, 0xA5, 0x05, 0x82, pH_value, 0x00, 0x00, 0x00};
unsigned char EC[8] = {0x5A, 0xA5, 0x05, 0x82, ec_value, 0x00, 0x00, 0x00};
unsigned char Moist[8] = {0x5A, 0xA5, 0x05, 0x82, moist_value, 0x00, 0x00, 0x00};


float moisture, ec, pH, nitroValue, phosValue, potas;
String season,texture,variety;
void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);
  Serial1.begin(4800);
}

void loop() {
  Serial.print(Serial2.available());
  byte queryData[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
  byte receivedData[19];
  Serial1.write(queryData, sizeof(queryData));  // Send the query data to the NPK sensor
  delay(1000);  // Wait for 1 second
  if (Serial1.available() >= sizeof(receivedData)) {  // Check if there are enough bytes available to read
    Serial1.readBytes(receivedData, sizeof(receivedData));  // Read the received data into the receivedData array

    // Parse and print the received data in decimal format
    unsigned int soilHumidity = (receivedData[3] << 8) | receivedData[4];
    unsigned int soilTemperature = (receivedData[5] << 8) | receivedData[6];
    unsigned int soilConductivity = (receivedData[7] << 8) | receivedData[8];
    unsigned int soilPH = (receivedData[9] << 8) | receivedData[10];
    unsigned int nitrogen = (receivedData[11] << 8) | receivedData[12];
    unsigned int phosphorus = (receivedData[13] << 8) | receivedData[14];
    unsigned int potassium = (receivedData[15] << 8) | receivedData[16];

    moisture = (soilHumidity / 10.0);
    // temp = soilTemperature;
    ec = (soilConductivity / 1000.0);
    pH = (soilPH / 10.0);
    nitroValue = (nitrogen / 1000.0 * 10.0);
    phosValue = (phosphorus);
    potas = (potassium / 1000.0 / 39.0983 * 100.0);

    Serial.print("Nitrogen");
    Serial.println(nitroValue);
    Serial.print("Phosphorus:");
    Serial.println(phosValue);
    Serial.print("Potassium");
    Serial.println(potas);
    Serial.print("EC");
    Serial.println(ec);
    Serial.print("PH");
    Serial.println(pH);
    Serial.print("Moisture");
    Serial.println(moisture);

    delay(5000);
  }
  while (Serial2.available()) {
    dwin();
  }



  Data_Arduino_to_Display();
}
void Data_Arduino_to_Display() {
  delay(100);
  season = "WET";
  texture = "LIGHT";
  variety = "HYBRID";
  // Update nitroValue and phosValue with actual sensor readings
  int n = static_cast<int>(nitroValue * 100);  
  int ps = static_cast<int>(phosValue * 100);  
  int k = static_cast<int>(potas * 100);
  int ph = static_cast<int>(pH * 100);
  int e = static_cast<int>(ec * 100);
  int m = static_cast<int>(moisture * 100);


  Serial.print("Nitrogen: ");
  Serial.println(n);
  Serial.print("Phosphorus: ");
  Serial.println(ps);
  Serial.print("Potassium: ");
  Serial.println(k);
  Serial.print("pH: ");
  Serial.println(ph);
  Serial.print("EC: ");
  Serial.println(e);
  Serial.print("Moisture: ");
  Serial.println(m);



  /*------Send Data to Display------*/

  Nitro[6] = highByte(n);
  Nitro[7] = lowByte(n);
  Serial2.write(Nitro, 8);
  
  Phos[6] = highByte(ps);
  Phos[7] = lowByte(ps);
  Serial2.write(Phos, 8);

  Potas[6] = highByte(k);
  Potas[7] = lowByte(k);
  Serial2.write(Potas, 8);
  
  PH[6] = highByte(ph);
  PH[7] = lowByte(ph);
  Serial2.write(PH, 8);
  
  EC[6] = highByte(e);
  EC[7] = lowByte(e);
  Serial2.write(EC, 8);
  
  Moist[6] = highByte(m);
  Moist[7] = lowByte(m);
  Serial2.write(Moist, 8);



}

void dwin() {
  if (Serial2.available()) {
    for (int i = 0; i <= 8; i++) //5A A5 06 83 55 00 01 00 01 frame sample received
    {
      Buffer[i] = Serial2.read();
    }
  }
  delay(10);  // must include delay
}
