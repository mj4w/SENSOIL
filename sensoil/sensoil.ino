#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>
#include <DS3231-RTC.h>
#include "Adafruit_Thermal.h"

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

SoftwareSerial mySerial1(10,11);
SoftwareSerial dwinSerial(19,18);
SoftwareSerial mySerial2(14,15); // TX, RX
Adafruit_Thermal printer(&mySerial2);
String nit_value,phos_value,potas_value,ph_value,soil_salinity_class,mois_value;
String season,variety,texture;
float nitro,phos,potas,pH,ec,moisture;
int nit_both,phos_both,potas_both;
int nitro_split1,phos_split1,nitro_split2,phos_split2,nitro_split3,phos_split3,potas_split1,potas_split2,potas_split3;
int button_selector_season = 0;
int button_selector_variety = 0;
int button_selector_texture = 0;
int buttonState = 0;
int oldButtonState = LOW;

const unsigned int PRINT_BUTTON = 13;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  // Dwin Serial
  dwinSerial.begin(9600);
  // NPK Sensor
  mySerial1.begin(4800);
  mySerial2.begin(9600);
  printer.begin();
  pinMode(DRY_SELECTOR, INPUT_PULLUP);
  pinMode(WET_SELECTOR, INPUT);
  pinMode(INBRED_SELECTOR, INPUT_PULLUP);
  pinMode(HYBRID_SELECTOR, INPUT);
  pinMode(LIGHT_SELECTOR, INPUT_PULLUP);
  pinMode(MEDIUM_SELECTOR, INPUT_PULLUP);
  pinMode(HEAVY_SELECTOR, INPUT_PULLUP);
  pinMode(PRINT_BUTTON, INPUT_PULLUP); 

}

