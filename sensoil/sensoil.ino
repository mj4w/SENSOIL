#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>
#include <DS3231-RTC.h>
#include "Adafruit_Thermal.h"
#include <EEPROM.h>


const int chipSelect = 53; // Change this to the CS pin of your SD card module
const int maxFiles = 10; // Maximum number of files, adjust as needed
unsigned long fileCounter = 0;
String fileNames[maxFiles]; // Array to store file names
File dataFile;
// SWITCH
#define WET_SELECTOR 23
#define DRY_SELECTOR 22

#define INBRED_SELECTOR 24
#define HYBRID_SELECTOR 25

#define LIGHT_SELECTOR 26 
#define MEDIUM_SELECTOR 27
#define HEAVY_SELECTOR 28

// DWIN LCD
unsigned char Buffer[9];
#define nitro_value_dwin 0x51
#define phos_value_dwin 0x52
#define potas_value_dwin 0x53
#define pH_value_dwin 0x54
#define ec_value_dwin 0x55
#define moist_value_dwin 0x56

#define nit_both_dwin 0x60
#define phos_both_dwin 0x61
#define potas_both_dwin 0x62

unsigned char Nitro_Dwin[8] = {0x5A, 0xA5, 0x05, 0x82, nitro_value_dwin, 0x00, 0x00, 0x00};
unsigned char Phos_Dwin[8] = {0x5A, 0xA5, 0x05, 0x82, phos_value_dwin, 0x00, 0x00, 0x00};
unsigned char Potas_Dwin[8] = {0x5A, 0xA5, 0x05, 0x82, potas_value_dwin, 0x00, 0x00, 0x00};
unsigned char PH_Dwin[8] = {0x5A, 0xA5, 0x05, 0x82, pH_value_dwin, 0x00, 0x00, 0x00};
unsigned char EC_Dwin[8] = {0x5A, 0xA5, 0x05, 0x82, ec_value_dwin, 0x00, 0x00, 0x00};
unsigned char Moist_Dwin[8] = {0x5A, 0xA5, 0x05, 0x82, moist_value_dwin, 0x00, 0x00, 0x00};

// Nutrient Hexa Data
unsigned char Nit_Both_Dwin[8] = {0x5A, 0xA5, 0x05, 0x82, nit_both_dwin, 0x00, 0x00, 0x00};
unsigned char Phos_Both_Dwin[8] = {0x5A, 0xA5, 0x05, 0x82, phos_both_dwin, 0x00, 0x00, 0x00};
unsigned char Potas_Both_Dwin[8] = {0x5A, 0xA5, 0x05, 0x82, potas_both_dwin, 0x00, 0x00, 0x00};

File myFile;
RTClib myRTC;
DS3231 Clock;

// SoftwareSerial Serial1(12,13);
// SoftwareSerial Serial2(10,11);
SoftwareSerial mySerial(14,15); // TX, RX
Adafruit_Thermal printer(&mySerial);
String nit_value,phos_value,potas_value,ph_value,soil_salinity_class,mois_value;
String season,variety,texture;
String filename;
String prefix;
String format;
float nitro,phos,potas,pH,ec,moisture;
int nit_both,phos_both,potas_both;
int nitro_split1,phos_split1,nitro_split2,phos_split2,nitro_split3,phos_split3,potas_split1,potas_split2,potas_split3;
int button_selector_season = 0;
int button_selector_variety = 0;
int button_selector_texture = 0;
int buttonState = 0;
int oldButtonState = LOW;

const unsigned int PRINT_BUTTON = 8;

String month;
int day;
int year;

void parseDate(String dateStr) {
  // Assuming the date string format is "MMM DD YYYY"
  char monthStr[4];
  char dayStr[3];
  char yearStr[5];

  // Extract substrings from the date string
  dateStr.substring(0, 3).toCharArray(monthStr, 4);
  dateStr.substring(4, 6).toCharArray(dayStr, 3);
  dateStr.substring(7, 11).toCharArray(yearStr, 5);

  // Convert char arrays to strings
  month = monthStrToNumber(monthStr);
  day = atoi(dayStr);
  year = atoi(yearStr);
}

String monthStrToNumber(const char* monthStr) {
  // Convert three-letter month abbreviation to a zero-padded string
  const char* months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  for (int i = 0; i < 12; ++i) {
    if (strcmp(monthStr, months[i]) == 0) {
      return String(i + 1, DEC);
    }
  }
  return "00";  // Return "00" if month abbreviation is not recognized
}

