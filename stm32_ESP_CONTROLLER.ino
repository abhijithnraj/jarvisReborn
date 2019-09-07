//Project SSAL IoT Core
//Project SSAL IoT Helper [Merge]
/* Author Abhiram Shibu, Preet Patel
 * Previous Author Abhijith N Raj
 * Copyright (c) TeamDestroyer Projects 2018
 * Copyright (c) 2019 SSAL
 * Copyright (c) 2019 BrainNet Technlogies
 * Copyright (c) 2019 TUXFourm
 * For queries goto https://tuxforum.com
 */
//Includes
#include<string.h>
/* STM32 */
// #include<SoftwareSerial.h>

#include<EEPROM.h>
//#include<TimerOne.h> - Timer Depriciation, interfering with i2c.
#include<MemoryFree.h>
/* STM32 */
#include<Wire_slave.h>
#include <LiquidCrystal_I2C.h>

/* STM32 */
// #include<avr/wdt.h>

//Convience Definitions
#define True 1
#define False 0
#define ESPTX 11
#define ESPRX 12
#define ESPBAUD 19200
#define BAUD 2000000
#define DEBUG true
#define VERSION 2.1
#define INSPECT 180619
#define PageBaseZero 0x801F800 //PageBase0
#define PageBaseOne 0x801FC00  //PageBase1

/*------Pin Maps---------
*-----------------------*
|        0 -> PB2       |
|        1 -> PB2       |
|        2 -> PB3       |
|        3 -> PB4       |
|        4 -> PB5       |
|        5 -> PB10      |
|        6 -> PB11      |
|        7 -> PB8       |
|        8 -> PB9       |
|        9 -> PC14      |
|        10 -> PC15     |
|        11 -> PB2      |
|        12 -> PB2      |
|        13 -> PC13     |
*-----------------------*
---End pinMaps DOC ------*/

//LCD VARIABLES
    LiquidCrystal_I2C lcd(0x27, 16, 2);
    int count=0;
    int count_wait=0;
    int pinNow=0;
    long time1=millis();
    long time2=millis();
//END LCD VARIABLES
    
bool checkESP=true;
int counter=0;
int counter_disconnect=0;
bool pins[16];    // Pin stats are stored here
float temp;
/** EEPROM BEGIN **/
bool PROGMEM EEPROMFormat=False;
void EEPROMinit(){
    EEPROM.PageBase0=0x800F000;
    EEPROM.PageBase1=0x800F400;
    if(EEPROMFormat==True){
        EEPROM.format();
        EEPROMFormat=False;
    }
}
byte encodeByteArray(bool * pin,int offset){
  byte val=0;
  for(int i=0;i<16;i++){
      val|=pin[i+offset]<<i;
  }
  return val;
}
void decodeIntAndRestore(bool * pin,byte val,int offset){
  byte temp;
  for(int i=0;i<16;i++){
    temp=val&(1<<i);
    if(temp==(1<<i)){
      pin[i+offset]=1;
    }
    else{
      pin[i+offset]=0;
    }
  }
  Serial.print("Restore:");
  for(int i=0;i<16;i++){
  Serial.print(pin[i]);    
 }
 Serial.println("");
}

void dumpToEEPROM(){
    byte val=encodeByteArray(pins,2);
    EEPROM.write(0,val);
    Serial.print(F("Write location:"));
}
void loadFromEEPROM(){
    byte val;
    int loc=0;
    val=EEPROM.read(loc);
    decodeIntAndRestore(pins,val,2);
}
/** END EEPROM **/

/* Serial Communication Handlers */
String dataESP,dataSerial;
String readSerial(){          //Data from serial Save and return
  dataSerial=Serial.readString();
  dataSerial.reserve(20);
  return dataSerial;
}
String readESP(){            //Data from ESP save and return
  /* STM32 */
  dataESP=Serial1.readString();
  dataESP.reserve(100);
  return dataESP;
}

