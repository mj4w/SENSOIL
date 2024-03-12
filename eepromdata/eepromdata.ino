#include <SD.h>
#include <EEPROM.h>

const int chipSelect = 53;  // CS pin
File dataFile;
const char* baseFilename = "Sensoil";
const char* fileExtension = ".csv";
unsigned long fileCounter = 0;

void setup() {
  // Initialize SD card
  if (SD.begin(chipSelect)) {
    Serial.begin(9600);
    // Retrieve the counter value from EEPROM
    retrieveCounterFromEEPROM();
    // Increment the counter and create a unique file name
    generateUniqueFileName();
    // Write data to the file
    writeDataToFile();
  } else {
    Serial.println("SD card initialization failed.");
  }
}

void loop() {
  // Your main loop code goes here
}

void retrieveCounterFromEEPROM() {
  // Retrieve the counter value from EEPROM
  EEPROM.get(0, fileCounter);
  // Increment the counter for the next file
  fileCounter++;
}

void saveCounterToEEPROM() {
  // Save the updated counter value to EEPROM
  EEPROM.put(0, fileCounter);
  // EEPROM.commit();
}

void generateUniqueFileName() {
  // Create a unique file name with the base name, counter, and extension
  char uniqueFileName[1000];
  snprintf(uniqueFileName, sizeof(uniqueFileName), "%s%lu%s", baseFilename, fileCounter, fileExtension);

  // Print the unique file name for verification
  Serial.print("Unique File Name: ");
  Serial.println(uniqueFileName);

  // Open the file for writing
  dataFile = SD.open(uniqueFileName, FILE_WRITE);
  if (!dataFile) {
    Serial.println("Error opening file for writing.");
  }
}

void writeDataToFile() {
  // Write some data to the file
  if (dataFile) {
    dataFile.println("Hello, this is some data!");
    dataFile.close();
    Serial.println("Data written to file successfully.");

    // Save the updated counter value to EEPROM
    saveCounterToEEPROM();
  } else {
    Serial.println("Error writing to file.");
  }
}
