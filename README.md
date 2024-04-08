> [!NOTE]  
> Arduino Mega used!

### Folder Structure
```bash
├── NPK
│   ├── A_printertest 
│   └── CoolTerm
│   ├── DWIN_SET
│   ├── Datalogger
│   └── eeprom_increment
│   ├── eepromdata
│   └── guideRestartDwin
│   └── UART Communication Software - SSCOM32
│   ├── sensoil
│   │   ├── MAIN CODE HERE
│   │   │ 
│   │   │  
│   │   │   
│   ├── formatSDCard.txt
│   └── ProductManual.docx
```
### Prototype Design
<a href="https://postimg.cc/McZPBCpx"><img src="https://i.postimg.cc/QM1R2sLB/SENSOIL-PROTOTYPE.png" alt="SENSOIL-PROTOTYPE" border="0" /></a>

### Schematic
<a href="https://postimg.cc/xqLv0s9P"><img src="https://i.postimg.cc/gcTNH551/SENSOIL-SCHEMATIC.png" alt="SENSOIL-SCHEMATIC" border="0" /></a>
# DEFINITION
* <strong>A_printertest</strong> - example code for printing data coming from sensor <br/>
* <strong>CoolTerm</strong> - application used to generate hexadecimal value to read if the sensor is working<br/>
* <strong>UART Communication Software - SSCOM32</strong> - it checks RX TX returning value is correct <br/>
* <strong>DWIN_SET</strong> - file needed to display in DWIN LCD <br/>
* <strong>Datalogger</strong> - for data logging module source code <br/>
* <strong>eeprom_increment</strong> - to get the increment number and save it <br/>
* <strong>guideRestartDwin</strong> - to restart dwin every changes made <br/>
* <strong>sensoil</strong> - main_code developed by your host. <br/>

# PRODUCT TYPE
* DWIN_LCD - DMG80480C070_04WTC [LINK](https://shopee.ph/☃Dwin-7-Inch-800x480-TFT-LCD-Display-HMI-touch-screen-Smart-Display-CTP-RTP-TTL-UART-Port-DMG80480-i.703180712.15960341033?sp_atk=a7181775-b1af-4ee6-9538-693fe30f52db&xptdk=a7181775-b1af-4ee6-9538-693fe30f52db)
* Switch Rotate Button [LINK](https://shopee.ph/9-24V-19mm-22mm-led-metal-rotary-push-button-brass-2-3-position-selector-Switch-Rotate-button-rotation-knob-metal-switch-i.277704323.13367910427?sp_atk=8fa4c536-8ebf-4b4d-be40-d163a9b767b8&xptdk=8fa4c536-8ebf-4b4d-be40-d163a9b767b8)
* Data Logger Module [LINK](https://shopee.ph/DIYMORE-Mini-Data-Logger-Module-Logging-Recorder-Shield-Ds1307-For-Arduino-Raspberry-Pi-i.173799909.5304357395?sp_atk=2d355ef6-40ac-4874-a585-230e3e9b2dbf&xptdk=2d355ef6-40ac-4874-a585-230e3e9b2dbf)
* 
# REFERENCE!
## CoolTerm
- https://karadev.net/basto/COOLTERM_191/CoolTermWin/CoolTerm%20Resources/HelpText.html
## Arduino Mini Thermal Printer / PNG Image to Byte Array
- https://mischianti.org/images-to-byte-array-online-converter-cpp-arduino/
## For Reference in Mini Thermal Printer -> Guide
- Install Library "Adafruit Thermal Printer"
https://forum.arduino.cc/t/printing-image-with-thermal-printer/864154/4
https://learn.adafruit.com/mini-thermal-receipt-printer/microcontroller
## GUIDE FOR DWIN_LCD
- DwinLCD Manual Guide
https://github.com/mj4w/NPK/blob/main/T5L_DGUSII-Application-Development-Guide-20220520.pdf
## Arduino Forum
- Ask Question relating in our device
https://forum.arduino.cc/

# TECHNOLOGY USED
- Arduino IDE
 https://www.arduino.cc/en/software/