void sendDateOverSerial() {
  unsigned char Month[8] = {0x5A, 0xA5, 0x05, 0x82, 0x36, 0x00, 0x00, 0x00};
  unsigned char Day[8] = {0x5A, 0xA5, 0x05, 0x82, 0x37, 0x00, 0x00, 0x00};
  unsigned char Year[8] = {0x5A, 0xA5, 0x05, 0x82, 0x38, 0x00, 0x00, 0x00};

  Month[6] = highByte(month.toInt());
  Month[7] = lowByte(month.toInt());
  Serial2.write(Month, 8);

  Day[6] = highByte(day);
  Day[7] = lowByte(day);
  Serial2.write(Day, 8);

  Year[6] = highByte(year);
  Year[7] = lowByte(year);
  Serial2.write(Year, 8);
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
void formatSDCard() {
  Serial.println("Deleting all files on SD card...");

  File root = SD.open("/");
  int count = 0;

  // Iterate through each file in the root directory
  while (true) {
    File entry = root.openNextFile();
    if (!entry) {
      // no more files
      break;
    }

    if (!entry.isDirectory()) {
      // Check if the entry is a file, not a directory
      String fileName = entry.name();
      Serial.print("Deleting file: ");
      Serial.println(fileName);

      // Delete the file
      if (SD.remove(fileName)) {
        Serial.println("File deleted successfully.");
      } else {
        Serial.println("Error deleting file.");
      }

      count++;
    }
    entry.close();
  }

  Serial.println("All files deleted.");
  root.close();
}
String extractPrefix(String filename) {
  int underscorePos = filename.indexOf('_');

  String prefix = filename.substring(0, underscorePos);

  return prefix;
}
String extractNumber(String input) {
  // Find the position of the 'ST' substring
  int stPos = input.indexOf("ST");

  // Extract the substring starting from 'ST' to the end of the string
  String remaining = input.substring(stPos + 2);

  // Find the position of the '.CSV' substring
  int csvPos = remaining.indexOf(".CSV");

  // Extract the number from 'ST' to the '.CSV' position
  String number = remaining.substring(0, csvPos);

  return number;
}
void dwinListen(){

  while(true){
    String dateStr = __DATE__;
    
    parseDate(dateStr);
    // Serial.println("Month: " + String(month));
    sendDateOverSerial();
    // delay(5000);
    switches();
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
    String var10 = fileNames[9];
    String var11 = fileNames[10];
    String var12 = fileNames[11];
    String var13 = fileNames[12];
    String var14 = fileNames[13];
    String var15 = fileNames[14];
    String var16 = fileNames[15];
    String var17 = fileNames[16];
    String var18 = fileNames[17];
    String var19 = fileNames[18];
    String var20 = fileNames[19];
    for (int i = 0; i < maxFiles; ++i) 
    {
      Serial.println(fileNames[i]);
      // delay(5000);
      // format = fileNames[i];
      prefix = extractPrefix(fileNames[i]); 
      // Serial.println("Prefix: " + prefix);
      String extractedNumber = extractNumber(fileNames[i]);

      // Serial.println("Extracted Number: " + extractedNumber);

      // delay(2000);
      if (i == 0) {
        unsigned char Data1[8] = {0x5A, 0xA5, 0x05, 0x82, 0x24, 0x00, 0x00, 0x00};
        unsigned char CharData1[] = {0x5A,0xA5,0x05,0x82,0x31,0x00,0x53,0x54};

        int dataF1 = extractedNumber.toInt();
        Data1[6] = highByte(dataF1);
        Data1[7] = lowByte(dataF1);
        Serial2.write(Data1, 8);
        Serial2.write(CharData1,8);
        // Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
      }
      if (i == 1){
        unsigned char Data2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x25, 0x00, 0x00, 0x00};
        int dataF2 = extractedNumber.toInt();
        Serial.println("Var2" + var2);
        Data2[6] = highByte(dataF2);
        Data2[7] = lowByte(dataF2);
        Serial2.write(Data2, 8);
        // Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
      }
      if (i == 2){
        
      }

    }
    printData(prefix);

    while (Serial2.available()) {
        int inhex = Serial2.read();
        if( inhex == 90 || inhex == 165){
          continue;
        }
        
        for (int i=1; i<=inhex; i++) {
          // Serial.print(inhex);
          while(!Serial2.available()); 
          int incomingByte = Serial2.read();
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
      Serial.print("Address " + address + " Data " + String(dataVal) + " | ");
      delay(100);
      // if (address == "1656"){
      //   deleteFile(filename);
      //   Serial.println(filename);
      // }

      switch (address.toInt()) {
        case 185:
            filename = var1;
            readDataAndAssignVariables(var1);
            delay(5000);
            break;
        case 186:
            filename = var2;
            readDataAndAssignVariables(var2);
            delay(5000);
            break;
        case 187:
            filename = var3;
            readDataAndAssignVariables(var3);
            delay(5000);
            break;
        case 188:
            filename = var4;
            readDataAndAssignVariables(var4);
            delay(5000);
            break;
        case 189:
            filename = var5;
            readDataAndAssignVariables(var5);
            delay(5000);
            break;
        case 1816:
            filename = var6;
            readDataAndAssignVariables(var6);
            delay(5000);
            break;
        case 1817:
            filename = var7;
            readDataAndAssignVariables(var7);
            delay(5000);
            break;
        case 1818:
            filename = var8;
            readDataAndAssignVariables(var8);
            delay(5000);
            break;
        case 1819:
            filename = var9;
            readDataAndAssignVariables(var9);
            delay(5000);
            break;
        case 1820:
            filename = var10;
            readDataAndAssignVariables(var10);
            delay(5000);
            break;
        case 1821:
            filename = var11;
            readDataAndAssignVariables(var11);
            delay(5000);
            break;
        case 1822:
            filename = var12;
            readDataAndAssignVariables(var12);
            delay(5000);
            break;
        case 1823:
            filename = var13;
            readDataAndAssignVariables(var13);
            delay(5000);
            break;
        case 1824:
            filename = var14;
            readDataAndAssignVariables(var14);
            delay(5000);
            break;
        case 1825:
            filename = var15;
            readDataAndAssignVariables(var15);
            delay(5000);
            break;
        case 1832:
            filename = var16;
            readDataAndAssignVariables(var16);
            delay(5000);
            break;
        case 1833:
            filename = var17;
            readDataAndAssignVariables(var17);
            delay(5000);
            break;
        case 1834:
            filename = var18;
            readDataAndAssignVariables(var18);
            delay(5000);
            break;
        case 1835:
            filename = var19;
            readDataAndAssignVariables(var19);
            delay(5000);
            break;
        case 1836:
            filename = var20;
            readDataAndAssignVariables(var20);
            delay(5000);
            break;
        case 510:
            deleteFile(filename);
            Serial.println(filename);
            break;
        // NPK result
        case 530:
            npkSense();
            restartDWIN();
            delay(20000);
            break;
        // Format All
        case 520:
          formatSDCard();
          delay(5000);
          break;

        // Add more cases for other addresses as needed

        default:
            break;
      }
      //   
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

}
void readDataAndAssignVariables(String fileName) {
  File dataFile = SD.open(fileName);
  String prefix = extractPrefix(fileName); 
  Serial.println("Prefix: " + prefix);
  if (dataFile) {
    Serial.println("Reading data from file: " + String(fileName));

    // Read data from the file line by line and assign values to variables
    while (dataFile.available()) {
      String dataLine = dataFile.readStringUntil('\n');
      parseAndAssignVariables(dataLine);
    }
    printData(prefix);
    // Close the file
    dataFile.close();

    // Print the assigned values for verification
    printAssignedValues(prefix);

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
    if (header == "Season") {
      season = value;
    } else if (header == "Variety") {
      variety = value;
    } else if (header == "Texture") {
      texture = value;
    } else if (header == "Nitrogen") {
      nitro = value.toFloat();
    } else if (header == "Phosphorus") {
      phos = value.toFloat();
    } else if (header == "Potassium") {
      potas = value.toFloat();
    } else if (header == "EC") {
      ec = value.toFloat();
    } else if (header == "pH") {
      pH = value.toFloat();
    } else if (header == "moisture") {
      moisture = value.toFloat();
    } else if (header == "N") {
      nit_both = value.toInt();
    } else if (header == "P") {
      phos_both = value.toInt();
    } else if (header == "K") {
      potas_both = value.toInt();
    }
    // } else if (header == "1st application") {
    //   firstApplication = value;
    // } else if (header == "2nd application") {
    //   secondApplication = value;
    // } else if (header == "3rd application") {
    //   thirdApplication = value;
    // }
  }
}
void printAssignedValues(String prefix) {
  Serial.println("No: " + prefix);
  Serial.println("DATE: "+ String(__DATE__));
  Serial.println("Season: " + season);
  Serial.println("Variety: " + variety);
  Serial.println("Texture: " + texture);
  Serial.println("Nitrogen: " + String(nitro));
  Serial.println("Phosphorus: " + String(phos));
  Serial.println("Potassium: " + String(potas));
  Serial.println("EC: " + String(ec));
  Serial.println("pH: " + String(pH));
  Serial.println("Moisture: " + String(moisture));
  Serial.println("N: " + String(nit_both));
  Serial.println("P: " + String(phos_both));
  Serial.println("K: " + String(potas_both));
}
void retrieveCounterFromEEPROM() {
  // Retrieve the counter value from EEPROM
  EEPROM.get(0, fileCounter);

  // If the counter is 0 (first time), set it to 1
  if (fileCounter == 0) {
    fileCounter = 1;
    saveCounterToEEPROM(); // Save the initial value to EEPROM
  }
}

void saveCounterToEEPROM() {
  // Save the updated counter value to EEPROM
  EEPROM.put(0, fileCounter);
  // EEPROM.commit();  // Commit the changes to EEPROM
}

String createFileName() {
  String newFileName;
  newFileName = "ST" + String(fileCounter++) + ".csv";
  return newFileName;
}

void logData() {
  String fileName = createFileName();
  String prefix = extractPrefix(fileName); 
  Serial.println("Prefix: " + prefix);
  // Create a new file
  dataFile = SD.open(fileName, FILE_WRITE);
  
  // Check if the file opened successfully
  if (dataFile) {
    // Write data to the file
    dataFile.println("No,"+prefix);
    dataFile.println("DATE,"+ String(__DATE__));
    dataFile.println("Season," + String(season));
    dataFile.println("Variety," + String(variety));
    dataFile.println("Texture," + String(texture));

    dataFile.println("Nitrogen," + String(nitro));
    dataFile.println("Phosphorus," + String(phos));
    dataFile.println("Potassium," + String(potas));
    dataFile.println("EC," + String(ec));
    dataFile.println("pH," + String(pH));
    dataFile.println("moisture," + String(moisture));

    dataFile.println("NUTRIENT RECOMMENDATION kg/ha");

    dataFile.println("N," + String(nit_both));
    dataFile.println("P," + String(phos_both));
    dataFile.println("K," + String(potas_both));

    Serial.println("Data logged to: " + fileName);

    // Close the file
    dataFile.close();
    
    // Save the updated counter to EEPROM
    saveCounterToEEPROM();
  } else {
    Serial.println("Error opening file: " + fileName);
  }
}
void printWithSpace(Adafruit_Thermal &printer, const char *parameter, float value, const char *unit) {
  printer.print(parameter);
  int spaces = 15 - strlen(parameter); 
  for (int i = 0; i < spaces; i++) {
    printer.print(" ");
  }
  printer.print(value, 2);
  printer.print(unit);
  printer.println();
}

void printWithString(Adafruit_Thermal &printer, const char *parameter, const String &value) {
  printer.print(parameter);
  int spaces = 10 - strlen(parameter); 
  for (int i = 0; i < spaces; i++) {
    printer.print(" ");
  }
  printer.println(value.c_str());
}
void printCenteredText(Adafruit_Thermal &printer, const char *text) {
  int spaces = (32 - strlen(text)) / 2;  // Adjust the total width (32) based on your layout
  for (int i = 0; i < spaces; i++) {
    printer.print(" ");
  }
  printer.println(text);
}
void printWithInt(Adafruit_Thermal &printer, const char *parameter, int value) {
  printer.print(parameter);
  int spaces = 10 - strlen(parameter); // Adjust the number of spaces based on your layout
  for (int i = 0; i < spaces; i++) {
    printer.print(" ");
  }
  printer.print(value); // Print the integer value
  printer.println();
}

// NUTRIENT 
void nutrient_reco(float nit_both_val, float phos_both_val, float potas_both_val){
  int nb = static_cast<int>(nit_both);  
  int pb = static_cast<int>(phos_both);  
  int pob = static_cast<int>(potas_both);

  /*------Send Data to Display------*/
  Nit_Both_Dwin[6] = highByte(nb);
  Nit_Both_Dwin[7] = lowByte(nb);
  Serial2.write(Nit_Both_Dwin, 8);
  
  Phos_Both_Dwin[6] = highByte(pb);
  Phos_Both_Dwin[7] = lowByte(pb);
  Serial2.write(Phos_Both_Dwin, 8);

  Potas_Both_Dwin[6] = highByte(pob);
  Potas_Both_Dwin[7] = lowByte(pob);
  Serial2.write(Potas_Both_Dwin, 8);
}

void phosphorus_(float pH, float phos){

  // ABOVE 5.5
  if(pH > 5.5) {
    if (phos >= 0 && phos <= 6){
      phos_both = 60;
      phos_value = "LOW";
    }
    else if (phos >= 6.1 && phos <= 10){
      phos_both = 40;
      phos_value = "MODERATELY LOW";
    }
    else if (phos >= 10.1 && phos <= 15){
      phos_both = 20;
      phos_value = "MODERATELY HIGH";
    }
    else if (phos >= 15.1 && phos <= 100){
      phos_both = 7;
      phos_value = "HIGH";
    }
    else if (phos > 100){
      phos_both = 0;
      phos_value = "VERY HIGH";
    }
  }
  //  EQUAL OR BELOW 5.5
  if (pH <= 5.5) {
    if (phos >= 0 && phos <= 2) {
      phos_both = 60;
      phos_value = "LOW";
    }
    else if (phos >= 2.1 && phos <= 6){
      phos_both = 40;
      phos_value = "MODERATELY LOW";
    }
    else if (phos >= 6.1 && phos <= 10){
      phos_both = 20;
      phos_value = "MODERATELY HIGH";
    }
    else if (phos >= 10.1 && phos <= 75){
      phos_both= 7;
      phos_value = "HIGH"; 
    }
    else if (phos > 75) {
      phos_both = 0;
      phos_value = "VERY HIGH";
    }
  }

  Serial.print("Phosphorus: ");
  Serial.print(phos);
  Serial.print("ppm");
  Serial.println();
  Serial.print("Phosphorus Value: ");
  Serial.print(phos_value);
  Serial.println();
  if(pH > 5.5) {
    if (phos >= 0 && phos <= 6){
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19); 
    }
    else if (phos >= 6.1 && phos <= 10){
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19); 
    }
    else if (phos >= 10.1 && phos <= 15){
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19);
    }
    else if (phos >= 15.1 && phos <= 100){
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19); 
    }
    else if (phos > 100){
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x2D,0x2D,0x56,0x45,0x52,0x59,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19); 
    }
  }
  if (pH <= 5.5) {
    if (phos >= 0 && phos <= 2) {
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19); 
    }
    if (phos >= 2.1 && phos <= 6){
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19); 
    }
    if (phos >= 6.1 && phos <= 10){
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19);
    }
    if (phos >= 10.1 && phos <= 75){
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19);  
    }
    else if (phos > 75) {
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x2D,0x2D,0x56,0x45,0x52,0x59,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19); 
    }
  }


}
void potassium_(float potas){
    // potassium
  if (potas >= 0 && potas <= 0.190){
    potas_both = 60;
    potas_value = "LOW";
  }
  else if (potas >= 0.191 && potas <= 0.290){
    potas_both = 45;
    potas_value = "MODERATELY LOW";
  }
  else if (potas >= 0.291 && potas <= 0.385){
    potas_both = 30;
    potas_value = "MODERATELY HIGH";
  }
  else if (potas >= 0.386 && potas <= 1.000){
    potas_both = 7;
    potas_value = "HIGH";
  }
  else if (potas >= 1.000){
    potas_both = 0;
    potas_value = "VERY HIGH";
  }
  Serial.print("Potassium: ");
  Serial.print(potas);
  Serial.print("cmol/kg");
  Serial.println();
  Serial.print("Potassium Value: ");
  Serial.print(potas_value);
  Serial.println();

  if (potas >= 0 && potas <= 0.190){
    unsigned char Potassium_Label[] = {0x5A,0xA5,0x10,0x82,0x23,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Potassium_Label,19); 
  }
  else if (potas >= 0.191 && potas <= 0.290){
    unsigned char Potassium_Label[] = {0x5A,0xA5,0x10,0x82,0x23,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
    Serial2.write(Potassium_Label,19); 
  }
  else if (potas >= 0.291 && potas <= 0.385){
    unsigned char Potassium_Label[] = {0x5A,0xA5,0x10,0x82,0x23,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
    Serial2.write(Potassium_Label,19);
  }
  else if (potas >= 0.386 && potas <= 1.000){
    unsigned char Potassium_Label[] = {0x5A,0xA5,0x10,0x82,0x23,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Potassium_Label,19); 
  }
  else if (potas >= 1.000){
    unsigned char Potassium_Label[] = {0x5A,0xA5,0x10,0x82,0x23,0x00,0x2D,0x2D,0x56,0x45,0x52,0x59,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D};
    Serial2.write(Potassium_Label,19); 
  }
}
void soil_ph(float pH){

    // soil ph
  if (pH == 7){
    ph_value = "NEUTRAL";
  }
  else if (pH >= 0 && pH < 7){
    ph_value = "ACIDIC";
  }
  else if (pH >= 7 && pH <= 14){
    ph_value = "ALKALINE";
  }
  else if (pH > 14 && pH < 26){
    ph_value = "";
  }
  Serial.print("Soil pH: ");
  Serial.print(pH);
  Serial.println();
  Serial.print("Soil pH Value: ");
  Serial.print(ph_value);
  Serial.println();

  if (pH == 7){
    unsigned char Soil_ph[] = {0x5A,0xA5,0x10,0x82,0x15,0x00,0x2D,0x2D,0x2D,0x4E,0x45,0x55,0x54,0x52,0x41,0x4C,0x2D,0x2D,0x2D};
    Serial2.write(Soil_ph,19);
  }
  else if (pH >= 0 && pH < 7){
    unsigned char Soil_ph[] = {0x5A,0xA5,0x10,0x82,0x15,0x00,0x2D,0x2D,0x2D,0x41,0x43,0x49,0x44,0x49,0x43,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Soil_ph,19);
  }
  else if (pH >= 7 && pH <= 14){
    unsigned char Soil_ph[] = {0x5A,0xA5,0x10,0x82,0x15,0x00,0x2D,0x2D,0x41,0x4C,0x4B,0x41,0x4C,0x49,0x4E,0x45,0x2D,0x2D,0x2D};
    Serial2.write(Soil_ph,19);
  }
}
void electrical_conductivity(float ec){
  // ec
  if (ec >= 0 && ec <= 2) {
    soil_salinity_class = "Non Saline";
  }
  else if (ec > 2.1 && ec <= 4) {
    soil_salinity_class = "Slightly Saline";
  } 
  else if (ec > 4.1 && ec <= 8) {
    soil_salinity_class = "Moderately Saline";
  }
  else if (ec > 8.1 &&  ec < 16) {
    soil_salinity_class = "Severely Saline";
  }
  else if (ec > 16){
    soil_salinity_class = "Very Severely Saline";
  }
  Serial.print("EC: ");
  Serial.print(ec);
  Serial.print("ms/cm");
  Serial.println();
  Serial.print("Soil Salinity Class: ");
  Serial.print(soil_salinity_class);
  Serial.println();

  if (ec >= 0 && ec <= 2) {
    unsigned char Saline[] = {0x5A,0xA5,0x10,0x82,0x16,0x00,0x2D,0x2D,0x4E,0x4F,0x4E,0x53,0x41,0x4C,0x49,0x4E,0x45,0x2D,0x2D};
    Serial2.write(Saline,19);
  }
  else if (ec > 2.1 && ec <= 4) {
    unsigned char Saline[] = {0x5A,0xA5,0x10,0x82,0x16,0x00,0x2D,0x2D,0x53,0x4C,0x49,0x47,0x48,0x54,0x4C,0x59,0x2D,0x2D,0x2D};
    Serial2.write(Saline,19);
  } 
  else if (ec > 4.1 && ec <= 8) {
    unsigned char Saline[] = {0x5A,0xA5,0x10,0x82,0x16,0x00,0x2D,0x4D,0x4F,0x44,0x45,0x52,0x41,0x54,0x45,0x4C,0x59,0x2D,0x2D};
    Serial2.write(Saline,19);
  }
  else if (ec > 8.1 &&  ec < 16) {
    unsigned char Saline[] = {0x5A,0xA5,0x10,0x82,0x16,0x00,0x2D,0x2D,0x53,0x45,0x56,0x45,0x52,0x45,0x4C,0x59,0x2D,0x2D,0x2D};
    Serial2.write(Saline,19);
  }
  else if (ec > 16){
    unsigned char Saline[] = {0x5A,0xA5,0x10,0x82,0x16,0x00,0x56,0x45,0x52,0x59,0x53,0x45,0x56,0x45,0x52,0x45,0x4C,0x59,0x2D};
    Serial2.write(Saline,19);
  }

}
void moisture_(float moisture){
    // Moisture
  if (moisture <= 15){
    mois_value = "Very Low Moisture"; 
  }
  else if (moisture > 15){
    mois_value = "Moist Soil";
  }
  Serial.print("Moisture: ");
  Serial.print(moisture);
  Serial.print("%");
  Serial.println();
  Serial.print("Moisture Value: ");
  Serial.print(mois_value);
  Serial.println();
  if (moisture <= 15){
    unsigned char Moisture[] = {0x5A,0xA5,0x10,0x82,0x17,0x00,0x2D,0x2D,0x2D,0x56,0x45,0x52,0x59,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
    Serial2.write(Moisture,19);
  }
  else if (moisture > 15){
    unsigned char Moisture[] = {0x5A,0xA5,0x10,0x82,0x17,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x49,0x53,0x54,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Moisture,19);
  }
}


// light nitro -> WET SEASON
void hybrid_nitrogen_lws(float nitro){

  if (nitro <= 2.0) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 100;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 80;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 60;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 7;
    nit_value = "HIGH";
  }
  Serial.println();
  Serial.print("Nitrogen: ");
  Serial.print(nitro);
  Serial.print("%");
  Serial.println();
  Serial.print("Nitrogen Value: ");
  Serial.print(nit_value);
  Serial.println();
}
// medium nitro -> WET SEASON
void hybrid_nitrogen_mws(float nitro){

  if (nitro <= 2.0) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 90;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 70;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 50;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 7;
    nit_value = "HIGH";
  }
  Serial.println();
  Serial.print("Nitrogen: ");
  Serial.print(nitro);
  Serial.print("%");
  Serial.println();
  Serial.print("Nitrogen Value: ");
  Serial.print(nit_value);
  Serial.println();
}
// heavy nitro -> WET SEASON
void hybrid_nitrogen_hws(float nitro){

  if (nitro <= 2.0) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 80;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 60;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 40;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 7;
    nit_value = "HIGH";
  }
  Serial.println();
  Serial.print("Nitrogen: ");
  Serial.print(nitro);
  Serial.print("%");
  Serial.println();
  Serial.print("Nitrogen Value: ");
  Serial.print(nit_value);
  Serial.println();
}
// DRY SEASON
void hybrid_nitrogen_lds(float nitro){

  if (nitro <= 2.0) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 120;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 100;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 80;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 7;
    nit_value = "HIGH";
  }
  Serial.println();
  Serial.print("Nitrogen: ");
  Serial.print(nitro);
  Serial.print("%");
  Serial.println();
  Serial.print("Nitrogen Value: ");
  Serial.print(nit_value);
  Serial.println();
}

// medium nitro -> DRY SEASON
void hybrid_nitrogen_mds(float nitro){

  if (nitro <= 2.0) {
    nit_both = 110;
    nit_value = "LOW";
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19);  
  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 90;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 70;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 7;
    nit_value = "HIGH";
  }
  Serial.println();
  Serial.print("Nitrogen: ");
  Serial.print(nitro);
  Serial.print("%");
  Serial.println();
  Serial.print("Nitrogen Value: ");
  Serial.print(nit_value);
  Serial.println();
}
// heavy nitro -> DRY SEASON
void hybrid_nitrogen_hds(float nitro){

  if (nitro <= 2.0) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 100;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 80;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 60;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 7;
    nit_value = "HIGH";
  }
  Serial.println();
  Serial.print("Nitrogen: ");
  Serial.print(nitro);
  Serial.print("%");
  Serial.println();
  Serial.print("Nitrogen Value: ");
  Serial.print(nit_value);
  Serial.println();
}

// INBRED
// WET SEASON
void inbred_nitrogen_lws(float nitro){

  if (nitro <= 2.0) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 80;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 60;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 40;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 7;
    nit_value = "HIGH";
  }
  Serial.println();
  Serial.print("Nitrogen: ");
  Serial.print(nitro);
  Serial.print("%");
  Serial.println();
  Serial.print("Nitrogen Value: ");
  Serial.print(nit_value);
  Serial.println();
}
// medium nitro -> WET SEASON
void inbred_nitrogen_mws(float nitro){

  if (nitro <= 2.0) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 70;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 50;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 30;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 7;
    nit_value = "HIGH";
  }
  Serial.println();
  Serial.print("Nitrogen: ");
  Serial.print(nitro);
  Serial.print("%");
  Serial.println();
  Serial.print("Nitrogen Value: ");
  Serial.print(nit_value);
  Serial.println();
}
// heavy nitro -> WET SEASON
void inbred_nitrogen_hws(float nitro){

  if (nitro <= 2.0) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 60;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 40;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 20;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 7;
    nit_value = "HIGH";
  }
  Serial.println();
  Serial.print("Nitrogen: ");
  Serial.print(nitro);
  Serial.print("%");
  Serial.println();
  Serial.print("Nitrogen Value: ");
  Serial.print(nit_value);
  Serial.println();
}

// INBRED
// DRY SEASON
void inbred_nitrogen_lds(float nitro){

  if (nitro <= 2.0) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 90;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 70;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19);
    nit_both = 50;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 7;
    nit_value = "HIGH";
  }
  Serial.println();
  Serial.print("Nitrogen: ");
  Serial.print(nitro);
  Serial.print("%");
  Serial.println();
  Serial.print("Nitrogen Value: ");
  Serial.print(nit_value);
  Serial.println();
}
// medium nitro -> DRY SEASON
void inbred_nitrogen_mds(float nitro){

  if (nitro <= 2.0) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 80;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 60;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 40;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 7;
    nit_value = "HIGH";
  }
  Serial.println();
  Serial.print("Nitrogen: ");
  Serial.print(nitro);
  Serial.print("%");
  Serial.println();
  Serial.print("Nitrogen Value: ");
  Serial.print(nit_value);
  Serial.println();
}
// heavy nitro -> DRY SEASON
void inbred_nitrogen_hds(float nitro){

  if (nitro <= 2.0) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 70;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x4C,0x4F,0x57,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 50;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x4D,0x4F,0x44,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 30;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x2D,0x2D,0x2D,0x2D,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 7;
    nit_value = "HIGH";
  }
  Serial.println();
  Serial.print("Nitrogen: ");
  Serial.print(nitro);
  Serial.print("%");
  Serial.println();
  Serial.print("Nitrogen Value: ");
  Serial.print(nit_value);
  Serial.println();
}