/* Depricated because Arduino fucked up and idk sol */
// String sendDataESP(int id,String data){  
//   Serial.println(F("Entered"));
//   String temp="AT+CIPSEND=";//<link ID>,<length>
//   temp.concat(id);
//   temp.concat(F(","));
//   temp.concat(data.length()+2);
//   temp.concat(F("\r\n"));
//   writeESP(temp);
//   delay(100);
//   String inputData=readESP();
//   if(inputData.indexOf(F("OK")>-1)){
//     data.concat("\r\n");
//     writeESP(data);
//   }
//    delay(100);
//    readESP();
//    Serial.println(F("Exited"));
// }

// WatchDog Stuff
/* -----------------------*
 * Setup Watch dog timer  *
 * Watch Dog should reset *
 * CPU on overflow of     *
 * timer. Can be reset by *
 * WDTCSR function        *
 * -----------------------*/
/* STM32 */
// void setupWatchDog(){
//     wdt_reset();
//     MCUSR|=_BV(WDRF);
//     WDTCSR = _BV(WDCE) | _BV(WDE);
//     WDTCSR = _BV(WDE) | _BV(WDCE) | _BV(WDP0) |_BV(WDP3);
// }
// END WatchDog Stuff

//Wire Stuff
void initWire(){
    Wire.begin();
}
bool checkLCD(){
    Serial.println("Begining transmission");
    Wire.beginTransmission(39);
    Serial.println("Ending transmission");
    if(Wire.endTransmission()==0){
        return true;
    }
    else{
        return false;
    }
}
void initLCD(){
    Serial.println("Check lcd calling");
    if(checkLCD()){
        Serial.println("Starting LCD");
        lcd.begin();
        lcd.print("Welcome to SSAL");
        lcd.setCursor(0,1);
        lcd.print("Loading...");
    }
}
String pinData(){
    String temp;
    temp.reserve(20);
    temp=String(pinNow);
    if(pins[pinNow]==true){
        temp+=" is on";
    }
    else{
        temp+=" is off";
    }
    if((millis()-time2)>500){
        time2=millis();
        if(pinNow++==13){
            pinNow=0;
        }
        return temp;
    }
}
String assembleMessage(){
    String MSG;
//    main_message.reserve(100);
	MSG=F("SSAL System UI  Status:");
	MSG+=F("Active");
	MSG+=F(", IoT Core:");
	MSG+=F("Active");
// 	main_message+=F(", Temp:");
// 	main_message+=String(temp)+F("C");
	MSG+=F(" ");
    return MSG;
}
void updateLCD(){
	if((millis()-time1)>300){ //Scroll and set if update time exceeds..
		time1=millis();
		String main_message=assembleMessage();
		String temp_message;
        temp_message.reserve(100);
		temp_message=String(main_message);
		String t=temp_message.substring(0,count);
		temp_message.remove(0,count);
		temp_message=temp_message+t;
		lcd.setCursor(0,0);
		lcd.print(temp_message.substring(0,16));
		if(count==0&&count_wait<10){
			count_wait++;
		}
		else{
			count_wait=0;
			count++;
			if(count==main_message.length()){
				count=0;
			}
			//Serial.println(main_message);
		}
	}
	lcd.setCursor(0,1);
	lcd.print(pinData()+"  ");
}
// END WIRE STUFF

void writeESP(String data){           //String to ESP
  /* STM32 */
  Serial1.print(data);
}
void writeSerial(String data){        //String to Serial
  Serial.print(data);
}
void passThrough(){                    // Serial -> ESP, ESP -> Serial
  if(Serial.available()){
    writeESP(readSerial());
  }
  /* STM32 */
  if(Serial1.available()){
    writeSerial(readESP());
  }
}
void serialClear(){                     // Clear extra data
  /* STM32 */
  Serial1.flush();
  Serial.flush();
}
void echoSerial(){                      // Test function Serial->Serial
  if(Serial.available()){
    Serial.println(Serial.readString());
  }
}

