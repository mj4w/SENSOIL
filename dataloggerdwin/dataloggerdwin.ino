#include <SoftwareSerial.h>
#include <SPI.h>
#include <SD.h>

const int chipSelect = 53; // Change this to the CS pin of your SD card module
const int maxFiles = 10; // Maximum number of files, adjust as needed

String fileNames[maxFiles]; // Array to store file names
SoftwareSerial dwinSerial(10,11); // RX, TX
SoftwareSerial mySerial1(12,13);
String season, variety, texture;
float nitrogen, phosphorus, potassium, ec, ph, moisture;
int n, p, k;
String firstApplication, secondApplication, thirdApplication;
String filename;

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
void dwinListen(){
  
  int startAdd = 00;
  int endAdd = 00;
  int dataVal = 0;
  String address;
  // // List all files on the SD card
  listFiles();
  String var1 = fileNames[0];
  String var2 = fileNames[1];
  String var3 = fileNames[2];
  String var4 = fileNames[3];
  String var5 = fileNames[4];
  String var6 = fileNames[5];
  String var7 = fileNames[6];
  String var8 = fileNames[7];
  String var9 = fileNames[8];
  for (int i = 0; i < maxFiles; ++i) 
  {
    if (i == 0) {
        unsigned char data1[] = {0x5A,0xA5,0x10,0x82,0x10,0x00,0x64,0x61,0x74,0x61,0x31};
        dwinSerial.write(data1,11);
        Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
    } else if (i == 1 ) {
        unsigned char data2[] = {0x5A,0xA5,0x10,0x82,0x12,0x30,0x64,0x61,0x74,0x61,0x32};
        dwinSerial.write(data2,11);
        Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
    } else if (i == 2 ) {
        unsigned char data3[] = {0x5A,0xA5,0x09,0x82,0x42,0x00,0x4C,0x69,0x67,0x68,0x74,0x2e};
        dwinSerial.write(data3,11);
        Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
    } else if (i == 3 ) {
        unsigned char data4[] = {0x5A,0xA5,0x10,0x82,0x12,0x50,0x64,0x61,0x74,0x61,0x34};
        dwinSerial.write(data4,11);
        Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
    }
    // } else if (i == 4 ) {
    //     unsigned char data1[] = {0x5A,0xA5,0x10,0x82,0x12,0x20,0x64,0x61,0x74,0x61,0x35};
    //     dwinSerial.write(data1,11);
    //     Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
    // } else if (i == 5 ) {
    //     unsigned char data1[] = {0x5A,0xA5,0x10,0x82,0x12,0x20,0x64,0x61,0x74,0x61,0x36};
    //     dwinSerial.write(data1,11);
    //     Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
    // } else if (i == 6 ) {
    //     unsigned char data1[] = {0x5A,0xA5,0x10,0x82,0x12,0x20,0x64,0x61,0x74,0x61,0x37};
    //     dwinSerial.write(data1,11);
    //     Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
    // } else if (i == 7 ) {
    //     unsigned char data1[] = {0x5A,0xA5,0x10,0x82,0x12,0x20,0x64,0x61,0x74,0x61,0x38};
    //     dwinSerial.write(data1,11);
    //     Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
    // } else if (i == 8 ) {
    //     unsigned char data1[] = {0x5A,0xA5,0x10,0x82,0x12,0x20,0x64,0x61,0x74,0x61,0x39};
    //     dwinSerial.write(data1,11);
    //     Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
    // } else if (i == 9 ) {
    //     unsigned char data1[] = {0x5A,0xA5,0x10,0x82,0x12,0x20,0x64,0x61,0x74,0x61,0x31,0x30};
    //     dwinSerial.write(data1,11);
    //     Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
    // }
  }


  while (dwinSerial.available()) {
      int inhex = dwinSerial.read();

      if( inhex == 90 || inhex == 165){
        continue;
      }
      
      for (int i=1; i<=inhex; i++) {
        // Serial.print(inhex);
        while(!dwinSerial.available()); 
        int incomingByte = dwinSerial.read();
        // Serial.print(incomingByte);
        if( i == 2 ){
          startAdd = incomingByte;
        }
        if( i == 3){
          endAdd = incomingByte;
        }
        
        if( i == inhex){
          dataVal = incomingByte;
        }
      }
    address = String(startAdd)+String(endAdd);
    Serial.println("Address "+ address +" Data "+String(dataVal));

    // if (address == "1816" || address == "15590" && dataVal == 90 || address == "13075" || address == "3420"){
    //   deleteFile(filename);
    //   Serial.println(filename);
    // }

    // if (address == "185" || address == "1865" || address == "90165"){
    //   filename = var1;
    //   readDataAndAssignVariables(var1);
    // } else if (address == "186" || address == "3480" || address == "13079") {
    //   filename = var2;
    //   readDataAndAssignVariables(var2);
    // } else if (address == "187" || address == "16280" || address == "7990") {
    //   filename = var3;
    //   readDataAndAssignVariables(var3);
    // } else if (address == "188" || address == "4834" || address == "190" || address == "45255") {
    //   filename = var4;
    //   readDataAndAssignVariables(var4);
    // } else if (address == "189" || address == "48162" || address == "105208" || address == "16266") {
    //   filename = var5;
    //   readDataAndAssignVariables(var5);
    // } 
    // else if (address == "1820" && dataVal == 1) {
    //   filename = var5;
    //   readDataAndAssignVariables(var5);
    // } else if (address == "1819" && dataVal == 1) {
    //   filename = var6;
    //   readDataAndAssignVariables(var6);
    // } else if (address == "1818" && dataVal == 1) {
    //   filename = var7;
    //   readDataAndAssignVariables(var7);
    // } else if (address == "1817" && dataVal == 1) {
    //   filename = var8;
    //   readDataAndAssignVariables(var8);
    // } else if (address == "189" && dataVal == 1) {
    //   filename = var9;
    //   readDataAndAssignVariables(var9);
    // }

  }
}
void deleteFile(String fileName){
  if (SD.exists(fileName)) {
  // File exists, delete it
  if (SD.remove(fileName)) {
    Serial.println("File deleted successfully.");
  } else {
    Serial.println("Error deleting file.");
  }
  } else {
    Serial.println("File does not exist.");
  }
}
void listFiles() {
  // Serial.println("Files on SD card:");
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
      // Serial.println(fileNames[count]);
      count++;
    }
    entry.close();
  }
  root.close();
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
void setup() {
  
  Serial.begin(9600);
  mySerial1.begin(4800);
  dwinSerial.begin(115200);
  Serial.println(" DWIN Arduino Button Demo ");
  Serial.println("Lets Begin");

  if (!SD.begin(chipSelect)) {
  Serial.println("SD card initialization failed. Check your connections.");
  return;
  }
  Serial.println("SD card initialized successfully.");


  
}

void loop() { 
  dwinListen();
  logData();

}
