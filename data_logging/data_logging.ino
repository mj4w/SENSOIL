#include <SD.h>
#include <SPI.h>
#include <Arduino.h>
#include <DS3231-RTC.h>

File myFile;
int pinCS = 53;

RTClib myRTC;
DS3231 Clock;

void setup() {
  Serial.begin(9600);
  pinMode(pinCS, OUTPUT);
  while (!Serial) {
    yield();
  }
  
  if (SD.begin()) {
    Serial.println("SD card is ready to use.");
  } else {
    Serial.println("SD card initialize failed");
    return;
  }

  // Extract the year from the __DATE__ string
  String currentYear = String(__DATE__).substring(7);
  String fileType = "Data";
  // Create/Open File
  String fileName = currentYear + "data.csv";
  myFile = SD.open(fileName, FILE_WRITE);
  if (myFile) {
    Serial.println("Writing to the file....");
    // write to file
    myFile.println(__DATE__);
    myFile.println(__TIME__);
    myFile.close();
    Serial.println("Done");
  } else {
    Serial.println("Error opening " + fileName);
  }

  // Reading the file
  // myFile = SD.open(fileName);
  // if (myFile) {
  //   Serial.println("Read:");
  //   while (myFile.available()) {
  //     Serial.write(myFile.read());
  //   }
  //   myFile.close();
  // } else {
  //   Serial.println("Error opening " + fileName);
  // }
}

void loop() {
  // put your main code here, to run repeatedly:
}