// Splitting Nutrient Recommendation
void splitting(int nit_both,int phos_both,int potas_both){
  int nitro_split1,phos_split1,potas_split1;
  int nitro_split2,phos_split2,potas_split2;
  int nitro_split3,phos_split3,potas_split3;
  // Serial.println(nit_both);
  // Serial.println(phos_both);
  // Serial.println(potas_both);
    //Splitting
    // 1st, 2nd, 3rd -> Application

  // first application
  if (button_selector_season == 0 || button_selector_season == 1) {
    // do this
    nitro_split1 = nit_both * 0.30;
    phos_split1 = phos_both;

    if (potas_both < 45) {
      potas_split1 = potas_both;
    }
    else{
      potas_split1 = potas_both * 0.50;
    }
    // Serial.println(nitro_split1);
    // Serial.println(phos_split1);
    // Serial.println(potas_split1);
  }

  // second application
  if (button_selector_season == 1) {
    // do this
    nitro_split2 = nit_both * 0.30;
    phos_split2 = 0;
    potas_split2 = 0;
  }
  else if (button_selector_season == 0) {
    // do this
    nitro_split2 = nit_both * 0.20;
    phos_split2 = 0;
    potas_split2 = 0;
    // Serial.println(nitro_split2);
    // Serial.println(phos_split2);
    // Serial.println(potas_split2);
  }
  // third application
  if (button_selector_season == 1) {
    // do this
    nitro_split3 = nit_both * 0.40;
    phos_split3 = 0;
    if (potas_split1 < 45) {
      potas_split3 = potas_split1 * 0.50;
    }
    else{
      potas_split3 = potas_split1;
    }
    // Serial.println(nitro_split3);
    // Serial.println(phos_split3);
    // Serial.println(potas_split3);
  }
  else if (button_selector_season == 0) {
    // do this
    nitro_split3 = nit_both * 0.50;
    phos_split3 = 0;
    if (potas_split1 < 45) {
      potas_split3 = potas_both * 0.50;
    }
    else {
      potas_split3 = potas_both;
    }
    // Serial.println(nitro_split3);
    // Serial.println(phos_split3);
    // Serial.println(potas_split3);
  }
  // // display 
  // // ------------


  // // Filterizer Contains
  // // 1st Application
  String value_fil;
  float n_fil,p_fil,k_fil;
  if (nitro_split1 > 1 && phos_split1 > 1 && potas_split1 > 1){
      value_fil = "Complete, Triple 14";
      unsigned char N_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x72, 0x00, 0x00, 0x00};
      unsigned char P_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x73, 0x00, 0x00, 0x00};
      unsigned char K_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x74, 0x00, 0x00, 0x00};
      n_fil = 7;
      p_fil = 7;
      k_fil = 7;

      int n_fil_val = static_cast<int>(n_fil);  
      int p_fil_val = static_cast<int>(p_fil);  
      int k_fil_val = static_cast<int>(k_fil);  
      /*------Send Data to Display------*/
      N_fil[6] = highByte(n_fil_val);
      N_fil[7] = lowByte(n_fil_val);
      Serial2.write(N_fil, 8);

      P_fil[6] = highByte(p_fil_val);
      P_fil[7] = lowByte(p_fil_val);
      Serial2.write(P_fil, 8);

      K_fil[6] = highByte(k_fil_val);
      K_fil[7] = lowByte(k_fil_val);
      Serial2.write(K_fil, 8);
  }

  else if (nitro_split1 > 1 && phos_split1 > 1 && potas_split1 < 1){
      value_fil = "Ammonium Phosphate";
      unsigned char N_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x72, 0x00, 0x00, 0x00};
      unsigned char P_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x73, 0x00, 0x00, 0x00};
      unsigned char K_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x74, 0x00, 0x00, 0x00};
      n_fil = 8;
      p_fil = 10;
      k_fil = 0;
      int n_fil_val = static_cast<int>(n_fil);  
      int p_fil_val = static_cast<int>(p_fil);  
      int k_fil_val = static_cast<int>(k_fil);  
      /*------Send Data to Display------*/
      N_fil[6] = highByte(n_fil_val);
      N_fil[7] = lowByte(n_fil_val);
      Serial2.write(N_fil, 8);

      P_fil[6] = highByte(p_fil_val);
      P_fil[7] = lowByte(p_fil_val);
      Serial2.write(P_fil, 8);

      K_fil[6] = highByte(k_fil_val);
      K_fil[7] = lowByte(k_fil_val);
      Serial2.write(K_fil, 8);
  }

  else if (nitro_split1 < 1 && phos_split1 > 1 && potas_split1 < 1){
      value_fil = "Superphospate";
      unsigned char N_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x72, 0x00, 0x00, 0x00};
      unsigned char P_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x73, 0x00, 0x00, 0x00};
      unsigned char K_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x74, 0x00, 0x00, 0x00};
      n_fil = 0;
      p_fil = 10;
      k_fil = 0;
      int n_fil_val = static_cast<int>(n_fil);  
      int p_fil_val = static_cast<int>(p_fil);  
      int k_fil_val = static_cast<int>(k_fil);  
      /*------Send Data to Display------*/
      N_fil[6] = highByte(n_fil_val);
      N_fil[7] = lowByte(n_fil_val);
      Serial2.write(N_fil, 8);

      P_fil[6] = highByte(p_fil_val);
      P_fil[7] = lowByte(p_fil_val);
      Serial2.write(P_fil, 8);

      K_fil[6] = highByte(k_fil_val);
      K_fil[7] = lowByte(k_fil_val);
      Serial2.write(K_fil, 8);
  }

  else if (nitro_split1 < 1 && phos_split1 < 1 && potas_split1 > 1 || nitro_split1 < 1 && phos_split1 > 1 && potas_split1 > 1){
      value_fil = "Muriate of Potash";
      unsigned char N_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x72, 0x00, 0x00, 0x00};
      unsigned char P_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x73, 0x00, 0x00, 0x00};
      unsigned char K_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x74, 0x00, 0x00, 0x00};
      n_fil = 0;
      p_fil = 0;
      k_fil = 30;
      int n_fil_val = static_cast<int>(n_fil);  
      int p_fil_val = static_cast<int>(p_fil);  
      int k_fil_val = static_cast<int>(k_fil);  
      /*------Send Data to Display------*/
      N_fil[6] = highByte(n_fil_val);
      N_fil[7] = lowByte(n_fil_val);
      Serial2.write(N_fil, 8);

      P_fil[6] = highByte(p_fil_val);
      P_fil[7] = lowByte(p_fil_val);
      Serial2.write(P_fil, 8);

      K_fil[6] = highByte(k_fil_val);
      K_fil[7] = lowByte(k_fil_val);
      Serial2.write(K_fil, 8);
  }

      
      
  if (pH < 6.6) {
      if (nitro_split1 > 1 && phos_split1 < 1 && potas_split1 < 1 || nitro_split1 > 1 && phos_split1 < 1 && potas_split1 > 1){
          value_fil = "Urea";
          unsigned char N_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x72, 0x00, 0x00, 0x00};
          unsigned char P_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x73, 0x00, 0x00, 0x00};
          unsigned char K_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x74, 0x00, 0x00, 0x00};
          n_fil = 23;
          p_fil = 0;
          k_fil = 0;
          int n_fil_val = static_cast<int>(n_fil);  
          int p_fil_val = static_cast<int>(p_fil);  
          int k_fil_val = static_cast<int>(k_fil);  
          /*------Send Data to Display------*/
          N_fil[6] = highByte(n_fil_val);
          N_fil[7] = lowByte(n_fil_val);
          Serial2.write(N_fil, 8);

          P_fil[6] = highByte(p_fil_val);
          P_fil[7] = lowByte(p_fil_val);
          Serial2.write(P_fil, 8);

          K_fil[6] = highByte(k_fil_val);
          K_fil[7] = lowByte(k_fil_val);
          Serial2.write(K_fil, 8);
      }

  }

  else {
      if (nitro_split1 > 1 && phos_split1 < 1 && potas_split1 < 1 || nitro_split1 > 1 && phos_split1 < 1 && potas_split1 > 1){
          value_fil = "Ammonium Sulfate";
          unsigned char N_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x72, 0x00, 0x00, 0x00};
          unsigned char P_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x73, 0x00, 0x00, 0x00};
          unsigned char K_fil[8] = {0x5A, 0xA5, 0x05, 0x82, 0x74, 0x00, 0x00, 0x00};
          n_fil = 10.5;
          p_fil = 0;
          k_fil = 0;
          int n_fil_val = static_cast<int>(n_fil);  
          int p_fil_val = static_cast<int>(p_fil);  
          int k_fil_val = static_cast<int>(k_fil);  
          /*------Send Data to Display------*/
          N_fil[6] = highByte(n_fil_val);
          N_fil[7] = lowByte(n_fil_val);
          Serial2.write(N_fil, 8);

          P_fil[6] = highByte(p_fil_val);
          P_fil[7] = lowByte(p_fil_val);
          Serial2.write(P_fil, 8);

          K_fil[6] = highByte(k_fil_val);
          K_fil[7] = lowByte(k_fil_val);
          Serial2.write(K_fil, 8);
      }

  }
  // divide 
  // float result_dividen1 = nitro_split1 / n_fil;
  // float result_dividep1 = phos_split1 / p_fil;
  // float result_dividek1 = potas_split1 / k_fil;

  float result_dividen1 = (n_fil != 0.0) ? (nitro_split1 / n_fil) : 0.0;;
  float result_dividep1 = (p_fil != 0.0) ? (phos_split1 / p_fil) : 0.0;
  float result_dividek1 = (k_fil != 0.0) ? (potas_split1 / k_fil) : 0.0;

  // Serial.println(result_dividen1);
  // Serial.println(result_dividep1);
  // Serial.println(result_dividek1);
  float lowest_value1 = findLowestNonZero(result_dividen1, result_dividep1, result_dividek1);
  int low_first = static_cast<int>(lowest_value1);
  float decimal_part_first = lowest_value1 - static_cast<float>(low_first);
  int decimal_as_int1 = static_cast<int>((decimal_part_first * 100.0) + 0.5);
  // Serial.println(lowest_value1);
  // print value_fil, lowest_value1
  // Serial.println(n_fil);
  // Serial.println(p_fil);
  // Serial.println(k_fil);
  // unsigned char Bags[8] = {0x5A, 0xA5, 0x05, 0x82, 0x70, 0x00, 0x00, 0x00};
  // int bags_val = static_cast<int>(lowest_value1);  
  // Bags[6] = highByte(bags_val);
  // Bags[7] = lowByte(bags_val);
  // Serial2.write(Bags, 8);
  
  // int get = int(lowest_value1 * 100) % 100;
  // float dividedBy2 = float(get) / 2;
  // int roundedValue = int((dividedBy2 + 2.5) / 5) * 5;
  // // Serial.println(roundedValue);
  // unsigned char Kilograms[8] = {0x5A, 0xA5, 0x05, 0x82, 0x71, 0x00, 0x00, 0x00};
  // int kg_val = static_cast<int>(roundedValue);  
  // Kilograms[6] = highByte(kg_val);
  // Kilograms[7] = lowByte(kg_val);
  // Serial2.write(Kilograms, 8);


  // multiply
  float result_multipn1 = lowest_value1 * n_fil;
  float result_multipp1 = lowest_value1 * p_fil;
  float result_multipk1 = lowest_value1 * k_fil;
  // Serial.println(result_multipn1);
  // Serial.println(result_multipp1);
  // Serial.println(result_multipk1);
  // minus the result
  float result_minusn1 = nitro_split1 - result_multipn1;
  float result_minusp1 = phos_split1 - result_multipp1;
  float result_minusk1 = potas_split1 - result_multipk1;
  // Serial.println(result_minusn1);
  // Serial.println(result_minusp1);
  // Serial.println(result_minusk1);

  float divide2_decimal_first = static_cast<float>(decimal_as_int1) / 2.0;
  int rounded_value1 = static_cast<int>(round(divide2_decimal_first));
  int get_number1 = int(lowest_value1);
  // Serial.println(get_number1);
  // Serial.println(rounded_value1);
  // bags
  unsigned char GetNumber1[8] = {0x5A, 0xA5, 0x05, 0x82, 0x70, 0x00, 0x00, 0x00};
  int getnumber1 = static_cast<int>(get_number1);   
  GetNumber1[6] = highByte(getnumber1);
  GetNumber1[7] = lowByte(getnumber1);
  Serial2.write(GetNumber1, 8);
  // kg
  unsigned char DivideDecimal1[8] = {0x5A, 0xA5, 0x05, 0x82, 0x71, 0x00, 0x00, 0x00};
  int dividedecimal1 = static_cast<int>(rounded_value1);   
  DivideDecimal1[6] = highByte(dividedecimal1);
  DivideDecimal1[7] = lowByte(dividedecimal1);
  Serial2.write(DivideDecimal1, 8);

  float n_fil_second;
  float p_fil_second;
  float k_fil_second;
  if (result_minusn1 != 0 && result_minusp1 != 0 && result_minusk1 != 0){
    Serial.println("All Values are Zero!");
  } else {
    if (result_minusn1 > 1 && result_minusp1 > 1 && result_minusk1 > 1){
      unsigned char N_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x77, 0x00, 0x00, 0x00};
      unsigned char P_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x78, 0x00, 0x00, 0x00};
      unsigned char K_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x79, 0x00, 0x00, 0x00};
      value_fil = "Complete, Triple 14";
      n_fil_second = 7;
      p_fil_second = 7;
      k_fil_second = 7;
      int n_fil_val_second = static_cast<int>(n_fil_second);  
      int p_fil_val_second = static_cast<int>(p_fil_second);  
      int k_fil_val_second = static_cast<int>(k_fil_second);  
      /*------Send Data to Display------*/
      N_filSecond[6] = highByte(n_fil_val_second);
      N_filSecond[7] = lowByte(n_fil_val_second);
      Serial2.write(N_filSecond, 8);

      P_filSecond[6] = highByte(p_fil_val_second);
      P_filSecond[7] = lowByte(p_fil_val_second);
      Serial2.write(P_filSecond, 8);

      K_filSecond[6] = highByte(k_fil_val_second);
      K_filSecond[7] = lowByte(k_fil_val_second);
      Serial2.write(K_filSecond, 8);
    }

    else if (result_minusn1 > 1 && result_minusp1 > 1 && result_minusk1 < 1){
      unsigned char N_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x77, 0x00, 0x00, 0x00};
      unsigned char P_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x78, 0x00, 0x00, 0x00};
      unsigned char K_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x79, 0x00, 0x00, 0x00};
      value_fil = "Ammonium Phosphate";
      n_fil_second = 8;
      p_fil_second = 10;
      k_fil_second = 0;
      int n_fil_val_second = static_cast<int>(n_fil_second);  
      int p_fil_val_second = static_cast<int>(p_fil_second);  
      int k_fil_val_second = static_cast<int>(k_fil_second);  
      /*------Send Data to Display------*/
      N_filSecond[6] = highByte(n_fil_val_second);
      N_filSecond[7] = lowByte(n_fil_val_second);
      Serial2.write(N_filSecond, 8);

      P_filSecond[6] = highByte(p_fil_val_second);
      P_filSecond[7] = lowByte(p_fil_val_second);
      Serial2.write(P_filSecond, 8);

      K_filSecond[6] = highByte(k_fil_val_second);
      K_filSecond[7] = lowByte(k_fil_val_second);
      Serial2.write(K_filSecond, 8);
    }

    else if (result_minusn1 < 1 && result_minusp1 > 1 && result_minusk1 < 1){
      unsigned char N_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x77, 0x00, 0x00, 0x00};
      unsigned char P_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x78, 0x00, 0x00, 0x00};
      unsigned char K_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x79, 0x00, 0x00, 0x00};
      value_fil = "Superphospate";
      n_fil_second = 0;
      p_fil_second = 10;
      k_fil_second = 0;
      int n_fil_val_second = static_cast<int>(n_fil_second);  
      int p_fil_val_second = static_cast<int>(p_fil_second);  
      int k_fil_val_second = static_cast<int>(k_fil_second);  
      /*------Send Data to Display------*/
      N_filSecond[6] = highByte(n_fil_val_second);
      N_filSecond[7] = lowByte(n_fil_val_second);
      Serial2.write(N_filSecond, 8);

      P_filSecond[6] = highByte(p_fil_val_second);
      P_filSecond[7] = lowByte(p_fil_val_second);
      Serial2.write(P_filSecond, 8);

      K_filSecond[6] = highByte(k_fil_val_second);
      K_filSecond[7] = lowByte(k_fil_val_second);
      Serial2.write(K_filSecond, 8);
    }

    else if (result_minusn1 < 1 && result_minusp1 < 1 && result_minusk1 > 1 || result_minusn1 < 1 && result_minusp1 > 1 && result_minusk1 > 1){
      unsigned char N_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x77, 0x00, 0x00, 0x00};
      unsigned char P_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x78, 0x00, 0x00, 0x00};
      unsigned char K_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x79, 0x00, 0x00, 0x00};
      value_fil = "Muriate of Potash";
      n_fil_second = 0;
      p_fil_second = 0;
      k_fil_second = 30;
      int n_fil_val_second = static_cast<int>(n_fil_second);  
      int p_fil_val_second = static_cast<int>(p_fil_second);  
      int k_fil_val_second = static_cast<int>(k_fil_second);  
      /*------Send Data to Display------*/
      N_filSecond[6] = highByte(n_fil_val_second);
      N_filSecond[7] = lowByte(n_fil_val_second);
      Serial2.write(N_filSecond, 8);

      P_filSecond[6] = highByte(p_fil_val_second);
      P_filSecond[7] = lowByte(p_fil_val_second);
      Serial2.write(P_filSecond, 8);

      K_filSecond[6] = highByte(k_fil_val_second);
      K_filSecond[7] = lowByte(k_fil_val_second);
      Serial2.write(K_filSecond, 8);
    }    
    if (pH < 6.6) {
          if (result_minusn1 > 1 && result_minusp1 < 1 && result_minusk1 < 1 || result_minusn1 > 1 && result_minusp1 < 1 && result_minusk1 > 1){
            unsigned char N_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x77, 0x00, 0x00, 0x00};
            unsigned char P_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x78, 0x00, 0x00, 0x00};
            unsigned char K_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x79, 0x00, 0x00, 0x00};
            value_fil = "Urea";
            n_fil_second = 23;
            p_fil_second = 0;
            k_fil_second = 0;
            int n_fil_val_second = static_cast<int>(n_fil_second);  
            int p_fil_val_second = static_cast<int>(p_fil_second);  
            int k_fil_val_second = static_cast<int>(k_fil_second);  
            /*------Send Data to Display------*/
            N_filSecond[6] = highByte(n_fil_val_second);
            N_filSecond[7] = lowByte(n_fil_val_second);
            Serial2.write(N_filSecond, 8);

            P_filSecond[6] = highByte(p_fil_val_second);
            P_filSecond[7] = lowByte(p_fil_val_second);
            Serial2.write(P_filSecond, 8);

            K_filSecond[6] = highByte(k_fil_val_second);
            K_filSecond[7] = lowByte(k_fil_val_second);
            Serial2.write(K_filSecond, 8);
          }
    }
    else {
        if (result_minusn1 > 1 && result_minusp1 < 1 && result_minusk1 < 1 || result_minusn1 > 1 && result_minusp1 < 1 && result_minusk1 > 1){
          unsigned char N_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x77, 0x00, 0x00, 0x00};
          unsigned char P_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x78, 0x00, 0x00, 0x00};
          unsigned char K_filSecond[8] = {0x5A, 0xA5, 0x05, 0x82, 0x79, 0x00, 0x00, 0x00};
          value_fil = "Ammonium Sulfate";
          n_fil_second = 10.5;
          p_fil_second = 0;
          k_fil_second = 0;
          int n_fil_val_second = static_cast<int>(n_fil_second);  
          int p_fil_val_second = static_cast<int>(p_fil_second);  
          int k_fil_val_second = static_cast<int>(k_fil_second);  
          /*------Send Data to Display------*/
          N_filSecond[6] = highByte(n_fil_val_second);
          N_filSecond[7] = lowByte(n_fil_val_second);
          Serial2.write(N_filSecond, 8);

          P_filSecond[6] = highByte(p_fil_val_second);
          P_filSecond[7] = lowByte(p_fil_val_second);
          Serial2.write(P_filSecond, 8);

          K_filSecond[6] = highByte(k_fil_val_second);
          K_filSecond[7] = lowByte(k_fil_val_second);
          Serial2.write(K_filSecond, 8);
        }

    }
    // Serial.println(n_fil_second);
    // Serial.println(p_fil_second);
    // Serial.println(k_fil_second);
    // Serial.println(result_minusn1);
    // Serial.println(result_minusp1);
    // Serial.println(result_minusk1);
    float result_divide_secondn1 = (n_fil_second != 0.0) ? (result_minusn1 / n_fil_second) : 0.0;;
    float result_divide_secondp1 = (p_fil_second != 0.0) ? (result_minusp1 / p_fil_second) : 0.0;
    float result_divide_secondk1 = (k_fil_second != 0.0) ? (result_minusk1 / k_fil_second) : 0.0;

    // Serial.println(result_divide_secondn1);
    // Serial.println(result_divide_secondp1);
    // Serial.println(result_divide_secondk1);
    float lowest_value_second = findLowestNonZero(result_divide_secondn1, result_divide_secondp1, result_divide_secondk1);
    int low_second = static_cast<int>(lowest_value_second);
    float decimal_part_second = lowest_value_second - static_cast<float>(low_second);
    int decimal_as_int2 = static_cast<int>((decimal_part_second * 100.0) + 0.5);
    // Serial.println(lowest_value_second);
    float result_multip_secondn1 = lowest_value_second * n_fil_second;
    float result_multip_secondp1 = lowest_value_second * p_fil_second;
    float result_multip_secondk1 = lowest_value_second * k_fil_second;
    // Serial.println(result_multip_secondn1);
    // Serial.println(result_multip_secondp1);
    // Serial.println(result_multip_secondk1);

    float result_minus_secondn1 = result_minusn1 - result_multip_secondn1;
    float result_minus_secondp1 = result_minusp1 - result_multip_secondp1;
    float result_minus_secondk1 = result_minusk1 - result_multip_secondk1;
    // Serial.println(result_minus_secondn1);
    // Serial.println(result_minus_secondp1);
    // Serial.println(result_minus_secondk1);

    float divide2_decimal_second = static_cast<float>(decimal_as_int2) / 2.0;
    int rounded_value2 = static_cast<int>(round(divide2_decimal_second));
    int get_number_second = int(lowest_value_second);
    // Serial.println(get_number_second);
    // Serial.println(rounded_value2);
    // Serial.println(get_number_second);
    // Serial.println(divide2_decimal_second);
    // bags
    unsigned char GetNumber2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x75, 0x00, 0x00, 0x00};
    int getnumber2 = static_cast<int>(get_number_second);   
    GetNumber2[6] = highByte(getnumber2);
    GetNumber2[7] = lowByte(getnumber2);
    Serial2.write(GetNumber2, 8);
    // kg
    unsigned char DivideDecimal2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x76, 0x00, 0x00, 0x00};
    int dividedecimal2 = static_cast<int>(rounded_value2);   
    DivideDecimal2[6] = highByte(dividedecimal2);
    DivideDecimal2[7] = lowByte(dividedecimal2);
    Serial2.write(DivideDecimal2, 8);

    float n_fil_third;
    float p_fil_third;
    float k_fil_third;
    if (result_minus_secondn1 != 0 && result_minus_secondp1 != 0 && result_minus_secondk1 != 0){
      Serial.println("All Values are Zero!");
    } else {

      if (result_minus_secondn1 > 1 && result_minus_secondp1 > 1 && result_minus_secondk1 > 1){
        unsigned char N_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x82, 0x00, 0x00, 0x00};
        unsigned char P_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x83, 0x00, 0x00, 0x00};
        unsigned char K_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x84, 0x00, 0x00, 0x00};
        value_fil = "Complete, Triple 14";
        n_fil_third = 7;
        p_fil_third = 7;
        k_fil_third = 7;
        int n_fil_val_third = static_cast<int>(n_fil_third);  
        int p_fil_val_third = static_cast<int>(p_fil_third);  
        int k_fil_val_third = static_cast<int>(k_fil_third);  
        /*------Send Data to Display------*/
        N_filThird[6] = highByte(n_fil_val_third);
        N_filThird[7] = lowByte(n_fil_val_third);
        Serial2.write(N_filThird, 8);

        P_filThird[6] = highByte(p_fil_val_third);
        P_filThird[7] = lowByte(p_fil_val_third);
        Serial2.write(P_filThird, 8);

        K_filThird[6] = highByte(k_fil_val_third);
        K_filThird[7] = lowByte(k_fil_val_third);
        Serial2.write(K_filThird, 8);
      }

      else if (result_minus_secondn1 > 1 && result_minus_secondp1 > 1 && result_minus_secondk1 < 1){
        unsigned char N_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x82, 0x00, 0x00, 0x00};
        unsigned char P_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x83, 0x00, 0x00, 0x00};
        unsigned char K_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x84, 0x00, 0x00, 0x00};
        value_fil = "Ammonium Phosphate";
        n_fil_third = 8;
        p_fil_third = 10;
        k_fil_third = 0;
        int n_fil_val_third = static_cast<int>(n_fil_third);  
        int p_fil_val_third = static_cast<int>(p_fil_third);  
        int k_fil_val_third = static_cast<int>(k_fil_third);  
        /*------Send Data to Display------*/
        N_filThird[6] = highByte(n_fil_val_third);
        N_filThird[7] = lowByte(n_fil_val_third);
        Serial2.write(N_filThird, 8);

        P_filThird[6] = highByte(p_fil_val_third);
        P_filThird[7] = lowByte(p_fil_val_third);
        Serial2.write(P_filThird, 8);

        K_filThird[6] = highByte(k_fil_val_third);
        K_filThird[7] = lowByte(k_fil_val_third);
        Serial2.write(K_filThird, 8);
      }

      else if (result_minus_secondn1 < 1 && result_minus_secondp1 > 1 && result_minus_secondk1 < 1){
        unsigned char N_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x82, 0x00, 0x00, 0x00};
        unsigned char P_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x83, 0x00, 0x00, 0x00};
        unsigned char K_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x84, 0x00, 0x00, 0x00};
        value_fil = "Superphospate";
        n_fil_third = 0;
        p_fil_third = 10;
        k_fil_third = 0;
        int n_fil_val_third = static_cast<int>(n_fil_third);  
        int p_fil_val_third = static_cast<int>(p_fil_third);  
        int k_fil_val_third = static_cast<int>(k_fil_third);  
        /*------Send Data to Display------*/
        N_filThird[6] = highByte(n_fil_val_third);
        N_filThird[7] = lowByte(n_fil_val_third);
        Serial2.write(N_filThird, 8);

        P_filThird[6] = highByte(p_fil_val_third);
        P_filThird[7] = lowByte(p_fil_val_third);
        Serial2.write(P_filThird, 8);

        K_filThird[6] = highByte(k_fil_val_third);
        K_filThird[7] = lowByte(k_fil_val_third);
        Serial2.write(K_filThird, 8);
      }

      else if (result_minus_secondn1 < 1 && result_minus_secondp1 < 1 && result_minus_secondk1 > 1 || result_minus_secondn1 < 1 && result_minus_secondp1 > 1 && result_minus_secondk1 > 1){
        value_fil = "Muriate of Potash";
        unsigned char N_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x82, 0x00, 0x00, 0x00};
        unsigned char P_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x83, 0x00, 0x00, 0x00};
        unsigned char K_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x84, 0x00, 0x00, 0x00};
        n_fil_third = 0;
        p_fil_third = 0;
        k_fil_third = 30;
        int n_fil_val_third = static_cast<int>(n_fil_third);  
        int p_fil_val_third = static_cast<int>(p_fil_third);  
        int k_fil_val_third = static_cast<int>(k_fil_third);  
        /*------Send Data to Display------*/
        N_filThird[6] = highByte(n_fil_val_third);
        N_filThird[7] = lowByte(n_fil_val_third);
        Serial2.write(N_filThird, 8);

        P_filThird[6] = highByte(p_fil_val_third);
        P_filThird[7] = lowByte(p_fil_val_third);
        Serial2.write(P_filThird, 8);

        K_filThird[6] = highByte(k_fil_val_third);
        K_filThird[7] = lowByte(k_fil_val_third);
        Serial2.write(K_filThird, 8);
      }    

      if (pH < 6.6) {
            if (result_minus_secondn1 > 1 && result_minus_secondp1 < 1 && result_minus_secondk1 < 1 || result_minus_secondn1 > 1 && result_minus_secondp1 < 1 && result_minus_secondk1 > 1){
              unsigned char N_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x82, 0x00, 0x00, 0x00};
              unsigned char P_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x83, 0x00, 0x00, 0x00};
              unsigned char K_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x84, 0x00, 0x00, 0x00};
              value_fil = "Urea";
              n_fil_third = 23;
              p_fil_third = 0;
              k_fil_third = 0;
              int n_fil_val_third = static_cast<int>(n_fil_third);  
              int p_fil_val_third = static_cast<int>(p_fil_third);  
              int k_fil_val_third = static_cast<int>(k_fil_third);  
              /*------Send Data to Display------*/
              N_filThird[6] = highByte(n_fil_val_third);
              N_filThird[7] = lowByte(n_fil_val_third);
              Serial2.write(N_filThird, 8);

              P_filThird[6] = highByte(p_fil_val_third);
              P_filThird[7] = lowByte(p_fil_val_third);
              Serial2.write(P_filThird, 8);

              K_filThird[6] = highByte(k_fil_val_third);
              K_filThird[7] = lowByte(k_fil_val_third);
              Serial2.write(K_filThird, 8);
            }
      }
      else {
          if (result_minus_secondn1 > 1 && result_minus_secondp1 < 1 && result_minus_secondk1 < 1 || result_minus_secondn1 > 1 && result_minus_secondp1 < 1 && result_minus_secondk1 > 1){
            unsigned char N_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x82, 0x00, 0x00, 0x00};
            unsigned char P_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x83, 0x00, 0x00, 0x00};
            unsigned char K_filThird[8] = {0x5A, 0xA5, 0x05, 0x82, 0x84, 0x00, 0x00, 0x00};
            value_fil = "Ammonium Sulfate";
            n_fil_third = 10.5;
            p_fil_third = 0;
            k_fil_third = 0;
            int n_fil_val_third = static_cast<int>(n_fil_third);  
            int p_fil_val_third = static_cast<int>(p_fil_third);  
            int k_fil_val_third = static_cast<int>(k_fil_third);  
            /*------Send Data to Display------*/
            N_filThird[6] = highByte(n_fil_val_third);
            N_filThird[7] = lowByte(n_fil_val_third);
            Serial2.write(N_filThird, 8);

            P_filThird[6] = highByte(p_fil_val_third);
            P_filThird[7] = lowByte(p_fil_val_third);
            Serial2.write(P_filThird, 8);

            K_filThird[6] = highByte(k_fil_val_third);
            K_filThird[7] = lowByte(k_fil_val_third);
            Serial2.write(K_filThird, 8);
          }
      }

      // Serial.println(n_fil_third);
      // Serial.println(p_fil_third);
      // Serial.println(k_fil_third);
      float result_divide_thirdn1 = (n_fil_third != 0.0) ? (result_minus_secondn1 / n_fil_third) : 0.0;;
      float result_divide_thirdp1 = (p_fil_third != 0.0) ? (result_minus_secondp1 / p_fil_third) : 0.0;
      float result_divide_thirdk1 = (k_fil_third != 0.0) ? (result_minus_secondk1 / k_fil_third) : 0.0;

      float lowest_value_third = findLowestNonZero(result_divide_thirdn1, result_divide_thirdp1, result_divide_thirdk1);
      int low_third = static_cast<int>(lowest_value_third);
      float decimal_part_third = lowest_value_third - static_cast<float>(low_third);
      int decimal_as_int3 = static_cast<int>((decimal_part_third * 100.0) + 0.5);
      
      float result_multip_thirdn1 = lowest_value_third * n_fil_third;
      float result_multip_thirdp1 = lowest_value_third * p_fil_third;
      float result_multip_thirdk1 = lowest_value_third * k_fil_third;

      float result_minus_thirdn1 = result_minus_secondn1 - result_multip_thirdn1;
      float result_minus_thirdp1 = result_minus_secondp1 - result_multip_thirdp1;
      float result_minus_thirdk1 = result_minus_secondk1 - result_multip_thirdk1;
      // // Serial.println(result_minus_secondn1);
      // // Serial.println(result_minus_secondp1);
      // // Serial.println(result_minus_secondk1);
      float divide2_decimal_third = static_cast<float>(decimal_as_int3) / 2.0;
      int rounded_value3 = static_cast<int>(round(divide2_decimal_third));
      int get_number_third = int(lowest_value_third);
      // Serial.println(get_number_third);
      // Serial.println(rounded_value3);
      unsigned char GetNumber3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x80, 0x00, 0x00, 0x00};
      int getnumber3 = static_cast<int>(get_number_third);   
      GetNumber3[6] = highByte(getnumber3);
      GetNumber3[7] = lowByte(getnumber3);
      Serial2.write(GetNumber3, 8);
      // kg
      unsigned char DivideDecimal3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x81, 0x00, 0x00, 0x00};
      int dividedecimal3 = static_cast<int>(rounded_value3);   
      DivideDecimal3[6] = highByte(dividedecimal3);
      DivideDecimal3[7] = lowByte(dividedecimal3);
      Serial2.write(DivideDecimal3, 8);
    }
  }
  Serial.println(nitro_split2);
  Serial.println(phos_split2);
  Serial.println(potas_split2);
  float n_fil_split2,p_fil_split2,k_fil_split2;
  if (nitro_split2 > 1 && phos_split2 > 1 && potas_split2 > 1){
    value_fil = "Complete, Triple 14";
    unsigned char N_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x87, 0x00, 0x00, 0x00};
    unsigned char P_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x88, 0x00, 0x00, 0x00};
    unsigned char K_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x89, 0x00, 0x00, 0x00};
    n_fil_split2 = 7;
    p_fil_split2 = 7;
    k_fil_split2 = 7;
    int n_fil_val_split2 = static_cast<int>(n_fil_split2);  
    int p_fil_val_split2 = static_cast<int>(p_fil_split2);  
    int k_fil_val_split2 = static_cast<int>(k_fil_split2);  
    /*------Send Data to Display------*/
    N_filSplit2[6] = highByte(n_fil_val_split2);
    N_filSplit2[7] = lowByte(n_fil_val_split2);
    Serial2.write(N_filSplit2, 8);

    P_filSplit2[6] = highByte(p_fil_val_split2);
    P_filSplit2[7] = lowByte(p_fil_val_split2);
    Serial2.write(P_filSplit2, 8);

    K_filSplit2[6] = highByte(k_fil_val_split2);
    K_filSplit2[7] = lowByte(k_fil_val_split2);
    Serial2.write(K_filSplit2, 8);
  }

  else if (nitro_split2 > 1 && phos_split2 > 1 && potas_split2 < 1){
    unsigned char N_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x87, 0x00, 0x00, 0x00};
    unsigned char P_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x88, 0x00, 0x00, 0x00};
    unsigned char K_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x89, 0x00, 0x00, 0x00};
    n_fil_split2 = 8;
    p_fil_split2 = 10;
    k_fil_split2 = 0;
    int n_fil_val_split2 = static_cast<int>(n_fil_split2);  
    int p_fil_val_split2 = static_cast<int>(p_fil_split2);  
    int k_fil_val_split2 = static_cast<int>(k_fil_split2);  
    /*------Send Data to Display------*/
    N_filSplit2[6] = highByte(n_fil_val_split2);
    N_filSplit2[7] = lowByte(n_fil_val_split2);
    Serial2.write(N_filSplit2, 8);

    P_filSplit2[6] = highByte(p_fil_val_split2);
    P_filSplit2[7] = lowByte(p_fil_val_split2);
    Serial2.write(P_filSplit2, 8);

    K_filSplit2[6] = highByte(k_fil_val_split2);
    K_filSplit2[7] = lowByte(k_fil_val_split2);
    Serial2.write(K_filSplit2, 8);
  }

  else if (nitro_split2 < 1 && phos_split2 > 1 && potas_split2 < 1){
    value_fil = "Superphospate";
    unsigned char N_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x87, 0x00, 0x00, 0x00};
    unsigned char P_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x88, 0x00, 0x00, 0x00};
    unsigned char K_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x89, 0x00, 0x00, 0x00};
    n_fil_split2 = 0;
    p_fil_split2 = 10;
    k_fil_split2 = 0;
    int n_fil_val_split2 = static_cast<int>(n_fil_split2);  
    int p_fil_val_split2 = static_cast<int>(p_fil_split2);  
    int k_fil_val_split2 = static_cast<int>(k_fil_split2);  
    /*------Send Data to Display------*/
    N_filSplit2[6] = highByte(n_fil_val_split2);
    N_filSplit2[7] = lowByte(n_fil_val_split2);
    Serial2.write(N_filSplit2, 8);

    P_filSplit2[6] = highByte(p_fil_val_split2);
    P_filSplit2[7] = lowByte(p_fil_val_split2);
    Serial2.write(P_filSplit2, 8);

    K_filSplit2[6] = highByte(k_fil_val_split2);
    K_filSplit2[7] = lowByte(k_fil_val_split2);
    Serial2.write(K_filSplit2, 8);
  }

  else if (nitro_split2 < 1 && phos_split2 < 1 && potas_split2 > 1 || nitro_split2 < 1 && phos_split2 > 1 && potas_split2 > 1){
    unsigned char N_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x87, 0x00, 0x00, 0x00};
    unsigned char P_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x88, 0x00, 0x00, 0x00};
    unsigned char K_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x89, 0x00, 0x00, 0x00};
    n_fil_split2 = 0;
    p_fil_split2 = 0;
    k_fil_split2 = 30;
    int n_fil_val_split2 = static_cast<int>(n_fil_split2);  
    int p_fil_val_split2 = static_cast<int>(p_fil_split2);  
    int k_fil_val_split2 = static_cast<int>(k_fil_split2);  
    /*------Send Data to Display------*/
    N_filSplit2[6] = highByte(n_fil_val_split2);
    N_filSplit2[7] = lowByte(n_fil_val_split2);
    Serial2.write(N_filSplit2, 8);

    P_filSplit2[6] = highByte(p_fil_val_split2);
    P_filSplit2[7] = lowByte(p_fil_val_split2);
    Serial2.write(P_filSplit2, 8);

    K_filSplit2[6] = highByte(k_fil_val_split2);
    K_filSplit2[7] = lowByte(k_fil_val_split2);
    Serial2.write(K_filSplit2, 8);
  }    

  if (pH < 6.6) {
        if (nitro_split2 > 1 && phos_split2 < 1 && potas_split2 < 1 || nitro_split2 > 1 && phos_split2 < 1 && potas_split2 > 1){
          unsigned char N_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x87, 0x00, 0x00, 0x00};
          unsigned char P_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x88, 0x00, 0x00, 0x00};
          unsigned char K_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x89, 0x00, 0x00, 0x00};
          value_fil = "Urea";
          n_fil_split2 = 23;
          p_fil_split2 = 0;
          k_fil_split2 = 0;
          int n_fil_val_split2 = static_cast<int>(n_fil_split2);  
          int p_fil_val_split2 = static_cast<int>(p_fil_split2);  
          int k_fil_val_split2 = static_cast<int>(k_fil_split2);  
          /*------Send Data to Display------*/
          N_filSplit2[6] = highByte(n_fil_val_split2);
          N_filSplit2[7] = lowByte(n_fil_val_split2);
          Serial2.write(N_filSplit2, 8);

          P_filSplit2[6] = highByte(p_fil_val_split2);
          P_filSplit2[7] = lowByte(p_fil_val_split2);
          Serial2.write(P_filSplit2, 8);

          K_filSplit2[6] = highByte(k_fil_val_split2);
          K_filSplit2[7] = lowByte(k_fil_val_split2);
          Serial2.write(K_filSplit2, 8);
        }
  }
  else {
      if (nitro_split2 > 1 && phos_split2 < 1 && potas_split2 < 1 || nitro_split2 > 1 && phos_split2 < 1 && potas_split2 > 1){
        unsigned char N_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x87, 0x00, 0x00, 0x00};
        unsigned char P_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x88, 0x00, 0x00, 0x00};
        unsigned char K_filSplit2[8] = {0x5A, 0xA5, 0x05, 0x82, 0x89, 0x00, 0x00, 0x00};
        value_fil = "Ammonium Sulfate";
        n_fil_split2 = 10.5;
        p_fil_split2 = 0;
        k_fil_split2 = 0;
        int n_fil_val_split2 = static_cast<int>(n_fil_split2);  
        int p_fil_val_split2 = static_cast<int>(p_fil_split2);  
        int k_fil_val_split2 = static_cast<int>(k_fil_split2);  
        /*------Send Data to Display------*/
        N_filSplit2[6] = highByte(n_fil_val_split2);
        N_filSplit2[7] = lowByte(n_fil_val_split2);
        Serial2.write(N_filSplit2, 8);

        P_filSplit2[6] = highByte(p_fil_val_split2);
        P_filSplit2[7] = lowByte(p_fil_val_split2);
        Serial2.write(P_filSplit2, 8);

        K_filSplit2[6] = highByte(k_fil_val_split2);
        K_filSplit2[7] = lowByte(k_fil_val_split2);
        Serial2.write(K_filSplit2, 8);
      }
  }
  float result_divide_splitn1 = (n_fil_split2 != 0.0) ? (nitro_split2 / n_fil_split2) : 0.0;;
  float result_divide_splitp1 = (p_fil_split2 != 0.0) ? (phos_split2 / p_fil_split2) : 0.0;
  float result_divide_splitk1 = (k_fil_split2 != 0.0) ? (potas_split2 / k_fil_split2) : 0.0;

  float lowest_value_split2 = findLowestNonZero(result_divide_splitn1, result_divide_splitp1, result_divide_splitk1);
  int low_split2 = static_cast<int>(lowest_value_split2);
  float decimal_part_split2 = lowest_value_split2 - static_cast<float>(low_split2);
  int decimal_as_int4 = static_cast<int>((decimal_part_split2 * 100.0) + 0.5);

  float result_multip_split2n1 = lowest_value_split2 * n_fil_split2;
  float result_multip_split2p1 = lowest_value_split2 * p_fil_split2;
  float result_multip_split2k1 = lowest_value_split2 * k_fil_split2;

  float result_minus_split2n1 = nitro_split2 - result_multip_split2n1;
  float result_minus_split2p1 = phos_split2 - result_multip_split2p1;
  float result_minus_split2k1 = potas_split2 - result_multip_split2k1;

  float divide2_decimal_split2 = static_cast<float>(decimal_as_int4) / 2.0;
  int rounded_value4 = static_cast<int>(round(divide2_decimal_split2));
  int get_number_split2 = int(lowest_value_split2);
  
  // bags
  unsigned char GetNumber4[8] = {0x5A, 0xA5, 0x05, 0x82, 0x85, 0x00, 0x00, 0x00};
  int getnumber4 = static_cast<int>(get_number_split2);   
  GetNumber4[6] = highByte(getnumber4);
  GetNumber4[7] = lowByte(getnumber4);
  Serial2.write(GetNumber4, 8);
  // kg
  unsigned char DivideDecimal4[8] = {0x5A, 0xA5, 0x05, 0x82, 0x86, 0x00, 0x00, 0x00};
  int dividedecimal4 = static_cast<int>(rounded_value4);   
  DivideDecimal4[6] = highByte(dividedecimal4);
  DivideDecimal4[7] = lowByte(dividedecimal4);
  Serial2.write(DivideDecimal4, 8);


  if (result_minus_split2n1 != 0 && result_minus_split2p1 != 0 && result_minus_split2k1 != 0){
    Serial.println("All Values are Zero!");
  } else {
    float n_fil_split2_second,p_fil_split2_second,k_fil_split2_second;
    if (result_minus_split2n1 > 1 && result_minus_split2p1 > 1 && result_minus_split2k1 > 1){
      value_fil = "Complete, Triple 14";
      unsigned char N_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x92, 0x00, 0x00, 0x00};
      unsigned char P_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x92, 0x50, 0x00, 0x00};
      unsigned char K_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x93, 0x00, 0x00, 0x00};
      n_fil_split2_second = 7;
      p_fil_split2_second = 7;
      k_fil_split2_second = 7;
      int n_fil_val_split2_second = static_cast<int>(n_fil_split2_second);  
      int p_fil_val_split2_second = static_cast<int>(p_fil_split2_second);  
      int k_fil_val_split2_second = static_cast<int>(k_fil_split2_second);  
      /*------Send Data to Display------*/
      N_filSplit2_Second[6] = highByte(n_fil_val_split2_second);
      N_filSplit2_Second[7] = lowByte(n_fil_val_split2_second);
      Serial2.write(N_filSplit2_Second, 8);

      P_filSplit2_Second[6] = highByte(p_fil_val_split2_second);
      P_filSplit2_Second[7] = lowByte(p_fil_val_split2_second);
      Serial2.write(P_filSplit2_Second, 8);

      K_filSplit2_Second[6] = highByte(k_fil_val_split2_second);
      K_filSplit2_Second[7] = lowByte(k_fil_val_split2_second);
      Serial2.write(K_filSplit2_Second, 8);
    }

    else if (result_minus_split2n1 > 1 && result_minus_split2p1 > 1 && result_minus_split2k1 < 1){
      unsigned char N_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x92, 0x00, 0x00, 0x00};
      unsigned char P_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x92, 0x50, 0x00, 0x00};
      unsigned char K_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x93, 0x00, 0x00, 0x00};
      n_fil_split2_second = 8;
      p_fil_split2_second = 10;
      k_fil_split2_second = 0;
      int n_fil_val_split2_second = static_cast<int>(n_fil_split2_second);  
      int p_fil_val_split2_second = static_cast<int>(p_fil_split2_second);  
      int k_fil_val_split2_second = static_cast<int>(k_fil_split2_second);  
      /*------Send Data to Display------*/
      N_filSplit2_Second[6] = highByte(n_fil_val_split2_second);
      N_filSplit2_Second[7] = lowByte(n_fil_val_split2_second);
      Serial2.write(N_filSplit2_Second, 8);

      P_filSplit2_Second[6] = highByte(p_fil_val_split2_second);
      P_filSplit2_Second[7] = lowByte(p_fil_val_split2_second);
      Serial2.write(P_filSplit2_Second, 8);

      K_filSplit2_Second[6] = highByte(k_fil_val_split2_second);
      K_filSplit2_Second[7] = lowByte(k_fil_val_split2_second);
      Serial2.write(K_filSplit2_Second, 8);
    }

    else if (result_minus_split2n1 < 1 && result_minus_split2p1 > 1 && result_minus_split2k1 < 1){
      value_fil = "Superphospate";
      unsigned char N_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x92, 0x00, 0x00, 0x00};
      unsigned char P_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x92, 0x50, 0x00, 0x00};
      unsigned char K_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x93, 0x00, 0x00, 0x00};
      n_fil_split2_second = 0;
      p_fil_split2_second = 10;
      k_fil_split2_second = 0;
      int n_fil_val_split2 = static_cast<int>(n_fil_split2_second);  
      int p_fil_val_split2 = static_cast<int>(p_fil_split2_second);  
      int k_fil_val_split2 = static_cast<int>(k_fil_split2_second);  
      /*------Send Data to Display------*/
      int n_fil_val_split2_second = static_cast<int>(n_fil_split2_second);  
      int p_fil_val_split2_second = static_cast<int>(p_fil_split2_second);  
      int k_fil_val_split2_second = static_cast<int>(k_fil_split2_second);  
      /*------Send Data to Display------*/
      N_filSplit2_Second[6] = highByte(n_fil_val_split2_second);
      N_filSplit2_Second[7] = lowByte(n_fil_val_split2_second);
      Serial2.write(N_filSplit2_Second, 8);

      P_filSplit2_Second[6] = highByte(p_fil_val_split2_second);
      P_filSplit2_Second[7] = lowByte(p_fil_val_split2_second);
      Serial2.write(P_filSplit2_Second, 8);

      K_filSplit2_Second[6] = highByte(k_fil_val_split2_second);
      K_filSplit2_Second[7] = lowByte(k_fil_val_split2_second);
      Serial2.write(K_filSplit2_Second, 8);
    }

    else if (result_minus_split2n1 < 1 && result_minus_split2p1 < 1 && result_minus_split2k1 > 1 || result_minus_split2n1 < 1 && result_minus_split2p1 > 1 && result_minus_split2k1 > 1){
      unsigned char N_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x92, 0x00, 0x00, 0x00};
      unsigned char P_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x92, 0x50, 0x00, 0x00};
      unsigned char K_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x93, 0x00, 0x00, 0x00};
      n_fil_split2_second = 0;
      p_fil_split2_second = 0;
      k_fil_split2_second = 30;
      int n_fil_val_split2_second = static_cast<int>(n_fil_split2_second);  
      int p_fil_val_split2_second = static_cast<int>(p_fil_split2_second);  
      int k_fil_val_split2_second = static_cast<int>(k_fil_split2_second);  
      /*------Send Data to Display------*/
      N_filSplit2_Second[6] = highByte(n_fil_val_split2_second);
      N_filSplit2_Second[7] = lowByte(n_fil_val_split2_second);
      Serial2.write(N_filSplit2_Second, 8);

      P_filSplit2_Second[6] = highByte(p_fil_val_split2_second);
      P_filSplit2_Second[7] = lowByte(p_fil_val_split2_second);
      Serial2.write(P_filSplit2_Second, 8);

      K_filSplit2_Second[6] = highByte(k_fil_val_split2_second);
      K_filSplit2_Second[7] = lowByte(k_fil_val_split2_second);
      Serial2.write(K_filSplit2_Second, 8);
    }    

    if (pH < 6.6) {
          if (result_minus_split2n1 > 1 && result_minus_split2p1 < 1 && result_minus_split2k1 < 1 || result_minus_split2n1 > 1 && result_minus_split2p1 < 1 && result_minus_split2k1 > 1){
            unsigned char N_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x92, 0x00, 0x00, 0x00};
            unsigned char P_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x92, 0x50, 0x00, 0x00};
            unsigned char K_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x93, 0x00, 0x00, 0x00};
            value_fil = "Urea";
            n_fil_split2_second = 23;
            p_fil_split2_second = 0;
            k_fil_split2_second = 0;
            int n_fil_val_split2_second = static_cast<int>(n_fil_split2_second);  
            int p_fil_val_split2_second = static_cast<int>(p_fil_split2_second);  
            int k_fil_val_split2_second = static_cast<int>(k_fil_split2_second);  
            /*------Send Data to Display------*/
            N_filSplit2_Second[6] = highByte(n_fil_val_split2_second);
            N_filSplit2_Second[7] = lowByte(n_fil_val_split2_second);
            Serial2.write(N_filSplit2_Second, 8);

            P_filSplit2_Second[6] = highByte(p_fil_val_split2_second);
            P_filSplit2_Second[7] = lowByte(p_fil_val_split2_second);
            Serial2.write(P_filSplit2_Second, 8);

            K_filSplit2_Second[6] = highByte(k_fil_val_split2_second);
            K_filSplit2_Second[7] = lowByte(k_fil_val_split2_second);
            Serial2.write(K_filSplit2_Second, 8);
          }
    }
    else {
        if (result_minus_split2n1 > 1 && result_minus_split2p1 < 1 && result_minus_split2k1 < 1 || result_minus_split2n1 > 1 && result_minus_split2p1 < 1 && result_minus_split2k1 > 1){
          unsigned char N_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x92, 0x00, 0x00, 0x00};
          unsigned char P_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x92, 0x50, 0x00, 0x00};
          unsigned char K_filSplit2_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x93, 0x00, 0x00, 0x00};
          value_fil = "Ammonium Sulfate";
          n_fil_split2_second = 10.5;
          p_fil_split2_second = 0;
          k_fil_split2_second = 0;
          int n_fil_val_split2_second = static_cast<int>(n_fil_split2_second);  
          int p_fil_val_split2_second = static_cast<int>(p_fil_split2_second);  
          int k_fil_val_split2_second = static_cast<int>(k_fil_split2_second);  
          /*------Send Data to Display------*/
          N_filSplit2_Second[6] = highByte(n_fil_val_split2_second);
          N_filSplit2_Second[7] = lowByte(n_fil_val_split2_second);
          Serial2.write(N_filSplit2_Second, 8);

          P_filSplit2_Second[6] = highByte(p_fil_val_split2_second);
          P_filSplit2_Second[7] = lowByte(p_fil_val_split2_second);
          Serial2.write(P_filSplit2_Second, 8);

          K_filSplit2_Second[6] = highByte(k_fil_val_split2_second);
          K_filSplit2_Second[7] = lowByte(k_fil_val_split2_second);
          Serial2.write(K_filSplit2_Second, 8);
        }
    }
    float result_divide_splitn1_second = (n_fil_split2_second != 0.0) ? (result_minus_split2n1 / n_fil_split2_second) : 0.0;;
    float result_divide_splitp1_second = (p_fil_split2_second != 0.0) ? (result_minus_split2p1 / p_fil_split2_second) : 0.0;
    float result_divide_splitk1_second = (k_fil_split2_second != 0.0) ? (result_minus_split2k1 / k_fil_split2_second) : 0.0;

    float lowest_value_split2_second = findLowestNonZero(result_divide_splitn1_second, result_divide_splitp1_second, result_divide_splitk1_second);
    int low_split2_second = static_cast<int>(lowest_value_split2_second);
    float decimal_part_split2 = lowest_value_split2_second - static_cast<float>(low_split2_second);
    int decimal_as_int5 = static_cast<int>((decimal_part_split2 * 100.0) + 0.5);

    float result_multip_split2n1_second = lowest_value_split2_second * n_fil_split2_second;
    float result_multip_split2p1_second = lowest_value_split2_second * p_fil_split2_second;
    float result_multip_split2k1_second = lowest_value_split2_second * k_fil_split2_second;

    float result_minus_split2n1_final = result_minus_split2n1 - result_multip_split2n1_second;
    float result_minus_split2p1_final = result_minus_split2p1 - result_multip_split2p1_second;
    float result_minus_split2k1_final = result_minus_split2k1 - result_multip_split2k1_second;

    float divide2_decimal_split2_second = static_cast<float>(decimal_as_int5) / 2.0;
    int rounded_value5 = static_cast<int>(round(divide2_decimal_split2_second));
    int get_number_split2_second = int(lowest_value_split2_second);
    
    // bags
    unsigned char GetNumber5[8] = {0x5A, 0xA5, 0x05, 0x82, 0x90, 0x00, 0x00, 0x00};
    int getnumber5 = static_cast<int>(get_number_split2_second);   
    GetNumber5[6] = highByte(getnumber5);
    GetNumber5[7] = lowByte(getnumber5);
    Serial2.write(GetNumber5, 8);
    // kg
    unsigned char DivideDecimal5[8] = {0x5A, 0xA5, 0x05, 0x82, 0x91, 0x00, 0x00, 0x00};
    int dividedecimal5 = static_cast<int>(rounded_value5);   
    DivideDecimal5[6] = highByte(dividedecimal5);
    DivideDecimal5[7] = lowByte(dividedecimal5);
    Serial2.write(DivideDecimal5, 8);
  }
  Serial.println(nitro_split3);
  Serial.println(phos_split3);
  Serial.println(potas_split3);
  float n_fil_split3,p_fil_split3,k_fil_split3;
  if (nitro_split3 > 1 && phos_split3 > 1 && potas_split3 > 1){
    value_fil = "Complete, Triple 14";
    unsigned char N_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x96, 0x00, 0x00, 0x00};
    unsigned char P_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x97, 0x00, 0x00, 0x00};
    unsigned char K_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x98, 0x00, 0x00, 0x00};
    n_fil_split3 = 7;
    p_fil_split3 = 7;
    k_fil_split3 = 7;
    int n_fil_val_split3 = static_cast<int>(n_fil_split3);  
    int p_fil_val_split3 = static_cast<int>(p_fil_split3);  
    int k_fil_val_split3 = static_cast<int>(k_fil_split3);  
    /*------Send Data to Display------*/
    N_filSplit3[6] = highByte(n_fil_val_split3);
    N_filSplit3[7] = lowByte(n_fil_val_split3);
    Serial2.write(N_filSplit3, 8);

    P_filSplit3[6] = highByte(p_fil_val_split3);
    P_filSplit3[7] = lowByte(p_fil_val_split3);
    Serial2.write(P_filSplit3, 8);

    K_filSplit3[6] = highByte(k_fil_val_split3);
    K_filSplit3[7] = lowByte(k_fil_val_split3);
    Serial2.write(K_filSplit3, 8);

  }

  else if (nitro_split3 > 1 && phos_split3 > 1 && potas_split3 < 1){
    unsigned char N_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x96, 0x00, 0x00, 0x00};
    unsigned char P_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x97, 0x00, 0x00, 0x00};
    unsigned char K_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x98, 0x00, 0x00, 0x00};
    n_fil_split3 = 8;
    p_fil_split3 = 10;
    k_fil_split3 = 0;
    int n_fil_val_split3 = static_cast<int>(n_fil_split3);  
    int p_fil_val_split3 = static_cast<int>(p_fil_split3);  
    int k_fil_val_split3 = static_cast<int>(k_fil_split3);  
    /*------Send Data to Display------*/
    N_filSplit3[6] = highByte(n_fil_val_split3);
    N_filSplit3[7] = lowByte(n_fil_val_split3);
    Serial2.write(N_filSplit3, 8);

    P_filSplit3[6] = highByte(p_fil_val_split3);
    P_filSplit3[7] = lowByte(p_fil_val_split3);
    Serial2.write(P_filSplit3, 8);

    K_filSplit3[6] = highByte(k_fil_val_split3);
    K_filSplit3[7] = lowByte(k_fil_val_split3);
    Serial2.write(K_filSplit3, 8);
  }

  else if (nitro_split3 < 1 && phos_split3 > 1 && potas_split3 < 1){
    unsigned char N_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x96, 0x00, 0x00, 0x00};
    unsigned char P_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x97, 0x00, 0x00, 0x00};
    unsigned char K_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x98, 0x00, 0x00, 0x00};
    value_fil = "Superphospate";
    n_fil_split3 = 0;
    p_fil_split3 = 10;
    k_fil_split3 = 0;
    int n_fil_val_split3 = static_cast<int>(n_fil_split3);  
    int p_fil_val_split3 = static_cast<int>(p_fil_split3);  
    int k_fil_val_split3 = static_cast<int>(k_fil_split3);  
    /*------Send Data to Display------*/
    N_filSplit3[6] = highByte(n_fil_val_split3);
    N_filSplit3[7] = lowByte(n_fil_val_split3);
    Serial2.write(N_filSplit3, 8);

    P_filSplit3[6] = highByte(p_fil_val_split3);
    P_filSplit3[7] = lowByte(p_fil_val_split3);
    Serial2.write(P_filSplit3, 8);

    K_filSplit3[6] = highByte(k_fil_val_split3);
    K_filSplit3[7] = lowByte(k_fil_val_split3);
    Serial2.write(K_filSplit3, 8);
  }

  else if (nitro_split3 < 1 && phos_split3 < 1 && potas_split3 > 1 || nitro_split3 < 1 && phos_split3 > 1 && potas_split3 > 1){
    unsigned char N_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x96, 0x00, 0x00, 0x00};
    unsigned char P_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x97, 0x00, 0x00, 0x00};
    unsigned char K_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x98, 0x00, 0x00, 0x00};
    n_fil_split3 = 0;
    p_fil_split3 = 0;
    k_fil_split3 = 30;
    int n_fil_val_split3 = static_cast<int>(n_fil_split3);  
    int p_fil_val_split3 = static_cast<int>(p_fil_split3);  
    int k_fil_val_split3 = static_cast<int>(k_fil_split3);  
    /*------Send Data to Display------*/
    N_filSplit3[6] = highByte(n_fil_val_split3);
    N_filSplit3[7] = lowByte(n_fil_val_split3);
    Serial2.write(N_filSplit3, 8);

    P_filSplit3[6] = highByte(p_fil_val_split3);
    P_filSplit3[7] = lowByte(p_fil_val_split3);
    Serial2.write(P_filSplit3, 8);

    K_filSplit3[6] = highByte(k_fil_val_split3);
    K_filSplit3[7] = lowByte(k_fil_val_split3);
    Serial2.write(K_filSplit3, 8);

  }    

  if (pH < 6.6) {
        if (nitro_split3 > 1 && phos_split3 < 1 && potas_split3 < 1 || nitro_split3 > 1 && phos_split3 < 1 && potas_split3 > 1){
          unsigned char N_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x96, 0x00, 0x00, 0x00};
          unsigned char P_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x97, 0x00, 0x00, 0x00};
          unsigned char K_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x98, 0x00, 0x00, 0x00};
          value_fil = "Urea";
          n_fil_split3 = 23;
          p_fil_split3 = 0;
          k_fil_split3 = 0;
          int n_fil_val_split3 = static_cast<int>(n_fil_split3);  
          int p_fil_val_split3 = static_cast<int>(p_fil_split3);  
          int k_fil_val_split3 = static_cast<int>(k_fil_split3);  
          /*------Send Data to Display------*/
          N_filSplit3[6] = highByte(n_fil_val_split3);
          N_filSplit3[7] = lowByte(n_fil_val_split3);
          Serial2.write(N_filSplit3, 8);

          P_filSplit3[6] = highByte(p_fil_val_split3);
          P_filSplit3[7] = lowByte(p_fil_val_split3);
          Serial2.write(P_filSplit3, 8);

          K_filSplit3[6] = highByte(k_fil_val_split3);
          K_filSplit3[7] = lowByte(k_fil_val_split3);
          Serial2.write(K_filSplit3, 8);
        }
  }
  else {
      if (nitro_split3 > 1 && phos_split3 < 1 && potas_split3 < 1 || nitro_split3 > 1 && phos_split3 < 1 && potas_split3 > 1){
        unsigned char N_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x96, 0x00, 0x00, 0x00};
        unsigned char P_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x97, 0x00, 0x00, 0x00};
        unsigned char K_filSplit3[8] = {0x5A, 0xA5, 0x05, 0x82, 0x98, 0x00, 0x00, 0x00};
        value_fil = "Ammonium Sulfate";
        n_fil_split3 = 10.5;
        p_fil_split3 = 0;
        k_fil_split3 = 0;
        int n_fil_val_split3 = static_cast<int>(n_fil_split3);  
        int p_fil_val_split3 = static_cast<int>(p_fil_split3);  
        int k_fil_val_split3 = static_cast<int>(k_fil_split3);  
        /*------Send Data to Display------*/
        N_filSplit3[6] = highByte(n_fil_val_split3);
        N_filSplit3[7] = lowByte(n_fil_val_split3);
        Serial2.write(N_filSplit3, 8);

        P_filSplit3[6] = highByte(p_fil_val_split3);
        P_filSplit3[7] = lowByte(p_fil_val_split3);
        Serial2.write(P_filSplit3, 8);

        K_filSplit3[6] = highByte(k_fil_val_split3);
        K_filSplit3[7] = lowByte(k_fil_val_split3);
        Serial2.write(K_filSplit3, 8);
      }
  }
  float result_divide_split3n1 = (n_fil_split3 != 0.0) ? (nitro_split3 / n_fil_split3) : 0.0;;
  float result_divide_split3p1 = (p_fil_split3 != 0.0) ? (phos_split3 / p_fil_split3) : 0.0;
  float result_divide_split3k1 = (k_fil_split3 != 0.0) ? (potas_split3 / k_fil_split3) : 0.0;

  float lowest_value_split3 = findLowestNonZero(result_divide_split3n1, result_divide_split3p1, result_divide_split3k1);
  int low_split3 = static_cast<int>(lowest_value_split3);
  float decimal_part_split3 = lowest_value_split3 - static_cast<float>(low_split3);
  int decimal_as_int6 = static_cast<int>((decimal_part_split3 * 100.0) + 0.5);

  float result_multip_split3n1 = lowest_value_split3 * n_fil_split3;
  float result_multip_split3p1 = lowest_value_split3 * p_fil_split3;
  float result_multip_split3k1 = lowest_value_split3 * k_fil_split3;

  float result_minus_split3n1 = nitro_split3 - result_multip_split3n1;
  float result_minus_split3p1 = phos_split3 - result_multip_split3p1;
  float result_minus_split3k1 = potas_split3 - result_multip_split3k1;

  float divide2_decimal_split3 = static_cast<float>(decimal_as_int6) / 2.0;
  int rounded_value6 = static_cast<int>(round(divide2_decimal_split3));
  int get_number_split3 = int(lowest_value_split3);
  // Serial.println(get_number_split3);
  // Serial.println(rounded_value6);
  // bags
  unsigned char GetNumber6[8] = {0x5A, 0xA5, 0x05, 0x82, 0x94, 0x00, 0x00, 0x00};
  int getnumber6 = static_cast<int>(get_number_split3);   
  GetNumber6[6] = highByte(getnumber6);
  GetNumber6[7] = lowByte(getnumber6);
  Serial2.write(GetNumber6, 8);
  // kg
  unsigned char DivideDecimal6[8] = {0x5A, 0xA5, 0x05, 0x82, 0x95, 0x00, 0x00, 0x00};
  int dividedecimal6 = static_cast<int>(rounded_value6);   
  DivideDecimal6[6] = highByte(dividedecimal6);
  DivideDecimal6[7] = lowByte(dividedecimal6);
  Serial2.write(DivideDecimal6, 8);


  if (result_minus_split3n1 != 0 && result_minus_split3p1 != 0 && result_minus_split3k1 != 0){
    Serial.println("All Values are Zero!");
  } else {
    float n_fil_split3_second,p_fil_split3_second,k_fil_split3_second;
    if (result_minus_split3n1 > 1 && result_minus_split3p1 > 1 && result_minus_split3k1 > 1){
      value_fil = "Complete, Triple 14";
      unsigned char N_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x12, 0x00, 0x00, 0x00};
      unsigned char P_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x13, 0x00, 0x00, 0x00};
      unsigned char K_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x14, 0x00, 0x00, 0x00};
      n_fil_split3_second = 7;
      p_fil_split3_second = 7;
      k_fil_split3_second = 7;
      int n_fil_val_split3_second = static_cast<int>(n_fil_split3_second);  
      int p_fil_val_split3_second = static_cast<int>(p_fil_split3_second);  
      int k_fil_val_split3_second = static_cast<int>(k_fil_split3_second);  
      /*------Send Data to Display------*/
      N_filSplit3_Second[6] = highByte(n_fil_val_split3_second);
      N_filSplit3_Second[7] = lowByte(n_fil_val_split3_second);
      Serial2.write(N_filSplit3_Second, 8);

      P_filSplit3_Second[6] = highByte(p_fil_val_split3_second);
      P_filSplit3_Second[7] = lowByte(p_fil_val_split3_second);
      Serial2.write(P_filSplit3_Second, 8);

      K_filSplit3_Second[6] = highByte(k_fil_val_split3_second);
      K_filSplit3_Second[7] = lowByte(k_fil_val_split3_second);
      Serial2.write(K_filSplit3_Second, 8);
    }

    else if (result_minus_split3n1 > 1 && result_minus_split3p1 > 1 && result_minus_split3k1 < 1){
      unsigned char N_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x12, 0x00, 0x00, 0x00};
      unsigned char P_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x13, 0x00, 0x00, 0x00};
      unsigned char K_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x14, 0x00, 0x00, 0x00};
      n_fil_split3_second = 8;
      p_fil_split3_second = 10;
      k_fil_split3_second = 0;
      int n_fil_val_split3_second = static_cast<int>(n_fil_split3_second);  
      int p_fil_val_split3_second = static_cast<int>(p_fil_split3_second);  
      int k_fil_val_split3_second = static_cast<int>(k_fil_split3_second);  
      /*------Send Data to Display------*/
      N_filSplit3_Second[6] = highByte(n_fil_val_split3_second);
      N_filSplit3_Second[7] = lowByte(n_fil_val_split3_second);
      Serial2.write(N_filSplit3_Second, 8);

      P_filSplit3_Second[6] = highByte(p_fil_val_split3_second);
      P_filSplit3_Second[7] = lowByte(p_fil_val_split3_second);
      Serial2.write(P_filSplit3_Second, 8);

      K_filSplit3_Second[6] = highByte(k_fil_val_split3_second);
      K_filSplit3_Second[7] = lowByte(k_fil_val_split3_second);
      Serial2.write(K_filSplit3_Second, 8);
    }

    else if (result_minus_split3n1 < 1 && result_minus_split3p1 > 1 && result_minus_split3k1 < 1){
      unsigned char N_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x12, 0x00, 0x00, 0x00};
      unsigned char P_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x13, 0x00, 0x00, 0x00};
      unsigned char K_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x14, 0x00, 0x00, 0x00};
      value_fil = "Superphospate";
      n_fil_split3_second = 0;
      p_fil_split3_second = 10;
      k_fil_split3_second = 0;
      int n_fil_val_split3_second = static_cast<int>(n_fil_split3_second);  
      int p_fil_val_split3_second = static_cast<int>(p_fil_split3_second);  
      int k_fil_val_split3_second = static_cast<int>(k_fil_split3_second);  
      /*------Send Data to Display------*/
      N_filSplit3_Second[6] = highByte(n_fil_val_split3_second);
      N_filSplit3_Second[7] = lowByte(n_fil_val_split3_second);
      Serial2.write(N_filSplit3_Second, 8);

      P_filSplit3_Second[6] = highByte(p_fil_val_split3_second);
      P_filSplit3_Second[7] = lowByte(p_fil_val_split3_second);
      Serial2.write(P_filSplit3_Second, 8);

      K_filSplit3_Second[6] = highByte(k_fil_val_split3_second);
      K_filSplit3_Second[7] = lowByte(k_fil_val_split3_second);
      Serial2.write(K_filSplit3_Second, 8);
    }

    else if (result_minus_split3n1 < 1 && result_minus_split3p1 < 1 && result_minus_split3k1 > 1 || result_minus_split3n1 < 1 && result_minus_split3p1 > 1 && result_minus_split3k1 > 1){
      unsigned char N_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x12, 0x00, 0x00, 0x00};
      unsigned char P_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x13, 0x00, 0x00, 0x00};
      unsigned char K_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x14, 0x00, 0x00, 0x00};
      n_fil_split3_second = 0;
      p_fil_split3_second = 0;
      k_fil_split3_second = 30;
      int n_fil_val_split3_second = static_cast<int>(n_fil_split3_second);  
      int p_fil_val_split3_second = static_cast<int>(p_fil_split3_second);  
      int k_fil_val_split3_second = static_cast<int>(k_fil_split3_second);  
      /*------Send Data to Display------*/
      N_filSplit3_Second[6] = highByte(n_fil_val_split3_second);
      N_filSplit3_Second[7] = lowByte(n_fil_val_split3_second);
      Serial2.write(N_filSplit3_Second, 8);

      P_filSplit3_Second[6] = highByte(p_fil_val_split3_second);
      P_filSplit3_Second[7] = lowByte(p_fil_val_split3_second);
      Serial2.write(P_filSplit3_Second, 8);

      K_filSplit3_Second[6] = highByte(k_fil_val_split3_second);
      K_filSplit3_Second[7] = lowByte(k_fil_val_split3_second);
      Serial2.write(K_filSplit3_Second, 8);
    }    

    if (pH < 6.6) {
      if (result_minus_split3n1 > 1 && result_minus_split3p1 < 1 && result_minus_split3k1 < 1 || result_minus_split3n1 > 1 && result_minus_split3p1 < 1 && result_minus_split3k1 > 1){
          unsigned char N_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x12, 0x00, 0x00, 0x00};
          unsigned char P_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x13, 0x00, 0x00, 0x00};
          unsigned char K_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x14, 0x00, 0x00, 0x00};
          n_fil_split3_second = 23;
          p_fil_split3_second = 0;
          k_fil_split3_second = 0;
          int n_fil_val_split3_second = static_cast<int>(n_fil_split3_second);  
          int p_fil_val_split3_second = static_cast<int>(p_fil_split3_second);  
          int k_fil_val_split3_second = static_cast<int>(k_fil_split3_second);  
          /*------Send Data to Display------*/
          N_filSplit3_Second[6] = highByte(n_fil_val_split3_second);
          N_filSplit3_Second[7] = lowByte(n_fil_val_split3_second);
          Serial2.write(N_filSplit3_Second, 8);

          P_filSplit3_Second[6] = highByte(p_fil_val_split3_second);
          P_filSplit3_Second[7] = lowByte(p_fil_val_split3_second);
          Serial2.write(P_filSplit3_Second, 8);

          K_filSplit3_Second[6] = highByte(k_fil_val_split3_second);
          K_filSplit3_Second[7] = lowByte(k_fil_val_split3_second);
          Serial2.write(K_filSplit3_Second, 8);
      }
    }
    else {
      if (result_minus_split3n1 > 1 && result_minus_split3p1 < 1 && result_minus_split3k1 < 1 || result_minus_split3n1 > 1 && result_minus_split3p1 < 1 && result_minus_split3k1 > 1){
        unsigned char N_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x12, 0x00, 0x00, 0x00};
        unsigned char P_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x13, 0x00, 0x00, 0x00};
        unsigned char K_filSplit3_Second[8] = {0x5A, 0xA5, 0x05, 0x82, 0x14, 0x00, 0x00, 0x00};
        value_fil = "Ammonium Sulfate";
        n_fil_split3_second = 10.5;
        p_fil_split3_second = 0;
        k_fil_split3_second = 0;
        int n_fil_val_split3_second = static_cast<int>(n_fil_split3_second);  
        int p_fil_val_split3_second = static_cast<int>(p_fil_split3_second);  
        int k_fil_val_split3_second = static_cast<int>(k_fil_split3_second);  
        /*------Send Data to Display------*/
        N_filSplit3_Second[6] = highByte(n_fil_val_split3_second);
        N_filSplit3_Second[7] = lowByte(n_fil_val_split3_second);
        Serial2.write(N_filSplit3_Second, 8);

        P_filSplit3_Second[6] = highByte(p_fil_val_split3_second);
        P_filSplit3_Second[7] = lowByte(p_fil_val_split3_second);
        Serial2.write(P_filSplit3_Second, 8);

        K_filSplit3_Second[6] = highByte(k_fil_val_split3_second);
        K_filSplit3_Second[7] = lowByte(k_fil_val_split3_second);
        Serial2.write(K_filSplit3_Second, 8);
      }
    }
    float result_divide_split3n1_second = (n_fil_split3_second != 0.0) ? (result_minus_split3n1 / n_fil_split3_second) : 0.0;;
    float result_divide_split3p1_second = (p_fil_split3_second != 0.0) ? (result_minus_split3p1 / p_fil_split3_second) : 0.0;
    float result_divide_split3k1_second = (k_fil_split3_second != 0.0) ? (result_minus_split3k1 / k_fil_split3_second) : 0.0;
    // Serial.println(result_minus_split3n1);
    // Serial.println(result_minus_split3p1);
    // Serial.println(result_minus_split3k1);
    float lowest_value_split3_second = findLowestNonZero(result_divide_split3n1_second, result_divide_split3p1_second, result_divide_split3k1_second);
    int low_split3_second = static_cast<int>(lowest_value_split3_second);
    float decimal_part_split2 = lowest_value_split3_second - static_cast<float>(low_split3_second);
    int decimal_as_int7 = static_cast<int>((decimal_part_split2 * 100.0) + 0.5);

    float result_multip_split3n1_second = lowest_value_split3_second * n_fil_split3_second;
    float result_multip_split3p1_second = lowest_value_split3_second * p_fil_split3_second;
    float result_multip_split3k1_second = lowest_value_split3_second * k_fil_split3_second;

    float result_minus_split3n1_final = result_minus_split3n1 - result_multip_split3n1_second;
    float result_minus_split3p1_final = result_minus_split3p1 - result_multip_split3p1_second;
    float result_minus_split3k1_final = result_minus_split3k1 - result_multip_split3k1_second;

    float divide2_decimal_split3_second = static_cast<float>(decimal_as_int7) / 2.0;
    int rounded_value7 = static_cast<int>(round(divide2_decimal_split3_second));
    int get_number_split3_second = int(lowest_value_split3_second);
    // Serial.println(get_number_split3_second);
    // Serial.println(rounded_value7);
    // bags
    unsigned char GetNumber7[8] = {0x5A, 0xA5, 0x05, 0x82, 0x99, 0x00, 0x00, 0x00};
    int getnumber7 = static_cast<int>(get_number_split3_second);   
    GetNumber7[6] = highByte(getnumber7);
    GetNumber7[7] = lowByte(getnumber7);
    Serial2.write(GetNumber7, 8);
    // kg
    unsigned char DivideDecimal7[8] = {0x5A, 0xA5, 0x05, 0x82, 0x11, 0x00, 0x00, 0x00};
    int dividedecimal7 = static_cast<int>(rounded_value7);   
    DivideDecimal7[6] = highByte(dividedecimal7);
    DivideDecimal7[7] = lowByte(dividedecimal7);
    Serial2.write(DivideDecimal7, 8);
  }





}
float findLowestNonZero(float a, float b, float c) {
  float values[] = {a, b, c};
  int nonZeroCount = 0;
  for (int i = 0; i < 3; ++i) {
    if (values[i] != 0.0) {
      values[nonZeroCount] = values[i];
      ++nonZeroCount;
    }
  }
  if (nonZeroCount > 0) {
    float lowest = values[0];
    for (int i = 1; i < nonZeroCount; ++i) {
      if (values[i] < lowest) {
        lowest = values[i];
      }
    }
    return lowest;
  } else {
    return 0.0;
  }
}
void setup() {
  pinMode(DRY_SELECTOR, INPUT_PULLUP);
  pinMode(WET_SELECTOR, INPUT);
  pinMode(INBRED_SELECTOR, INPUT_PULLUP);
  pinMode(HYBRID_SELECTOR, INPUT);
  pinMode(LIGHT_SELECTOR, INPUT_PULLUP);
  pinMode(MEDIUM_SELECTOR, INPUT_PULLUP);
  pinMode(HEAVY_SELECTOR, INPUT_PULLUP);
  pinMode(PRINT_BUTTON, INPUT_PULLUP);

  // Serial initialization
  Serial.begin(9600);
  Serial1.begin(4800);
  // Dwin Serial initialization
  Serial2.begin(115200);
  // NPK Sensor initialization
  // Serial1.begin(4800);
  mySerial.begin(9600);
  // Printer initialization
  printer.begin();

  // SD Card initialization
  if (SD.begin(chipSelect)) {
    Serial.println("SD card initialized successfully.");
  } else {
    Serial.println("SD card initialization failed. Check your connections.");
    return;
  }
  


}

