#include <SoftwareSerial.h>

String RETURN_BUTTON = "185";
// Address of Button

SoftwareSerial Serial2(10, 11); // RX, TX


void dwinListen(){
  
  int startAdd = 00;
  int endAdd = 00;
  int dataVal = 0;
  String address;
  
  while (Serial2.available()) {
      int inhex = Serial2.read();

      if( inhex == 90 || inhex == 165){
        continue;
      }
      
      for (int i=1; i<=inhex; i++) {
        while(!Serial2.available()); 
        int incomingByte = Serial2.read();
      //  Serial.print(" "+String(incomingByte));
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

    if ( address == RETURN_BUTTON && dataVal == 1 ){
      sendRandomNo();
    }
  }
}

void sendRandomNo(){
  byte randomNo = random(11, 220);

  byte open[] = {0x5A, 0xA5, 0x05, 0x82, 0x12, 0x01, 0x00, randomNo};
  Serial2.write(open, sizeof(open)); 
  delay(50);

  while (Serial2.available()) {
      int inhex = Serial2.read();
      if( inhex == 90 || inhex == 165){
        continue;
      }
  }
  
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

void pageSwitch(byte pPageNO){
 
  byte open[] = {0x5A, 0xA5, 0x07, 0x82, 0x00, 0x84, 0x5A, 0x01, 0x00, pPageNO};
  Serial2.write(open, sizeof(open)); 
  delay(10);

  String response = "";
  while (Serial2.available()) {
      int inhex = Serial2.read();
      if( inhex == 90 || inhex == 165){
        continue;
      }
      for(int i = 1; i < inhex; i++){
         if(i == 1){
          Serial2.read();
         }
        // response.concat(String(char(dwinListen.read())));
      }
  }
  Serial.println("Page Switch : " + String(pPageNO) + " -> " + response);
  response = "";
  delay(10);
}

void setup() {
  
  Serial.begin(115200);
  Serial2.begin(115200);
  Serial.println(" DWIN Arduino Button Demo ");
  Serial.println("Lets Begin");
  
}

void loop() { 
  dwinListen();
}