/* ESP initallizer */
void initializeESP(){
    int count=0;
    mux:
    writeESP(F("AT+CWMODE?\r\n"));
    delay(200);
    String data;
    data.reserve(100);
    data=readESP();
    if(data.indexOf("CWMODE:1")==-1){
        writeESP(F("AT+CWMODE=1\r\n"));
        Serial.println(F("Setting ap mode")); //Need to be removed..
        delay(200);
        Serial.println(readESP());
        count++;
        if(count==10){
            writeESP(F("AT+RST\r\n"));
            Serial.println(F("Reset ESP"));
            delay(200);
            Serial.println(readESP());
            count=0;
        }
        goto mux;
    }
    writeESP(F("AT+CIPMUX=1\r\n"));   //Set mux to 1 thereby allowing multi connection
    delay(200);
    data =readESP();
    /* REST ESP ON UNKNOWN STATE */
    if(data.indexOf(F("ERROR"))>-1){
      if(data.indexOf(F("builded"))>-1){
        counter++;
        if(counter==3){
          writeESP(F("AT+RST\r\n"));
          serialClear();
        }
        goto mux;
      }
      else{
//         Serial.print("Link Active");
      }
    }
    writeESP(F("AT+CIPSERVER=1,23\r\n"));  //Start server
    delay(200);
    Serial.print(F("Data:"));
    data =readESP();
    Serial.println(data);
    /* REST ESP ON UNKNOWN STATE */
    if(data.indexOf(F("ERROR"))>-1){
      if(data.indexOf(F("builded"))>-1){
        counter++;
        if(counter==3){
          writeESP(F("AT+RST\r\n"));
          serialClear();
        }
        goto mux;
      }
      else{
        Serial.print(F("Link Active"));
      }
    }
    counter=0;
}
bool led=False;

/* Interrupt Service Routine for legacy timer one, ESP reset check routine */
void CUSTOM_ISR(void){
  if(led){
    led=False;
    digitalWrite(pinmap(13),led);
  }
  else{
    led=True;
    digitalWrite(pinmap(13),led);
  }
  checkESP=true;
}

/* Interrupt initialization code for above ISR */ // Depricated!
// void initializeInterrupt(){
//   pinMode(13,1);
//   Timer1.initialize(10000000); //Heart Beat ( ESP refresh signal.. )
//   Timer1.attachInterrupt(CUSTOM_ISR); 
// }

/* pinmap definition */
/* 
 * Standard Map for pins from stm32 to arduino
 * PA0  0
 * PA1  1
 * PA2  2
 * PA3  3
 * PA4  4
 * PA5  5
 * PA6  6
 * PA7  7
 * PA8  8
 * PA9  9
 * PA10 10
 * PA11 11
 * PA12 12
 * PA13 13
 * PA14 14
 * PA15 15
 * PB0  16
 * PB1  17
 * PB2  18
 * PB3  19
 * PB4  20
 * PB5  21
 * PB6  22
 * PB7  23
 * PB8  24  
 * PB9  25
 * PB10 26
 * PB11 27
 * PB12 28
 * PB13 29
 * PB14 30
 * PB15 31
 * PC0  32
 * PC1  33
 * PC2  34
 * PC3  35 
 * PC4  36
 * PC5  37
 * PC6  38
 * PC7  39
 * PC8  40
 * PC9  41
 * PC10 42
 * PC11 43
 * PC12 44
 * PC13 45
 * PC14 46
 * PC15 47
 */
int pinmap(int x){
    short pin[14]; // No std cpp libs in arduino, implementing map from scratch
    pin[0]=PB2;  // PB2 Boot 1 jumper.. Too much in need for pins.
    pin[1]=PB2;  // PB2
    pin[2]=PB3;  // PB3
    pin[3]=PB4;  // PB4
    pin[4]=PB5;  // PB5
    pin[5]=PB10;  // PB6
    pin[6]=PB11;  // PB7
    pin[7]=PB8;  // PB8
    pin[8]=PB9;  // PB9
    pin[9]=PB14; // PC14
    pin[10]=PC15;// PC15
    pin[11]=PB2; // PB2 
    pin[12]=PB2; // PB2
    pin[13]=PC13; // PC13 // LED
    // PB10 and PB11 is used for i2c
    return pin[x];
}
/* END pinmap */