void loop() {
  // put your main code here, to run repeatedly:
  dwinListen();
  // if (loopCounter > maxIterations){
  //   while (true) {
  //   }
  // }
  // delay(10000);
}
void npkSense(){
  byte queryData[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
  byte receivedData[19];
  float nit_val, phos_val, potas_val, ph_val, ec_val, mois_val;
  Serial1.write(queryData, sizeof(queryData));  // Send the query data to the NPK sensor
  delay(1000);  // Wait for 1 second
  int lightSwitchState = digitalRead(LIGHT_SELECTOR);
  int mediumSwitchState = digitalRead(MEDIUM_SELECTOR);
  int heavySwitchState = digitalRead(HEAVY_SELECTOR);
  // Serial.print(lightSwitchState);
  // Serial.print(mediumSwitchState);
  // Serial.print(heavySwitchState);
  button_selector_season = digitalRead(DRY_SELECTOR);
  button_selector_variety = digitalRead(INBRED_SELECTOR);
  button_selector_texture = digitalRead(LIGHT_SELECTOR);
  if (Serial1.available() >= sizeof(receivedData)) {   // Check if there are enough bytes available to read
    Serial1.readBytes(receivedData, sizeof(receivedData));  // Read the received data into the receivedData array
    // Parse and print the received data in decimal format
    unsigned int soilHumidity = (receivedData[3] << 8) | receivedData[4];
    // unsigned int soilTemperature = (receivedData[5] << 8) | receivedData[6];
    unsigned int soilConductivity = (receivedData[7] << 8) | receivedData[8];
    unsigned int soilPH = (receivedData[9] << 8) | receivedData[10];
    unsigned int nitrogen = (receivedData[11] << 8) | receivedData[12];
    unsigned int phosphorus = (receivedData[13] << 8) | receivedData[14];
    unsigned int potassium = (receivedData[15] << 8) | receivedData[16];
    
    moisture = (soilHumidity / 10.0) * 0.5175;
    ec = (soilConductivity / 1000.0) * 2.8621; 
    pH = (soilPH / 10.0) * 0.7556;
    nitro = (nitrogen / 10.0) * 0.89;
    phos = phosphorus * 0.1653;
    potas =  (potassium  / 39.0983) * 0.047;

    int n = static_cast<int>(nitro * 100);  
    int ps = static_cast<int>(phos * 100);  
    int k = static_cast<int>(potas * 100);
    int ph = static_cast<int>(pH * 100);
    int e = static_cast<int>(ec * 100);
    int m = static_cast<int>(moisture * 100);


    /*------Send Data to Display------*/

    Nitro_Dwin[6] = highByte(n);
    Nitro_Dwin[7] = lowByte(n);
    Serial2.write(Nitro_Dwin, 8);
    
    Phos_Dwin[6] = highByte(ps);
    Phos_Dwin[7] = lowByte(ps);
    Serial2.write(Phos_Dwin, 8);

    Potas_Dwin[6] = highByte(k);
    Potas_Dwin[7] = lowByte(k);
    Serial2.write(Potas_Dwin, 8);
    
    PH_Dwin[6] = highByte(ph);
    PH_Dwin[7] = lowByte(ph);
    Serial2.write(PH_Dwin, 8);
    
    EC_Dwin[6] = highByte(e);
    EC_Dwin[7] = lowByte(e);
    Serial2.write(EC_Dwin, 8);
    
    Moist_Dwin[6] = highByte(m);
    Moist_Dwin[7] = lowByte(m);
    Serial2.write(Moist_Dwin, 8);

    // Serial.println(nitro);
    // Serial.println(phos);
    // Serial.println(potas);
    // Serial.println(ec);
    // Serial.println(pH);
    // Serial.println(moisture);
    // Serial.println(season);
    // Serial.println(texture);
    // Serial.println(variety);
    if (season == "WET" && texture == "LIGHT" && variety == "HYBRID"){
      hybrid_nitrogen_lws(nitro);
      phosphorus_(pH,phos);
      potassium_(potas);
      soil_ph(pH);
      electrical_conductivity(ec);
      moisture_(moisture);
      nutrient_reco(nit_both,phos_both,potas_both);
    }
    else if (season == "WET" && texture == "MEDIUM" && variety == "HYBRID") {
      hybrid_nitrogen_mws(nitro);
      phosphorus_(pH,phos);
      potassium_(potas);
      soil_ph(pH);
      electrical_conductivity(ec);
      moisture_(moisture); 
      nutrient_reco(nit_both,phos_both,potas_both);
    } else if (season == "WET" && texture == "HEAVY" && variety == "HYBRID"){
      hybrid_nitrogen_hws(nitro);
      phosphorus_(pH,phos);
      potassium_(potas);
      soil_ph(pH);
      electrical_conductivity(ec);
      moisture_(moisture);

      nutrient_reco(nit_both,phos_both,potas_both);
    } else if (season == "DRY" && texture == "LIGHT" && variety == "HYBRID"){
      hybrid_nitrogen_lds(nitro);
      phosphorus_(pH,phos);
      potassium_(potas);
      soil_ph(pH);
      electrical_conductivity(ec);
      moisture_(moisture);
      nutrient_reco(nit_both,phos_both,potas_both);
    } else if (season == "DRY" && texture == "MEDIUM" && variety == "HYBRID"){
      hybrid_nitrogen_mds(nitro);
      phosphorus_(pH,phos);
      potassium_(potas);
      soil_ph(pH);
      electrical_conductivity(ec);
      moisture_(moisture); 
      nutrient_reco(nit_both,phos_both,potas_both);
    } else if (season == "DRY" && texture == "HEAVY" && variety == "HYBRID"){
      hybrid_nitrogen_hds(nitro);
      phosphorus_(pH,phos);
      potassium_(potas);
      soil_ph(pH);
      electrical_conductivity(ec);
      moisture_(moisture);
      nutrient_reco(nit_both,phos_both,potas_both);
    } else if (season == "WET" && texture == "LIGHT" && variety == "INBRED"){
      inbred_nitrogen_lws(nitro);
      phosphorus_(pH,phos);
      potassium_(potas);
      soil_ph(pH);
      electrical_conductivity(ec);
      moisture_(moisture);
      nutrient_reco(nit_both,phos_both,potas_both);
    } else if (season == "WET" && texture == "MEDIUM" && variety == "INBRED"){
      inbred_nitrogen_mws(nitro);
      phosphorus_(pH,phos);
      potassium_(potas);
      soil_ph(pH);
      electrical_conductivity(ec);
      moisture_(moisture);  
      nutrient_reco(nit_both,phos_both,potas_both);
    } else if (season == "WET" && texture == "HEAVY" && variety == "INBRED"){
      inbred_nitrogen_hws(nitro);
      phosphorus_(pH,phos);
      potassium_(potas);
      soil_ph(pH);
      electrical_conductivity(ec);
      moisture_(moisture);
      nutrient_reco(nit_both,phos_both,potas_both);
    } else if (season == "DRY" && texture == "LIGHT" && variety == "INBRED"){
      inbred_nitrogen_lds(nitro);
      phosphorus_(pH,phos);
      potassium_(potas);
      soil_ph(pH);
      electrical_conductivity(ec);
      moisture_(moisture);
      nutrient_reco(nit_both,phos_both,potas_both);
    } else if (season == "DRY" && texture == "MEDIUM" && variety == "INBRED"){
      inbred_nitrogen_mds(nitro);
      phosphorus_(pH,phos);
      potassium_(potas);
      soil_ph(pH);
      electrical_conductivity(ec);
      moisture_(moisture);    
      nutrient_reco(nit_both,phos_both,potas_both);
    } else if (season == "DRY" && texture == "HEAVY" && variety == "INBRED"){
      inbred_nitrogen_hds(nitro);
      phosphorus_(pH,phos);
      potassium_(potas);
      soil_ph(pH);
      electrical_conductivity(ec);
      moisture_(moisture);
      nutrient_reco(nit_both,phos_both,potas_both);
    }
    splitting(nit_both,phos_both,potas_both);

  }
  retrieveCounterFromEEPROM();
  // Log data
  logData();
}
void restartDWIN(){
  // Structure   {Initial} {len}  {Wo/p}(Add} {DataLen} {Command}
  byte open[] = {0x5A, 0xA5, 0x06, 0x82, 0x04, 0x55, 0xAA, 0x5A, 0xA5};
  Serial2.write(open, sizeof(open)); 
  delay(50);
  while (Serial2.available()) {
      Serial2.read();
  }
}
void printData(String prefix){
  buttonState  = digitalRead(PRINT_BUTTON);
  if (buttonState != oldButtonState &&
    buttonState == HIGH)
  {
    printer.justify('C');
    printer.setSize('L');
    printer.boldOn();
    printer.println(F("S E N S O I L"));
    printer.boldOff();

    printer.println();

    printer.setSize('S');
    printer.print("No: ");
    printer.println(prefix);
    printer.println();

    printer.setSize('S');
    printer.print("Date: ");
    printer.print(__DATE__);
    printer.println();

    // Separator

    printer.println();
    printer.setSize('S');
    printer.justify('C');
    printWithString(printer, "SEASON: ",season);
    printWithString(printer, "TEXTURE: ", texture);
    printWithString(printer, "VARIETY: ", variety);

    printer.println();
    printer.justify('L');
    printer.println(F("   PARAMETER     VALUE"));

    printer.justify('L');
    printWithSpace(printer, "   Nitrogen-------",nitro, "%");
    printWithSpace(printer, "   Phosphorus-----", phos, "ppm");
    printWithSpace(printer, "   Potassium------", potas, "cmol/kg");
    printWithSpace(printer, "   pH-------------", pH, " ");
    printWithSpace(printer, "   EC-------------", ec, "mS/cm");
    printWithSpace(printer, "   Moisture-------", moisture, "%");


    printer.println("--------------------------------");
    printer.justify('C'); // center the image
    printer.boldOn();
    printer.println(F("NUTRIENT RECOMMENDATION"));
    printer.boldOff();
    printer.setSize('S');
    printer.println(F("kg/ha"));

    printer.println();

    printer.setSize('S');
    printer.justify('C');
    printWithInt(printer, "N:",nit_both);
    printWithInt(printer, "P:", phos_both);
    printWithInt(printer, "K:", potas_both);

    printer.println("--------------------------------");
    printer.justify('C'); 
    printer.boldOn();
    printer.println(F("FERTILIZER RECOMMENDATION"));
    printer.boldOff();
    printer.setSize('S');
    printer.println(F("(per ha)"));

    printer.println();
    printer.justify('C');
    printer.print("Basal Application: ");
    const char *basal = "10-20 bags, Organic Fertilizer";

    // Print centered text
    printer.println();
    printer.justify('C');
    printCenteredText(printer, basal);

    printer.println();

    const char *topdressing1 = "1st TopDressing(5-7 DAT):";
    printCenteredText(printer, topdressing1);
    printer.println();
    const char *topdressing2 = "2nd TopDressing(20-24 DAT):";
    printCenteredText(printer, topdressing2);
    printer.println();
    const char *topdressing3 = "3rd TopDressing(30-35 DAT):";
    printCenteredText(printer, topdressing3);
    printer.println();

    printer.justify('C');
    printer.setSize('S');
    printer.print("It is recommended to test your");
    printer.println();
    printer.print("soil every planting season for");
    printer.println();
    printer.print("efficient farming. Thank You!");
    printer.println();
    printer.boldOn();
    printer.println();
    printer.println("Produced by: SENSOIL @2024");
    printer.boldOff();
    printer.feed(3); 
    
    printer.sleep();      // Tell printer to sleep
    printer.wake();       // MUST wake() before printing again, even if reset
    printer.setDefault(); // Restore printer to defaults
  }
  oldButtonState = buttonState;
}
void switches(){
  int lightSwitchState = digitalRead(LIGHT_SELECTOR);
  int mediumSwitchState = digitalRead(MEDIUM_SELECTOR);
  int heavySwitchState = digitalRead(HEAVY_SELECTOR);
  // Serial.print(lightSwitchState);
  // Serial.print(mediumSwitchState);
  // Serial.print(heavySwitchState);
  button_selector_season = digitalRead(DRY_SELECTOR);
  button_selector_variety = digitalRead(INBRED_SELECTOR);
  button_selector_texture = digitalRead(LIGHT_SELECTOR);
  if (button_selector_variety == 1) {
    // light 110
    if (heavySwitchState == 0 && button_selector_season == 1){
      unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x57,0x45,0x54};
      unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4C,0x49,0x47,0x48,0x54,0x2e};
      unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x48,0x59,0x42,0x52,0x49,0x44};
      season = "WET";
      texture = "LIGHT";
      variety = "HYBRID";
      Serial2.write(Season_Dwin,9);
      Serial2.write(Texture_Dwin,12);
      Serial2.write(Variety_Dwin,12);
    // medium 111
    } else if (lightSwitchState == 1 && mediumSwitchState && heavySwitchState && button_selector_season == 1){
      unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x57,0x45,0x54};
      unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4D,0x45,0x44,0x49,0x55,0x4D};
      unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x48,0x59,0x42,0x52,0x49,0x44};
      season = "WET";
      texture = "MEDIUM";
      variety = "HYBRID";
      Serial2.write(Season_Dwin,9);
      Serial2.write(Texture_Dwin,12);
      Serial2.write(Variety_Dwin,12);     
    // heavy 011
    }else if (lightSwitchState == 0 && button_selector_season == 1){
      unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x57,0x45,0x54};
      unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x48,0x45,0x41,0x56,0x59,0x2e};
      unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x48,0x59,0x42,0x52,0x49,0x44};
      season = "WET";
      texture = "HEAVY";
      variety = "HYBRID";
      Serial2.write(Season_Dwin,9);
      Serial2.write(Texture_Dwin,12);
      Serial2.write(Variety_Dwin,12);
    }
    // Dry Season
    // light 110
    if (heavySwitchState == 0 && button_selector_season == 0){
      unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x44,0x52,0x59};
      unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4C,0x49,0x47,0x48,0x54,0x2e};
      unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x48,0x59,0x42,0x52,0x49,0x44};
      season = "DRY";
      texture = "LIGHT";
      variety = "HYBRID";
      Serial2.write(Season_Dwin,9);
      Serial2.write(Texture_Dwin,12);
      Serial2.write(Variety_Dwin,12);
    // medium 111
    } else if (lightSwitchState == 1 && mediumSwitchState && heavySwitchState && button_selector_season == 0){
      unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x44,0x52,0x59};
      unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4D,0x45,0x44,0x49,0x55,0x4D};
      unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x48,0x59,0x42,0x52,0x49,0x44};
      season = "DRY";
      texture = "MEDIUM";
      variety = "HYBRID";
      Serial2.write(Season_Dwin,9);
      Serial2.write(Texture_Dwin,12);
      Serial2.write(Variety_Dwin,12);  

    // heavy 011
    } else if (lightSwitchState == 0 && button_selector_season == 0){
      unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x44,0x52,0x59};
      unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x48,0x45,0x41,0x56,0x59,0x2e};
      unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x48,0x59,0x42,0x52,0x49,0x44};
      season = "DRY";
      texture = "HEAVY";
      variety = "HYBRID";
      Serial2.write(Season_Dwin,9);
      Serial2.write(Texture_Dwin,12);
      Serial2.write(Variety_Dwin,12);
    }

  
  } else {
    // if (button_selector_season == 1){
    //   Serial.print("WET INBRED");
    //   Serial.println();
    // }else if (button_selector_season == 0){
    //   Serial.print("DRY INBRED");
    //   Serial.println();
    // }
    // light 110
    if (heavySwitchState == 0 && button_selector_season == 1){
      unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x57,0x45,0x54};
      unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4C,0x49,0x47,0x48,0x54,0x2e};
      unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x49,0x4e,0x42,0x52,0x45,0x44};
      season = "WET";
      texture = "LIGHT";
      variety = "INBRED";
      Serial2.write(Season_Dwin,9);
      Serial2.write(Texture_Dwin,12);
      Serial2.write(Variety_Dwin,12);
    // medium 111
    } else if (lightSwitchState == 1 && mediumSwitchState && heavySwitchState && button_selector_season == 1){
      unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x57,0x45,0x54};
      unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4D,0x45,0x44,0x49,0x55,0x4D};
      unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x49,0x4e,0x42,0x52,0x45,0x44};
      season = "WET";
      texture = "MEDIUM";
      variety = "INBRED";
      Serial2.write(Season_Dwin,9);
      Serial2.write(Texture_Dwin,12);
      Serial2.write(Variety_Dwin,12);
    // heavy 011
    } else if (lightSwitchState == 0 && button_selector_season == 1){
      unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x57,0x45,0x54};
      unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x48,0x45,0x41,0x56,0x59,0x2e};
      unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x49,0x4e,0x42,0x52,0x45,0x44};        
      season = "WET";
      texture = "HEAVY";
      variety = "INBRED";
      Serial2.write(Season_Dwin,9);
      Serial2.write(Texture_Dwin,12);
      Serial2.write(Variety_Dwin,12);
    }
    // Dry Season
    // light 110
    if (heavySwitchState == 0 && button_selector_season == 0){
      unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x44,0x52,0x59};
      unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4C,0x49,0x47,0x48,0x54,0x2e};
      unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x49,0x4e,0x42,0x52,0x45,0x44};
      season = "DRY";
      texture = "LIGHT";
      variety = "INBRED";
      Serial2.write(Season_Dwin,9);
      Serial2.write(Texture_Dwin,12);
      Serial2.write(Variety_Dwin,12);
    // medium 111
    } else if (lightSwitchState == 1 && mediumSwitchState && heavySwitchState && button_selector_season == 0){
      unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x44,0x52,0x59};
      unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4D,0x45,0x44,0x49,0x55,0x4D};
      unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x49,0x4e,0x42,0x52,0x45,0x44};
      season = "DRY";
      texture = "MEDIUM";
      variety = "INBRED";
      Serial2.write(Season_Dwin,9);
      Serial2.write(Texture_Dwin,13);
      Serial2.write(Variety_Dwin,12);  
    // heavy 011
    } else if (lightSwitchState == 0 && button_selector_season == 0){
      unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x44,0x52,0x59};
      unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x48,0x45,0x41,0x56,0x59,0x2e};
      unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x49,0x4e,0x42,0x52,0x45,0x44};
      season = "DRY";
      texture = "HEAVY";
      variety = "INBRED";
      Serial2.write(Season_Dwin,9);
      Serial2.write(Texture_Dwin,12);
      Serial2.write(Variety_Dwin,12);
    }
  }
}