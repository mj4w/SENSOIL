#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>
#include <DS3231-RTC.h>
#include "Adafruit_Thermal.h"

const int chipSelect = 53; // Change this to the CS pin of your SD card module
const int maxFiles = 10; // Maximum number of files, adjust as needed

String fileNames[maxFiles]; // Array to store file names
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
float nitro,phos,potas,pH,ec,moisture;
int nit_both,phos_both,potas_both;
int nitro_split1,phos_split1,nitro_split2,phos_split2,nitro_split3,phos_split3,potas_split1,potas_split2,potas_split3;
int button_selector_season = 0;
int button_selector_variety = 0;
int button_selector_texture = 0;
int buttonState = 0;
int oldButtonState = LOW;

const unsigned int PRINT_BUTTON = 8;
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
        unsigned char data1[] = {0x5A,0xA5,0x10,0x82,0x31,0x00,0x64,0x61,0x74,0x61,0x31};
        Serial2.write(data1,11);
        Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
    } else if (i == 1 ) {
        unsigned char data2[] = {0x5A,0xA5,0x10,0x82,0x31,0x10,0x64,0x61,0x74,0x61,0x32};
        Serial2.write(data2,11);
        Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
    } else if (i == 2 ) {
        unsigned char data3[] = {0x5A,0xA5,0x10,0x82,0x31,0x20,0x64,0x61,0x74,0x61,0x33};
        Serial2.write(data3,11);
        Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
    } else if (i == 3 ) {
        unsigned char data4[] = {0x5A,0xA5,0x10,0x82,0x31,0x30,0x64,0x61,0x74,0x61,0x34};
        Serial2.write(data4,11);
        Serial.println("var" + String(i + 1) + ": " + fileNames[i]);
    }

  }


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
    Serial.println("Address "+ address +" Data "+String(dataVal));

    if (address == "1656"){
      deleteFile(filename);
      Serial.println(filename);
    }

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
// void readDataAndAssignVariables(String fileName) {
//   File dataFile = SD.open(fileName);

//   if (dataFile) {
//     Serial.println("Reading data from file: " + String(fileName));

//     // Read data from the file line by line and assign values to variables
//     while (dataFile.available()) {
//       String dataLine = dataFile.readStringUntil('\n');
//       parseAndAssignVariables(dataLine);
//     }

//     // Close the file
//     dataFile.close();