/* pin initialization */
void pinInit(){
    for(int i=0;i<=13;i++){
        pinMode(pinmap(i),OUTPUT);
    }
    for(int i=0;i<=13;i++){
        if(pins[i]==1){
            digitalWrite(pinmap(i),HIGH);
        }
        else{
            digitalWrite(pinmap(i),LOW);
        }
    }
}
/* END pin initialization */


void setup() {
  initWire();
  initLCD();
  EEPROMinit();
  loadFromEEPROM();
  pinInit();
  if(DEBUG){
  Serial.begin(BAUD); //Serial to debugger 0,1
  }
  /* STM32 */
  Serial1.begin(ESPBAUD); //Serial to ESP mainly 11,12
  Serial.setTimeout(10); // Set string read timeout, without this readString is slow
  Serial1.setTimeout(100);   // Reduce rx tx timeout, we dont have decades to wait.
  initializeESP();
  Serial.println(F("Welcome to SSAL IoT Core"));
  Serial.print(F("freeMemory()="));
  Serial.println(freeMemory());  // Print free memory ocationally  // Can interrupt
  /* STM32 */
//   setupWatchDog();
}
long time=millis();
void loop() {
  updateLCD();
  //ISR is now replaced by millis watchdog
  if((millis()-time)>60000){ // Run timer every 5 seconds
    CUSTOM_ISR();  //Call legacy ISR
    time=millis(); //Reset virtual timer 
  }
  if(checkESP){
      /* ESP RESET/SERVER CHECK */
    writeESP(F("AT+CIPMUX?\r\n"));  //Check MUX status, for server to run it should be 1
    delay(300);
    String data;
    data.reserve(100);
    data=readESP();
//     Serial.print("Interrpt:");
//     Serial.println(data);
    if(data.indexOf(F("+CIPMUX:1"))==-1){  // not 1 make it 1
//       Serial.println("ESP RESET DETECTED!");
      initializeESP();                  //Reinitiallize there by making mux 1
      Serial.println(F("Re-initallized ESP"));
    }
    dataESP="";
    Serial.print(F("Free RAM = "));
    Serial.println(freeMemory());        //Print free ram periodically
    checkESP=false;
    
    /* ESP CONNECTION CHECK */
    writeESP(F("AT+CIPSTATUS\r\n"));
    delay(300);
    data = readESP();
    Serial.print(F("Hotspot data:"));
    Serial.println(data);
    if(data.indexOf(F(":5"))>-1){
        counter_disconnect++;
        if(counter_disconnect==3){
            counter_disconnect=0;
            writeESP(F("AT+CWMODE_CUR=2\r\n"));
            delay(300);
            readESP();
            writeESP(F("AT+CWDHCP_CUR=0,1\r\n"));
            delay(300);
            readESP();
            Serial.println(F("Hotspot initallized!"));
            long time=millis();
            while((millis()-time)<60000){
        //Wire.onRequest(sendData);
                passThrough();
                if(dataESP.indexOf(F("+IPD,"))>-1){     //Check if data from SSAL Core is available
                    dataESP.remove(0,dataESP.indexOf(F(","))+1); //Remove header
                    int id=dataESP.substring(0,1).toInt();  // Get id
                    Serial.print(F("ID :"));       
                    Serial.println(id);
                    dataESP.remove(0,dataESP.indexOf(":")+1);  //Remove id
                    Serial.print(F("Trimmed data:"));
                    Serial.println(dataESP);     //Now u have data\r\n left
                    if(id==0){                   // Allow only id 0
//                         if(dataESP.startsWith(F("writeAP="))){
//                             //Syntax wiriteAP=ssid,pass no quotes needed
//                             dataESP.remove(0,dataESP.indexOf(F("="))+1);
//                             String SSID = dataESP.substring(0,dataESP.indexOf(F(",")));
//                             String PASS = dataESP.substring(dataESP.indexOf(F(","))+1);
//                             Serial.print(F("SSID ="));
//                             writeESSID_EEPROM(SSID);
//                             Serial.println(readESSID_EEPROM());
//                             Serial.print(F("PASS ="));
//                             writePass_EEPROM(PASS);
//                             Serial.println(readPass_EEPROM());
//                         }
                    }
                }
            }
            writeESP(F("AT+RST\r\n"));
            delay(300);
            readESP();
        }
    }
  }
  passThrough();                         //Pass data from one serial to another
  if(!dataSerial.equals("")){            //Data from serial is available
    dataSerial="";
  }
  if(!dataESP.equals("")){               //Data from ESP available
    if(dataESP.indexOf(F("CONNECT"))>-1){   //Check if there is a connection and absorb next line
      delay(100);                        //Wait for some time
      readESP();
    }
    if(dataESP.indexOf(F("+IPD,"))>-1){     //Check if data from SSAL Core is available
      dataESP.remove(0,dataESP.indexOf(",")+1); //Remove header
      int id=dataESP.substring(0,1).toInt();  // Get id
      //Serial.print(F("ID :"));       
      //Serial.println(id);
      dataESP.remove(0,dataESP.indexOf(":")+1);  //Remove id
      //Serial.print(F("Trimmed data:"));
      //Serial.println(dataESP);     //Now u have data\r\n left
      if(id==0){                   // Allow only id 0
        if(dataESP.indexOf(F(" "))>-1){
        /* Find space split into pin and operation, then compile and send */
        int space = dataESP.indexOf(F(" "));
        int pin=dataESP.substring(0,space).toInt();
        bool operation=dataESP.substring(space,dataESP.length()-2).toInt();
        pinMode(pinmap(pin),OUTPUT);
        digitalWrite(pinmap(pin),operation); // do operation
        pins[pin]=operation;// update internal DB
        dumpToEEPROM();
        String pinString;
        pinString.reserve(10);
        pinString=dataESP.substring(0,dataESP.length()-2);
/* Depreciated for faster reply */
//         pinString=String(pin);
//         pinString.concat(F(" "));
//         if(operation){
//             pinString.concat(F("on"));
//         }
//         else{
//             pinString.concat(F("off"));
//         }
        /* Send assembled reply */
        
        //SendData - Because ardunio dont like functions
        String temp="AT+CIPSEND=";//<link ID>,<length>
        temp.concat(id);
        temp.concat(F(","));
        temp.concat(pinString.length()+2);
        temp.concat(F("\r\n"));
        writeESP(temp);
        delay(100);
        String inputData=readESP();
        if(inputData.indexOf(F("OK"))>-1){
            pinString.concat(F("\r\n"));
            writeESP(pinString);
        }
        delay(100);
        readESP();
        //End SendData
        
        
        }
        else{
        /* Here pin status is retrived and send to the SSAL Core */
        int pin=dataESP.substring(0,dataESP.length()-2).toInt();
        
        //SendData - Because ardunio dont like functions
        String pinString;
        pinString.reserve(10);
        pinString=String(pins[pin]);
        String temp="AT+CIPSEND=";//<link ID>,<length>
        temp.reserve(50);
        temp.concat(id);
        temp.concat(",");
        temp.concat(pinString.length()+2);
        temp.concat(F("\r\n"));
        writeESP(temp);
        delay(100);
        String inputData=readESP();
        if(inputData.indexOf("OK")>-1){
            pinString.concat("\r\n");
            writeESP(pinString);
        }
        delay(100);
        readESP();
        //End SendData
        
        }
      }
      else{
        /* There can be only one active connection */
        String pinString=F("Not allowed!");
        //SendData 
          String temp;
          temp.reserve(50);
          temp="AT+CIPSEND="; //<link ID>,<length>
          temp.concat(id);
          temp.concat(",");
          temp.concat(pinString.length()+2);
          temp.concat("\r\n");
          writeESP(temp);
          delay(100);
          String inputData=readESP();
          if(inputData.indexOf(F("OK"))>-1){
            pinString.concat(F("\r\n"));
            writeESP(pinString);
          }
          delay(100);
          readESP();
          //End SendData
      }
    }
    dataESP="";       //ESP data never got logged wink wink
    /* STM32 */
    //     wdt_reset();
  }
}