void loop() {
  // put your main code here, to run repeatedly:
  byte queryData[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
  byte receivedData[19];
  int loopCounter = 15;  // Counter to keep track of loop iterations with data
  const int maxIterations = 20;  
  float nit_val, phos_val, potas_val, ph_val, ec_val, mois_val;
  mySerial1.write(queryData, sizeof(queryData));  // Send the query data to the NPK sensor
  delay(1000);  // Wait for 1 second
  int lightSwitchState = digitalRead(LIGHT_SELECTOR);
  int mediumSwitchState = digitalRead(MEDIUM_SELECTOR);
  int heavySwitchState = digitalRead(HEAVY_SELECTOR);
  button_selector_season = digitalRead(DRY_SELECTOR);
  button_selector_variety = digitalRead(INBRED_SELECTOR);
  button_selector_texture = digitalRead(LIGHT_SELECTOR);
  if (mySerial1.available() >= sizeof(receivedData)) {   // Check if there are enough bytes available to read
    mySerial1.readBytes(receivedData, sizeof(receivedData));  // Read the received data into the receivedData array
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
    dwinSerial.write(Nitro_Dwin, 8);
    
    Phos_Dwin[6] = highByte(ps);
    Phos_Dwin[7] = lowByte(ps);
    dwinSerial.write(Phos_Dwin, 8);

    Potas_Dwin[6] = highByte(k);
    Potas_Dwin[7] = lowByte(k);
    dwinSerial.write(Potas_Dwin, 8);
    
    PH_Dwin[6] = highByte(ph);
    PH_Dwin[7] = lowByte(ph);
    dwinSerial.write(PH_Dwin, 8);
    
    EC_Dwin[6] = highByte(e);
    EC_Dwin[7] = lowByte(e);
    dwinSerial.write(EC_Dwin, 8);
    
    Moist_Dwin[6] = highByte(m);
    Moist_Dwin[7] = lowByte(m);
    dwinSerial.write(Moist_Dwin, 8);

    if (button_selector_variety == 1) {
      // light 110
      if (heavySwitchState == 0 && button_selector_season == 1){
        unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x57,0x65,0x74};
        unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4C,0x69,0x67,0x68,0x74,0x2e};
        unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x48,0x79,0x62,0x72,0x69,0x64};
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

        dwinSerial.write(Season_Dwin,9);
        dwinSerial.write(Texture_Dwin,11);
        dwinSerial.write(Variety_Dwin,12);
        delay(100);
      // medium 111
      } else if (lightSwitchState == 1 && mediumSwitchState && heavySwitchState && button_selector_season == 1){
        unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x57,0x65,0x74};
        unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4D,0x65,0x64,0x69,0x75,0x6D};
        unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x48,0x79,0x62,0x72,0x69,0x64};
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

        dwinSerial.write(Season_Dwin,9);
        dwinSerial.write(Texture_Dwin,12);
        dwinSerial.write(Variety_Dwin,12);  
        delay(100);   
      // heavy 011
      }else if (lightSwitchState == 0 && button_selector_season == 1){
        unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x57,0x65,0x74};
        unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x48,0x65,0x61,0x76,0x79,0x2e};
        unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x48,0x79,0x62,0x72,0x69,0x64};
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

        dwinSerial.write(Season_Dwin,9);
        dwinSerial.write(Texture_Dwin,12);
        dwinSerial.write(Variety_Dwin,12);
        delay(100);
      }
      // Dry Season
      // light 110
      if (heavySwitchState == 0 && button_selector_season == 0){
        unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x44,0x72,0x79};
        unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4C,0x69,0x67,0x68,0x74,0x2e};
        unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x48,0x79,0x62,0x72,0x69,0x64};
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

        dwinSerial.write(Season_Dwin,9);
        dwinSerial.write(Texture_Dwin,12);
        dwinSerial.write(Variety_Dwin,12);
        delay(100);
      // medium 111
      } else if (lightSwitchState == 1 && mediumSwitchState && heavySwitchState && button_selector_season == 0){
        unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x44,0x72,0x79};
        unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4D,0x65,0x64,0x69,0x75,0x6D};
        unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x48,0x79,0x62,0x72,0x69,0x64};
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

        dwinSerial.write(Season_Dwin,9);
        dwinSerial.write(Texture_Dwin,12);
        dwinSerial.write(Variety_Dwin,12);   
        delay(100);

      // heavy 011
      } else if (lightSwitchState == 0 && button_selector_season == 0){
        unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x44,0x72,0x79};
        unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x48,0x65,0x61,0x76,0x79,0x2e};
        unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x48,0x79,0x62,0x72,0x69,0x64};
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

        dwinSerial.write(Season_Dwin,9);
        dwinSerial.write(Texture_Dwin,12);
        dwinSerial.write(Variety_Dwin,12);
        delay(100);
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
        unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x57,0x65,0x74};
        unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4C,0x69,0x67,0x68,0x74,0x2e};
        unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x49,0x6e,0x62,0x72,0x65,0x64};
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

        dwinSerial.write(Season_Dwin,9);
        dwinSerial.write(Texture_Dwin,12);
        dwinSerial.write(Variety_Dwin,12);
        delay(100);
      // medium 111
      } else if (lightSwitchState == 1 && mediumSwitchState && heavySwitchState && button_selector_season == 1){
        unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x57,0x65,0x74};
        unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4D,0x65,0x64,0x69,0x75,0x6D};
        unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x49,0x6e,0x62,0x72,0x65,0x64};
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

        dwinSerial.write(Season_Dwin,9);
        dwinSerial.write(Texture_Dwin,12);
        dwinSerial.write(Variety_Dwin,12);
        delay(100);
      // heavy 011
      } else if (lightSwitchState == 0 && button_selector_season == 1){
        unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x57,0x65,0x74};
        unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x48,0x65,0x61,0x76,0x79,0x2e};
        unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x49,0x6e,0x62,0x72,0x65,0x64};        
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

        dwinSerial.write(Season_Dwin,9);
        dwinSerial.write(Texture_Dwin,12);
        dwinSerial.write(Variety_Dwin,12);
        delay(100);
      }
      // Dry Season
      // light 110
      if (heavySwitchState == 0 && button_selector_season == 0){
        unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x44,0x72,0x79};
        unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4C,0x69,0x67,0x68,0x74,0x2e};
        unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x49,0x6e,0x62,0x72,0x65,0x64};
        season = "DRY";
        texture = "LIGHT";
        variety = "INBRED";

        inbred_nitrogen_lds(nitro);
        phosphorus_(pH,phos);
        potassium_(potas);
        soil_ph(pH);
        electrical_conductivity(ec);
        moisture_(moisture);

        nutrient_reco(nit_both,phos_both,potas_both);

        dwinSerial.write(Season_Dwin,9);
        dwinSerial.write(Texture_Dwin,12);
        dwinSerial.write(Variety_Dwin,12);
        delay(100);
      // medium 111
      } else if (lightSwitchState == 1 && mediumSwitchState && heavySwitchState && button_selector_season == 0){
        unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x44,0x72,0x79};
        unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x4D,0x65,0x64,0x69,0x75,0x6D};
        unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x49,0x6e,0x62,0x72,0x65,0x64};
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

        dwinSerial.write(Season_Dwin,9);
        dwinSerial.write(Texture_Dwin,13);
        dwinSerial.write(Variety_Dwin,12);  
        delay(100);
      // heavy 011
      } else if (lightSwitchState == 0 && button_selector_season == 0){
        unsigned char Season_Dwin[] = {0x5A,0xA5,0x06,0x82,0x20,0x00,0x44,0x72,0x79};
        unsigned char Texture_Dwin[] = {0x5A,0xA5,0x09,0x82,0x10,0x00,0x48,0x65,0x61,0x76,0x79,0x2e};
        unsigned char Variety_Dwin[] = {0x5A,0xA5,0x09,0x82,0x30,0x00,0x49,0x6e,0x62,0x72,0x65,0x64};
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

        dwinSerial.write(Season_Dwin,9);
        dwinSerial.write(Texture_Dwin,12);
        dwinSerial.write(Variety_Dwin,12);
        delay(100);
      }
    }
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
  dwinSerial.write(Nit_Both_Dwin, 8);
  
  Phos_Both_Dwin[6] = highByte(pb);
  Phos_Both_Dwin[7] = lowByte(pb);
  dwinSerial.write(Phos_Both_Dwin, 8);

  Potas_Both_Dwin[6] = highByte(pob);
  Potas_Both_Dwin[7] = lowByte(pob);
  dwinSerial.write(Potas_Both_Dwin, 8);
}


