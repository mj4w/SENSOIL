#include <SoftwareSerial.h>
// Date
#include <RTClib.h>
RTC_DS1307 rtc;
// SWITCH
#define WET_SELECTOR 6
#define DRY_SELECTOR 7
#define INBRED_SELECTOR 8
#define HYBRID_SELECTOR 9
#define LIGHT_SELECTOR 10
#define MEDIUM_SELECTOR 11 
#define HEAVY_SELECTOR 12

/* Mini Thermal Printer 
  MODEL: CSN-4AL 
  NOTE! USE 9V 2AMP POWER SUPPLY 
*/
#include "Adafruit_Thermal.h"
/* BAUDRATE

TO CHECK THE BAUDRATE, KINDLY CHECK THE SAMPLE PRINT TEST IN THERMAL PRINTER
*/
#define BAUDRATE 9600

SoftwareSerial mySerial1(2, 3);  // TX, RX
SoftwareSerial mySerial2(6,5); // TX, RX

Adafruit_Thermal printer(&mySerial2);
String nit_value,phos_value,potas_value,ph_value,soil_salinity_class,mois_value;
int nit_both,phos_both,potas_both;
int button_selector_season = 0;
int button_selector_variety = 0;
int button_selector_texture = 0;
void setup() {
  Serial.begin(9600);
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__),F(__TIME__)));
  // NPK Sensor
  mySerial1.begin(4800);
  // Mini Thermal Printer
  mySerial2.begin(9600);

  printer.begin();
  pinMode(DRY_SELECTOR, INPUT_PULLUP);
  pinMode(WET_SELECTOR, INPUT);
  pinMode(INBRED_SELECTOR, INPUT_PULLUP);
  pinMode(HYBRID_SELECTOR, INPUT);
  pinMode(LIGHT_SELECTOR, INPUT_PULLUP);
  pinMode(MEDIUM_SELECTOR, INPUT_PULLUP);
  pinMode(HEAVY_SELECTOR, INPUT_PULLUP);
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
  if (potas >= 0 && potas <= 0.0190){
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
void loop() {
  byte queryData[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x07, 0x04, 0x08};
  byte receivedData[19];
  String season,variety,texture;
  float nit_val, phos_val, potas_val, ph_val, ec_val, mois_val;
  int loopCounter = 15;  // Counter to keep track of loop iterations with data
  const int maxIterations = 20;  
  mySerial1.write(queryData, sizeof(queryData));  // Send the query data to the NPK sensor
  delay(1000);  // Wait for 1 second
  int lightSwitchState = digitalRead(LIGHT_SELECTOR);
  int mediumSwitchState = digitalRead(MEDIUM_SELECTOR);
  int heavySwitchState = digitalRead(HEAVY_SELECTOR);

  button_selector_season = digitalRead(DRY_SELECTOR);
  button_selector_variety = digitalRead(INBRED_SELECTOR);
  button_selector_texture = digitalRead(LIGHT_SELECTOR);
  
  if (button_selector_variety == 1) {
      // light 110
      if (heavySwitchState == 0 && button_selector_season == 1){
        season = "WET";
        texture = "LIGHT";
        variety = "HYBRID";
        Serial.print("Light & Wet HYBRID");
        Serial.println();
      // medium 111
      } else if (lightSwitchState == 1 && mediumSwitchState && heavySwitchState && button_selector_season == 1){
        season = "WET";
        texture = "MEDIUM";
        variety = "HYBRID";
        Serial.print("Medium & Wet HYBRID");
        Serial.println();       
      // heavy 011
      }else if (lightSwitchState == 0 && button_selector_season == 1){
        season = "WET";
        texture = "HEAVY";
        variety = "HYBRID";
        Serial.print("Heavy & Wet HYBRID");
        Serial.println();
      }
      // Dry Season
      // light 110
      if (heavySwitchState == 0 && button_selector_season == 0){
        season = "DRY";
        texture = "LIGHT";
        variety = "HYBRID";
        Serial.print("Light & Dry HYBRID");
        Serial.println();
      // medium 111
      } else if (lightSwitchState == 1 && mediumSwitchState && heavySwitchState && button_selector_season == 0){
        season = "DRY";
        texture = "MEDIUM";
        variety = "HYBRID";
        Serial.print("Medium & Dry HYBRID");
        Serial.println();       
      // heavy 011
      } else if (lightSwitchState == 0 && button_selector_season == 0){
        season = "DRY";
        texture = "HEAVY";
        variety = "HYBRID";
        Serial.print("Heavy & Dry HYBRID");
        Serial.println();
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
        season = "WET";
        texture = "LIGHT";
        variety = "INBRED";
        Serial.print("Light & Wet INBRED");
        Serial.println();
      // medium 111
      } else if (lightSwitchState == 1 && mediumSwitchState && heavySwitchState && button_selector_season == 1){
        season = "WET";
        texture = "MEDIUM";
        variety = "INBRED";
        Serial.print("Medium & Wet INBRED");
        Serial.println();       
      // heavy 011
      } else if (lightSwitchState == 0 && button_selector_season == 1){
        season = "WET";
        texture = "HEAVY";
        variety = "INBRED";
        Serial.print("Heavy & Wet INBRED");
        Serial.println();
      }
      // Dry Season
      // light 110
      if (heavySwitchState == 0 && button_selector_season == 0){
        season = "DRY";
        texture = "LIGHT";
        variety = "INBRED";
        Serial.print("Light & Dry INBRED");
        Serial.println();
      // medium 111
      } else if (lightSwitchState == 1 && mediumSwitchState && heavySwitchState && button_selector_season == 0){
        season = "DRY";
        texture = "MEDIUM";
        variety = "INBRED";
        Serial.print("Medium & Dry INBRED");
        Serial.println();       
      // heavy 011
      } else if (lightSwitchState == 0 && button_selector_season == 0){
        season = "DRY";
        texture = "HEAVY";
        variety = "INBRED";
        Serial.print("Heavy & Dry INBRED");
        Serial.println();
      }


  }


  if (mySerial1.available() >= sizeof(receivedData)) {  // Check if there are enough bytes available to read
    mySerial1.readBytes(receivedData, sizeof(receivedData));  // Read the received data into the receivedData array
    // Parse and print the received data in decimal format
    unsigned int soilHumidity = (receivedData[3] << 8) | receivedData[4];
    // unsigned int soilTemperature = (receivedData[5] << 8) | receivedData[6];
    unsigned int soilConductivity = (receivedData[7] << 8) | receivedData[8];
    unsigned int soilPH = (receivedData[9] << 8) | receivedData[10];
    unsigned int nitrogen = (receivedData[11] << 8) | receivedData[12];
    unsigned int phosphorus = (receivedData[13] << 8) | receivedData[14];
    unsigned int potassium = (receivedData[15] << 8) | receivedData[16];

    
    float moisture = soilHumidity / 10.0;
    float ec = soilConductivity / 1000.0; 
    float pH = soilPH / 10.0;
    float nitro = nitrogen / 1000.0 * 10.0;
    float phos = phosphorus;
    float potas =  potassium / 1000.0 / 39.0983 * 100.0;

   // if (hybrid == 1){90
    // WET SEASON AREA
    //   if (light && wet_season){
    hybrid_nitrogen_lws(nitro);
    phosphorus_(pH,phos);
    potassium_(potas);
    soil_ph(pH);
    electrical_conductivity(ec);
    moisture_(moisture);
    //   }
    //else if (medium && wet_season){
    hybrid_nitrogen_mws(nitro);
    phosphorus_(pH,phos);
    potassium_(potas);
    soil_ph(pH);
    electrical_conductivity(ec);
    moisture_(moisture);
    //   }
    //else if (heavy && wet_season){
    hybrid_nitrogen_hws(nitro);
    phosphorus_(pH,phos);
    potassium_(potas);
    soil_ph(pH);
    electrical_conductivity(ec);
    moisture_(moisture);
    //   }

    // DRY SEASON AREA
    //   if (light && dry_season){
    hybrid_nitrogen_lds(nitro);
    phosphorus_(pH,phos);
    potassium_(potas);
    soil_ph(pH);
    electrical_conductivity(ec);
    moisture_(moisture);
    //   }
    //else if (medium && dry_season){
    hybrid_nitrogen_mds(nitro);
    phosphorus_(pH,phos);
    potassium_(potas);
    soil_ph(pH);
    electrical_conductivity(ec);
    moisture_(moisture);
    //   }
    //else if (heavy && dry_season){
    hybrid_nitrogen_hds(nitro);
    phosphorus_(pH,phos);
    potassium_(potas);
    soil_ph(pH);
    electrical_conductivity(ec);
    moisture_(moisture);
    //   }

    // }

    // if (inbred == 1){
    // WET SEASON AREA
    //   if (light && wet_season){
    inbred_nitrogen_lws(nitro);
    phosphorus_(pH,phos);
    potassium_(potas);
    soil_ph(pH);
    electrical_conductivity(ec);
    moisture_(moisture);
    //   }
    //else if (medium && wet_season){
    inbred_nitrogen_mws(nitro);
    phosphorus_(pH,phos);
    potassium_(potas);
    soil_ph(pH);
    electrical_conductivity(ec);
    moisture_(moisture);
    //   }
    //else if (heavy && wet_season){
    inbred_nitrogen_hws(nitro);
    phosphorus_(pH,phos);
    potassium_(potas);
    soil_ph(pH);
    electrical_conductivity(ec);
    moisture_(moisture);
    //   }

    // DRY SEASON AREA
    //   if (light && dry_season){
    inbred_nitrogen_lds(nitro);
    phosphorus_(pH,phos);
    potassium_(potas);
    soil_ph(pH);
    electrical_conductivity(ec);
    moisture_(moisture);
    //   }
    //else if (medium && dry_season){
    inbred_nitrogen_mds(nitro);
    phosphorus_(pH,phos);
    potassium_(potas);
    soil_ph(pH);
    electrical_conductivity(ec);
    moisture_(moisture);
    //   }
    //else if (heavy && dry_season){
    inbred_nitrogen_hds(nitro);
    phosphorus_(pH,phos);
    potassium_(potas);
    soil_ph(pH);
    electrical_conductivity(ec);
    moisture_(moisture);
    //   }

    // }
    

    //NUTRIENT REQUIREMENT

    // print nit_both, phos_both, potas_both

    //Splitting
    // 1st, 2nd, 3rd -> Application

    // first application
    // if wet season && dry season
    //   // do this
    //   nitro_split1 = nith_both * 0.30
    //   phos_split1 = phos_both

    //   if (potas_both < 45) {
    //     potas_split1 = potas_both
    //   }
    //   else{
    //     potas_split1 = potas_both * 0.50
    //   }
    // // second application
    // if wet season
    //   // do this
    //   nitro_split2 = nith_both * 0.30
    //   phos_split2 = 0
    //   potas_split2 = 0
    // else if dry season
    //   // do this
    //   nitro_split2 = nith_both * 0.20
    //   phos_split2 = 0
    //   potas_split2 = 0

    // // third application
    // if wet season
    //   // do this
    //   nitro_split3 = nith_both * 0.40
    //   phos_split3 = 0
    //   if (potas_both < 45) {
    //     potas_split3 = potas_both * 0.50
    //   }
    //   else{
    //     potas_split3 = potas_both
    //   }
    // else if dry season
    //   // do this
    //   nitro_split3 = nith_both * 0.50
    //   phos_split3 = 0
    //   if (potas_both < 45) {
    //     potas_split3 = potas_both * 0.50
    //   }
    //   else{
    //     potas_split3 = potas_both
    //   }

    // // display 
    // // ------------


    // // Filterizer Contains
    // // 1st Application
    // String value_fil;
    // if (nitro_split1 > 1 && phos_split1 > 1 && potas_split1 > 1){
    //     value_fil = "Complete, Triple 14";
    //     n_fil = 7.0;
    //     p_fil = 7.0;
    //     k_fil = 7.0;
    // }

    // else if (nitro_split1 > 1 && phos_split1 > 1 && potas_split1 < 1){
    //     value_fil = "Ammonium Phosphate";
    //     n_fil = 8.0;
    //     p_fil = 10.0;
    //     k_fil = 0.0;
    // }

    // else if (nitro_split1 < 1 && phos_split1 > 1 && potas_split1 < 1){
    //     value_fil = "Superphospate";
    //     n_fil = 0.0;
    //     p_fil = 10.0;
    //     k_fil = 0.0;
    // }

    // else if (nitro_split1 < 1 && phos_split1 < 1 && potas_split1 > 1){
    //     value_fil = "Muriate of Potash";
    //     n_fil = 0.0;
    //     p_fil = 0.0;
    //     k_fil = 30.0;
    // }

        
        
    // if (pH < 6.6) {
    //     if (nitro_split1 > 1 && phos_split1 < 1 && potas_split1 < 1){
    //         value_fil = "Urea";
    //         n_fil = 23.0;
    //         p_fil = 0.0;
    //         k_fil = 0.0;
    //     }

    // }

    // else {
    //     if (nitro_split1 > 1 && phos_split1 < 1 && potas_split1 < 1){
    //         value_fil = "Ammonium Sulfate";
    //         n_fil = 10.5;
    //         p_fil = 0.0;
    //         k_fil = 0.0;
    //     }

    // }



    //   // divide 
    //   float result_dividen1 = nitro_split1 / n_fil;
    //   float result_dividep1 = phos_split1 / p_fil;
    //   float result_dividek1 = potas_split1 / k_fil;

    //   float lowest_value1 = min(result_dividen1, min(result_dividep1, result_dividek1));
    //   // print value_fil, lowest_value1


    //   // multiply
    //   float result_multipn1 = lowest_value1 * n_fil;
    //   float result_multipp1 = lowest_value1 * p_fil;
    //   float result_multipk1 = lowest_value1 * k_fil;

    //   // minus the result
    //   float result_minusn1 = nitro_split1 - result_multipn1;
    //   float result_minusp1 = phos_split1 - result_multipp1;
    //   float result_minusk1 = potas_split1 - result_multipk1;
    //   float nonZeroValue = 0.0;

    //   if (result_minusn1 < 1 && result_minusp1 < 1 && result_multipk1 < 1){
    //     break;
    //   }

    //   if (result_minusn1 != 0.0) {
    //     nonZeroValue = result_minusn1
    //   } else if (result_minusp1 != 0.0) {
    //     nonZeroValue = result_minusp1
    //   } else if (result_multipk1 != 0.0) {
    //     nonZeroValue = result_minusk1
    //   }

    //   if (nonZeroValue != 0.0){
    //     String value_fil_;
    //     float divisor = 0.0;
    //     if (result_minusn1 > 1 && result_minusp1 > 1 && result_minusk1 > 1){
    //         value_fil_ = "Complete, Triple 14";
    //         n_fil = 7.0;
    //         p_fil = 7.0;
    //         k_fil = 7.0;
    //     }

    //     else if (result_minusn1 > 1 && result_minusp1 > 1 && result_minusk1 < 1){
    //         value_fil_ = "Ammonium Phosphate";
    //         n_fil = 8.0;
    //         p_fil = 10.0;
    //         k_fil = 0.0;
    //     }

    //     else if (result_minusn1 < 1 && result_minusp1 > 1 && result_minusk1 < 1){
    //         value_fil_ = "Superphospate";
    //         n_fil = 0.0;
    //         p_fil = 10.0;
    //         k_fil = 0.0;
    //         divisor = p_fil;
    //     }

    //     else if (result_minusn1 < 1 && result_minusp1 < 1 && result_minusk1 > 1){
    //         value_fil_ = "Muriate of Potash";
    //         n_fil = 0.0;
    //         p_fil = 0.0;
    //         k_fil = 30.0;
    //         divisor = k_fil;
    //     }

            
            
    //     if (pH < 6.6) {
    //         if (result_minusn1 > 1 && result_minusp1 < 1 && result_minusk1 < 1){
    //             value_fil_ = "Urea";
    //             n_fil = 23.0;
    //             p_fil = 0.0;
    //             k_fil = 0.0;
    //             divisor = n_fil;
    //         }

    //     }

    //     else {
    //         if (result_minusn1 > 1 && result_minusp1 < 1 && result_minusk1 < 1){
    //             value_fil_ = "Ammonium Sulfate";
    //             n_fil = 10.5;
    //             p_fil = 0.0;
    //             k_fil = 0.0;
    //             divisor = n_fil;
    //         }

    //     }
    //     float nonZeroResult = nonZeroValue / divisor;
    //     float result_lastn1 = result_minusn1 * nonZeroResult;
    //     float result_lastp1 = result_minusp1 * nonZeroResult;
    //     float result_lastk1 = result_minusk1 * nonZeroResult;

    //     // minus
    //     float result_last_minusn1 = result_multipn1 - result_lastn1;
    //     float result_last_minusp1 = result_multipp1 - result_lastp1;
    //     float result_last_minusk1 = result_multipk1 - result_lastk1;

    //     if (result_last_minusn1 < 1 && result_last_minusp1 < 1 && result_last_minusk1 < 1){
    //     break;
    //     }
    //     // print nonZeroResult & value_fil_
    //     int get_decimal_1 = int(nonZeroResult * 100) % 100;
    //     int divide2_decimal_1 = get_decimal_1 / 2;
    //     int get_number_1 = int(nonZeroResult);

    //     // get_number_ bags & divide2_decimal_1 kg value_fil_
    //   }

    //   int get_decimal1 = int(lowest_value1 * 100) % 100;
    //   int divide2_decimal1 = get_decimal_1 / 2;
    //   int get_number1 = int(lowest_value1);

      // get_number bags & divide2_decimal1 kg value_fil










        







    delay(3000);
    
    loopCounter++;

  }
  else {
    DateTime date_obj = rtc.now();
    // Center the image and print the header
    printer.justify('C');
    printer.setSize('L');
    printer.boldOn();
    printer.println(F("S E N S O I L"));
    printer.boldOff();

    printer.println();

    // Center the Test No.
    printer.justify('C');
    printer.setSize('S');
    printer.print("Test No. ");
    printer.print("001");
    printer.println();

    // Center the date
    printer.justify('C');
    printer.setSize('S');
    printer.print("Date: ");
    printer.print("May 10, 2024");
    printer.println();

    // Separator
    printer.justify('C');
    printer.setSize('S');
    printer.println(F("--------------------------------"));

    printer.setSize('S');

    printer.print("    SEASON:");
    printer.print(season);
    printer.println("    TEXTURE:");
    printer.print(texture);
    printer.println("    VARIETY:");
    printer.print(variety);
    printer.println();

    printer.justify('L');
    printer.println(F("     PARAMETER      VALUE"));

    float nitrogenValue = 1.97;
    float phosphorusValue = 12.00;
    float potassiumValue = 11.97;
    float pHValue = 2.34;
    float ecValue = 3.22;
    float moistureValue = 10.34;

    printer.justify('L');
    printWithSpace(printer, "     Nitrogen-------",nitrogenValue, "%");
    printWithSpace(printer, "     Phosphorus-----", phosphorusValue, "ppm");
    printWithSpace(printer, "     Potassium------", potassiumValue, "cmol/kg");
    printWithSpace(printer, "     pH-------------", pHValue, " ");
    printWithSpace(printer, "     EC-------------", ecValue, "mS/cm");
    printWithSpace(printer, "     Moisture-------", moistureValue, "%");


    printer.println(F("--------------------------------"));
    printer.justify('C'); // center the image
    printer.setSize('M');
    printer.boldOn();
    printer.println(F("NUTRIENT RECOMMENDATION"));
    printer.boldOff();
    printer.setSize('S');
    printer.println(F("kg"));


    printer.feed(3); // Move to the next line





    // printer.justify('C');
    // printer.setSize('S');
    // printer.boldOn();
    // printer.print("PARAMETERS-----------");
    // printer.print("VALUE");
    // printer.boldOff();
    // printer.println();

    // printer.justify('C');
    // printer.setSize('S');
    // printer.print("Nitrogen----------");
    // printer.print("1.97");
    // printer.print(" %");
    // printer.println();

    // printer.justify('C');
    // printer.setSize('S');
    // printer.print("Phosphorus----------");
    // printer.print("12.00");
    // printer.print(" ppm");
    // printer.println();

    // printer.justify('C');
    // printer.setSize('S');
    // printer.print("Potassium---------");
    // printer.print("11.97");
    // printer.print(" cmol/kg");
    // printer.println();



    printer.sleep();      // Tell printer to sleep
    printer.wake();       // MUST wake() before printing again, even if reset
    printer.setDefault(); // Restore printer to defaults
  }


  if (loopCounter > maxIterations){
    while (true) {
    }
  }

}
void printWithSpace(Adafruit_Thermal &printer, const char *parameter, float value, const char *unit) {
  printer.print(parameter);
  int spaces = 15 - strlen(parameter); // Adjust the number of spaces based on your layout
  for (int i = 0; i < spaces; i++) {
    printer.print(" ");
  }
  printer.print(value, 2); // Assuming you want to print values with 2 decimal places
  printer.print(unit);
  printer.println();
}

