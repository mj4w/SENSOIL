#include <SPI.h>
#include <SD.h>

const int chipSelect = 53; // Change this to the CS pin of your SD card module
const int maxFiles = 10; // Maximum number of files, adjust as needed

String fileNames[maxFiles]; // Array to store file names
// Variables for different types of data
String season, variety, texture;
float nitrogen, phosphorus, potassium, ec, ph, moisture;
int n, p, k;
String firstApplication, secondApplication, thirdApplication;
void listFiles() {
  Serial.println("Files on SD card:");
  File root = SD.open("/");
  int count = 0;

  while (count < maxFiles) {
    File entry = root.openNextFile();
    if (!entry) {
      // no more files
      break;
    }

    if (!entry.isDirectory()) {
      // Check if the entry is a file, not a directory
      fileNames[count] = entry.name();
      Serial.println(fileNames[count]);
      count++;
    }
    entry.close();
  }
  root.close();
}
void setup() {
  Serial.begin(9600);

  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed. Check your connections.");
    return;
  }

  Serial.println("SD card initialized successfully.");
  // logData(); // Commented out logData to avoid writing unnecessary data during testing
  // // List all files on the SD card
  listFiles();
  String var1 = fileNames[0];
  String var2 = fileNames[1];
  String var3 = fileNames[2];
  // Access individual file names using the array
//   if (maxFiles > 0) {
//     Serial.println("var1: " + var1);
//   }
// 4
//   if (maxFiles > 1) {
//     Serial.println("var2: " + var2);
//   }
//   if (maxFiles > 2) {
//     Serial.println("var2: " + var3);
//   }
  readDataAndAssignVariables(var1);
}

void logData() {
  String fileName = createFileName();

  // Create a new file
  File dataFile = SD.open(fileName, FILE_WRITE);

  // Check if the file opened successfully
  if (dataFile) {
    // Write data to the file
    dataFile.println("season, DRY");
    dataFile.println("variety, LIGHT");
    dataFile.println("texture,Inbred");

    dataFile.println("Nitrogen, 2.30");
    dataFile.println("Phosphorus,  4.00");
    dataFile.println("Potassium, 3.00");
    dataFile.println("EC, 3.00");
    dataFile.println("ph, 4.00");
    dataFile.println("moisture, 4.00");

    dataFile.println("n, 40");
    dataFile.println("p, 50");
    dataFile.println("k, 60");

    dataFile.println("1st application, 10 bag 2");
    dataFile.println("2nd application, 30 bag 2");
    dataFile.println("3rd application, 40 bag 3");

    Serial.println("Data logged to: " + fileName);

    // Close the file
    dataFile.close();
  } else {
    Serial.println("Error opening file: " + fileName);
  }
}

void readDataAndAssignVariables(String fileName) {
  File dataFile = SD.open(fileName);

  if (dataFile) {
    Serial.println("Reading data from file: " + String(fileName));

    // Read data from the file line by line and assign values to variables
    while (dataFile.available()) {
      String dataLine = dataFile.readStringUntil('\n');
      parseAndAssignVariables(dataLine);
    }

    // Close the file
    dataFile.close();

    // Print the assigned values for verification
    printAssignedValues();
  } else {
    Serial.println("Error opening file: " + String(fileName));
  }
}

void parseAndAssignVariables(String dataLine) {
  // Split the dataLine into individual values
  int commaIndex = dataLine.indexOf(',');

  if (commaIndex != -1) {
    String header = dataLine.substring(0, commaIndex);
    String value = dataLine.substring(commaIndex + 1);

    // Assign values based on the type of header
    if (header == "season") {
      season = value;
    } else if (header == "variety") {
      variety = value;
    } else if (header == "texture") {
      texture = value;
    } else if (header == "Nitrogen") {
      nitrogen = value.toFloat();
    } else if (header == "Phosphorus") {
      phosphorus = value.toFloat();
    } else if (header == "Potassium") {
      potassium = value.toFloat();
    } else if (header == "EC") {
      ec = value.toFloat();
    } else if (header == "ph") {
      ph = value.toFloat();
    } else if (header == "moisture") {
      moisture = value.toFloat();
    } else if (header == "n") {
      n = value.toInt();
    } else if (header == "p") {
      p = value.toInt();
    } else if (header == "k") {
      k = value.toInt();
    } else if (header == "1st application") {
      firstApplication = value;
    } else if (header == "2nd application") {
      secondApplication = value;
    } else if (header == "3rd application") {
      thirdApplication = value;
    }
  }
}

void printAssignedValues() {
  Serial.println("Season: " + season);
  Serial.println("Variety: " + variety);
  Serial.println("Texture: " + texture);
  Serial.println("Nitrogen: " + String(nitrogen));
  Serial.println("Phosphorus: " + String(phosphorus));
  Serial.println("Potassium: " + String(potassium));
  Serial.println("EC: " + String(ec));
  Serial.println("pH: " + String(ph));
  Serial.println("Moisture: " + String(moisture));
  Serial.println("N: " + String(n));
  Serial.println("P: " + String(p));
  Serial.println("K: " + String(k));
  Serial.println("1st Application: " + firstApplication);
  Serial.println("2nd Application: " + secondApplication);
  Serial.println("3rd Application: " + thirdApplication);
}

String createFileName() {
  String fileName = "data_" + String(random(1000)) + ".csv";
  return fileName;
}

void loop() {
  // logData(); // Commented out logData to avoid writing unnecessary data during testing
  delay(5000); // Adjust the delay as needed
  // You can add other code in the loop if needed
}