// light nitro -> WET SEASON
void hybrid_nitrogen_lws(float nitro){

  if (nitro <= 2.0) {
    nit_both = 100;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    nit_both = 80;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    nit_both = 60;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
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
    nit_both = 90;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    nit_both = 70;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    nit_both = 50;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
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
    nit_both = 80;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    nit_both = 60;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    nit_both = 40;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
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
      phos_both=7;
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
}
// DRY SEASON
void hybrid_nitrogen_lds(float nitro){

  if (nitro <= 2.0) {
    nit_both = 120;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    nit_both = 100;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    nit_both = 80;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
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
  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    nit_both = 90;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    nit_both = 70;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
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
    nit_both = 100;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    nit_both = 80;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    nit_both = 60;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
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
    nit_both = 80;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    nit_both = 60;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    nit_both = 40;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
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
    nit_both = 70;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    nit_both = 50;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    nit_both = 30;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
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
    nit_both = 60;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    nit_both = 40;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    nit_both = 20;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
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
    nit_both = 90;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    nit_both = 70;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    nit_both = 50;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
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
    nit_both = 80;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    nit_both = 60;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    nit_both = 40;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
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
    nit_both = 70;
    nit_value = "LOW";

  }
  else if (nitro >= 2.1 && nitro <= 3.5) {
    nit_both = 50;
    nit_value = "MODERATELY LOW";

  }
  else if (nitro >= 3.6 && nitro <= 4.5) {
    nit_both = 30;
    nit_value = "MODERATELY HIGH";
  }
  else if (nitro >= 4.6){
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