//     // Print the assigned values for verification
//     printAssignedValues();
//   } else {
//     Serial.println("Error opening file: " + String(fileName));
//   }
// }
String createFileName() {
  String fileName = "data_" + String(random(1000)) + ".csv";
  return fileName;
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
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19); 
    }
    else if (phos >= 6.1 && phos <= 10){
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19); 
    }
    else if (phos >= 10.1 && phos <= 15){
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19);
    }
    else if (phos >= 15.1 && phos <= 100){
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19); 
    }
    else if (phos > 100){
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x56,0x45,0x52,0x59,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19); 
    }
  }
  if (pH <= 5.5) {
    if (phos >= 0 && phos <= 2) {
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19); 
    }
    if (phos >= 2.1 && phos <= 6){
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19); 
    }
    if (phos >= 6.1 && phos <= 10){
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
      Serial2.write(Phosphorus_Label,19);
    }
    if (phos >= 10.1 && phos <= 75){
    unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Phosphorus_Label,19);  
    }
    else if (phos > 75) {
      unsigned char Phosphorus_Label[] = {0x5A,0xA5,0x10,0x82,0x21,0x00,0x56,0x45,0x52,0x59,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Potassium_Label[] = {0x5A,0xA5,0x10,0x82,0x23,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Potassium_Label,19); 
  }
  else if (potas >= 0.191 && potas <= 0.290){
    unsigned char Potassium_Label[] = {0x5A,0xA5,0x10,0x82,0x23,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Potassium_Label,19); 
  }
  else if (potas >= 0.291 && potas <= 0.385){
    unsigned char Potassium_Label[] = {0x5A,0xA5,0x10,0x82,0x23,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Potassium_Label,19);
  }
  else if (potas >= 0.386 && potas <= 1.000){
    unsigned char Potassium_Label[] = {0x5A,0xA5,0x10,0x82,0x23,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Potassium_Label,19); 
  }
  else if (potas >= 1.000){
    unsigned char Potassium_Label[] = {0x5A,0xA5,0x10,0x82,0x23,0x00,0x56,0x45,0x52,0x59,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Soil_ph[] = {0x5A,0xA5,0x10,0x82,0x15,0x00,0x4E,0x45,0x55,0x54,0x52,0x41,0x4C,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Soil_ph,19);
  }
  else if (pH >= 0 && pH < 7){
    unsigned char Soil_ph[] = {0x5A,0xA5,0x10,0x82,0x15,0x00,0x41,0x43,0x49,0x44,0x49,0x43,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Soil_ph,19);
  }
  else if (pH >= 7 && pH <= 14){
    unsigned char Soil_ph[] = {0x5A,0xA5,0x10,0x82,0x15,0x00,0x41,0x4C,0x4B,0x41,0x4C,0x49,0x4E,0x45,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Saline[] = {0x5A,0xA5,0x10,0x82,0x16,0x00,0x4E,0x4F,0x4E,0x53,0x41,0x4C,0x49,0x4E,0x45,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Saline,19);
  }
  else if (ec > 2.1 && ec <= 4) {
    unsigned char Saline[] = {0x5A,0xA5,0x10,0x82,0x16,0x00,0x53,0x4C,0x49,0x47,0x48,0x54,0x4C,0x59,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Saline,19);
  } 
  else if (ec > 4.1 && ec <= 8) {
    unsigned char Saline[] = {0x5A,0xA5,0x10,0x82,0x16,0x00,0x4D,0x4F,0x44,0x45,0x52,0x41,0x54,0x45,0x4C,0x59,0x2D,0x2D,0x2D};
    Serial2.write(Saline,19);
  }
  else if (ec > 8.1 &&  ec < 16) {
    unsigned char Saline[] = {0x5A,0xA5,0x10,0x82,0x16,0x00,0x53,0x45,0x56,0x45,0x52,0x45,0x4C,0x59,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Moisture[] = {0x5A,0xA5,0x10,0x82,0x17,0x00,0x56,0x45,0x52,0x59,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Moisture,19);
  }
  else if (moisture > 15){
    unsigned char Moisture[] = {0x5A,0xA5,0x10,0x82,0x17,0x00,0x4D,0x4F,0x49,0x53,0x54,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Moisture,19);
  }
}


// light nitro -> WET SEASON
void hybrid_nitrogen_lws(float nitro){

  if (nitro <= 2.0) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 100;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 80;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 60;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 90;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 70;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 50;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 80;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 60;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 40;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 120;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 100;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 80;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19);  
  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 90;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 70;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 100;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 80;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 60;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 80;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 60;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 40;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 70;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 50;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 30;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 60;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 40;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 20;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 90;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 70;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19);
    nit_both = 50;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 80;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 60;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 40;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
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
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 70;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x4C,0x4F,0x57,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 50;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x4D,0x4F,0x44,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
    Serial2.write(Nitrogen_Label_Dwin,19); 
    nit_both = 30;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
    unsigned char Nitrogen_Label_Dwin[] = {0x5A,0xA5,0x10,0x82,0x22,0x00,0x48,0x49,0x47,0x48,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D};
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
  float n_fil_last,p_fil_last,k_fil_last;
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

  else if (nitro_split1 < 1 && phos_split1 < 1 && potas_split1 > 1){
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
      if (nitro_split1 > 1 && phos_split1 < 1 && potas_split1 < 1){
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
      if (nitro_split1 > 1 && phos_split1 < 1 && potas_split1 < 1){
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
    float result_dividen1 = nitro_split1 / n_fil;
    float result_dividep1 = phos_split1 / p_fil;
    float result_dividek1 = potas_split1 / k_fil;
    // Serial.println(result_dividen1);
    // Serial.println(result_dividep1);
    // Serial.println(result_dividek1);
    float lowest_value1 = min(result_dividen1, min(result_dividep1, result_dividek1));
    // print value_fil, lowest_value1
    // Serial.println(n_fil);
    // Serial.println(p_fil);
    // Serial.println(k_fil);
    unsigned char Bags[8] = {0x5A, 0xA5, 0x05, 0x82, 0x70, 0x00, 0x00, 0x00};
    int bags_val = static_cast<int>(lowest_value1);  
    Bags[6] = highByte(bags_val);
    Bags[7] = lowByte(bags_val);
    Serial2.write(Bags, 8);
    
    int get = int(lowest_value1 * 100) % 100;
    float dividedBy2 = float(get) / 2;
    int roundedValue = int((dividedBy2 + 2.5) / 5) * 5;
    // Serial.println(roundedValue);
    unsigned char Kilograms[8] = {0x5A, 0xA5, 0x05, 0x82, 0x71, 0x00, 0x00, 0x00};
    int kg_val = static_cast<int>(roundedValue);  
    Kilograms[6] = highByte(kg_val);
    Kilograms[7] = lowByte(kg_val);
    Serial2.write(Kilograms, 8);


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
    float nonZeroValue = 0.0;

    if (result_minusn1 < 1 && result_minusp1 < 1 && result_multipk1 < 1){
        Serial.println("End");
    }

    if (result_minusn1 != 0.0) {
      nonZeroValue = result_minusn1;
    } else if (result_minusp1 != 0.0) {
      nonZeroValue = result_minusp1;
    } else if (result_multipk1 != 0.0) {
      nonZeroValue = result_minusk1;
    }

    if (nonZeroValue != 0.0){
      String value_fil_;
      float divisor = 0.0;
      if (result_minusn1 > 1 && result_minusp1 > 1 && result_minusk1 > 1){
          value_fil_ = "Complete, Triple 14";
          n_fil_last = 7;
          p_fil_last = 7;
          k_fil_last = 7;
      }

      else if (result_minusn1 > 1 && result_minusp1 > 1 && result_minusk1 < 1){
          value_fil_ = "Ammonium Phosphate";
          n_fil_last = 8;
          p_fil_last = 10;
          k_fil_last = 0;
      }

      else if (result_minusn1 < 1 && result_minusp1 > 1 && result_minusk1 < 1){
          value_fil_ = "Superphospate";
          n_fil_last = 0;
          p_fil_last = 10;
          k_fil_last = 0;
          divisor = p_fil_last;
      }

      else if (result_minusn1 < 1 && result_minusp1 < 1 && result_minusk1 > 1){
          value_fil_ = "Muriate of Potash";
          n_fil_last = 0;
          p_fil_last = 0;
          k_fil_last = 30;
          divisor = k_fil_last;
      }

          
          
      if (pH < 6.6) {
          if (result_minusn1 > 1 && result_minusp1 < 1 && result_minusk1 < 1){
              value_fil_ = "Urea";
              n_fil_last = 23;
              p_fil_last = 0;
              k_fil_last = 0;
              divisor = n_fil_last;
          }

      }

      else {
          if (result_minusn1 > 1 && result_minusp1 < 1 && result_minusk1 < 1){
              value_fil_ = "Ammonium Sulfate";
              n_fil_last = 10.5;
              p_fil_last = 0;
              k_fil_last = 0;
              divisor = n_fil_last;
          }

      }
      float nonZeroResult = nonZeroValue / divisor;
      float result_lastn1 = result_minusn1 * nonZeroResult;
      float result_lastp1 = result_minusp1 * nonZeroResult;
      float result_lastk1 = result_minusk1 * nonZeroResult;

      // minus
      float result_last_minusn1 = result_multipn1 - result_lastn1;
      float result_last_minusp1 = result_multipp1 - result_lastp1;
      float result_last_minusk1 = result_multipk1 - result_lastk1;

      if (result_last_minusn1 < 1 && result_last_minusp1 < 1 && result_last_minusk1 < 1){
        Serial.println("End");
      }
      // print nonZeroResult & value_fil_
      int get_decimal_1 = int(nonZeroResult * 100) % 100;
      int divide2_decimal_1 = get_decimal_1 / 2;
      int get_number_1 = int(nonZeroResult);

      // get_number_ bags & divide2_decimal_1 kg value_fil_
    }

    int get_decimal1 = int(lowest_value1 * 100) % 100;
    int divide2_decimal1 = get_decimal1 / 2;
    int get_number1 = int(lowest_value1);
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
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card initialization failed. Check your connections.");
    return;
  }
  
  Serial.println("SD card initialized successfully.");

}

void loop() {
  // put your main code here, to run repeatedly:
  byte queryData[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
  byte receivedData[19];
  int loopCounter = 15;  // Counter to keep track of loop iterations with data
  const int maxIterations = 20;  
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

    if (button_selector_variety == 1) {
      // light 110
      if (heavySwitchState == 0 && button_selector_season == 1){
        unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x57,0x45,0x54};
        unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4C,0x49,0x47,0x48,0x54,0x2e};
        unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x48,0x59,0x42,0x52,0x49,0x44};
        season = "WET";
        texture = "LIGHT";
        variety = "HYBRID";

        hybrid_nitrogen_lws(nitro);
        phosphorus_(pH,phos);
        potassium_(potas);
        soil_ph(pH);
        electrical_conductivity(ec);
        moisture_(moisture);

        nutrient_reco(nit_both,phos_both,potas_both);

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
        hybrid_nitrogen_mws(nitro);
        phosphorus_(pH,phos);
        potassium_(potas);
        soil_ph(pH);
        electrical_conductivity(ec);
        moisture_(moisture); 

        nutrient_reco(nit_both,phos_both,potas_both);

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

        hybrid_nitrogen_hws(nitro);
        phosphorus_(pH,phos);
        potassium_(potas);
        soil_ph(pH);
        electrical_conductivity(ec);
        moisture_(moisture);

        nutrient_reco(nit_both,phos_both,potas_both);

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

        hybrid_nitrogen_lds(nitro);
        phosphorus_(pH,phos);
        potassium_(potas);
        soil_ph(pH);
        electrical_conductivity(ec);
        moisture_(moisture);

        nutrient_reco(nit_both,phos_both,potas_both);

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

        hybrid_nitrogen_mds(nitro);
        phosphorus_(pH,phos);
        potassium_(potas);
        soil_ph(pH);
        electrical_conductivity(ec);
        moisture_(moisture); 

        nutrient_reco(nit_both,phos_both,potas_both);

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

        hybrid_nitrogen_hds(nitro);
        phosphorus_(pH,phos);
        potassium_(potas);
        soil_ph(pH);
        electrical_conductivity(ec);
        moisture_(moisture);
        
        nutrient_reco(nit_both,phos_both,potas_both);

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

        inbred_nitrogen_lws(nitro);
        phosphorus_(pH,phos);
        potassium_(potas);
        soil_ph(pH);
        electrical_conductivity(ec);
        moisture_(moisture);

        nutrient_reco(nit_both,phos_both,potas_both);

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

        inbred_nitrogen_mws(nitro);
        phosphorus_(pH,phos);
        potassium_(potas);
        soil_ph(pH);
        electrical_conductivity(ec);
        moisture_(moisture);  

        nutrient_reco(nit_both,phos_both,potas_both);

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

        inbred_nitrogen_hws(nitro);
        phosphorus_(pH,phos);
        potassium_(potas);
        soil_ph(pH);
        electrical_conductivity(ec);
        moisture_(moisture);

        nutrient_reco(nit_both,phos_both,potas_both);

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
        // unsigned char Soil_ph[] = {0x5A,0xA5,0x11,0x82,0x15,0x00,0x4E,0x75,0x65,0x74,0x72,0x61,0x6C,0x2D};
        // Serial2.write(Soil_ph,14);
        inbred_nitrogen_lds(nitro);
        phosphorus_(pH,phos);
        potassium_(potas);
        soil_ph(pH);
        electrical_conductivity(ec);
        moisture_(moisture);

        nutrient_reco(nit_both,phos_both,potas_both);

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

        inbred_nitrogen_mds(nitro);
        phosphorus_(pH,phos);
        potassium_(potas);
        soil_ph(pH);
        electrical_conductivity(ec);
        moisture_(moisture);    

        nutrient_reco(nit_both,phos_both,potas_both);

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

        inbred_nitrogen_hds(nitro);
        phosphorus_(pH,phos);
        potassium_(potas);
        soil_ph(pH);
        electrical_conductivity(ec);
        moisture_(moisture);

        nutrient_reco(nit_both,phos_both,potas_both);

        Serial2.write(Season_Dwin,9);
        Serial2.write(Texture_Dwin,12);
        Serial2.write(Variety_Dwin,12);
      }
    }
    splitting(nit_both,phos_both,potas_both);
    loopCounter++;
  }
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

    // printer.setSize('S');
    // printer.print("Test No. ");
    // printer.print("001");
    // printer.println();

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
  if (loopCounter > maxIterations){
    while (true) {
    }
  }
  // logData();
  // delay(10000);
  dwinListen();
}